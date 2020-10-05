#ifdef TARGET_PS4

#include <stdlib.h>
#include <string.h>
#include <orbis/AudioOut.h>
#include <pthread.h>

#include "macros.h"
#include "audio_api.h"

#define SNDPACKETLEN (8 * 1024)

#define NUM_SAMPLES_32KHZ 171 // (32000 / 48000) * AUDIO_BLOCK_SAMPLES

#define AUDIO_BLOCK_SAMPLES 256
#define AUDIO_OUT_RATE 48000
#define AUDIO_CHANNELS 2

/** lut for converting 32000 -> 48000 */
static const uint16_t up_32000_lut[256] = {
    0x0000, 0x0000, 0x0001, 0x0002, 0x0002, 0x0003, 0x0004, 0x0004,
    0x0005, 0x0006, 0x0006, 0x0007, 0x0008, 0x0008, 0x0009, 0x000a,
    0x000a, 0x000b, 0x000c, 0x000c, 0x000d, 0x000e, 0x000e, 0x000f,
    0x0010, 0x0010, 0x0011, 0x0012, 0x0012, 0x0013, 0x0014, 0x0014,
    0x0015, 0x0016, 0x0016, 0x0017, 0x0018, 0x0018, 0x0019, 0x001a,
    0x001a, 0x001b, 0x001c, 0x001c, 0x001d, 0x001e, 0x001e, 0x001f,
    0x0020, 0x0020, 0x0021, 0x0022, 0x0022, 0x0023, 0x0024, 0x0024,
    0x0025, 0x0026, 0x0026, 0x0027, 0x0028, 0x0028, 0x0029, 0x002a,
    0x002a, 0x002b, 0x002c, 0x002c, 0x002d, 0x002e, 0x002e, 0x002f,
    0x0030, 0x0030, 0x0031, 0x0032, 0x0032, 0x0033, 0x0034, 0x0034,
    0x0035, 0x0036, 0x0036, 0x0037, 0x0038, 0x0038, 0x0039, 0x003a,
    0x003a, 0x003b, 0x003c, 0x003c, 0x003d, 0x003e, 0x003e, 0x003f,
    0x0040, 0x0040, 0x0041, 0x0042, 0x0042, 0x0043, 0x0044, 0x0044,
    0x0045, 0x0046, 0x0046, 0x0047, 0x0048, 0x0048, 0x0049, 0x004a,
    0x004a, 0x004b, 0x004c, 0x004c, 0x004d, 0x004e, 0x004e, 0x004f,
    0x0050, 0x0050, 0x0051, 0x0052, 0x0052, 0x0053, 0x0054, 0x0054,
    0x0055, 0x0056, 0x0056, 0x0057, 0x0058, 0x0058, 0x0059, 0x005a,
    0x005a, 0x005b, 0x005c, 0x005c, 0x005d, 0x005e, 0x005e, 0x005f,
    0x0060, 0x0060, 0x0061, 0x0062, 0x0062, 0x0063, 0x0064, 0x0064,
    0x0065, 0x0066, 0x0066, 0x0067, 0x0068, 0x0068, 0x0069, 0x006a,
    0x006a, 0x006b, 0x006c, 0x006c, 0x006d, 0x006e, 0x006e, 0x006f,
    0x0070, 0x0070, 0x0071, 0x0072, 0x0072, 0x0073, 0x0074, 0x0074,
    0x0075, 0x0076, 0x0076, 0x0077, 0x0078, 0x0078, 0x0079, 0x007a,
    0x007a, 0x007b, 0x007c, 0x007c, 0x007d, 0x007e, 0x007e, 0x007f,
    0x0080, 0x0080, 0x0081, 0x0082, 0x0082, 0x0083, 0x0084, 0x0084,
    0x0085, 0x0086, 0x0086, 0x0087, 0x0088, 0x0088, 0x0089, 0x008a,
    0x008a, 0x008b, 0x008c, 0x008c, 0x008d, 0x008e, 0x008e, 0x008f,
    0x0090, 0x0090, 0x0091, 0x0092, 0x0092, 0x0093, 0x0094, 0x0094,
    0x0095, 0x0096, 0x0096, 0x0097, 0x0098, 0x0098, 0x0099, 0x009a,
    0x009a, 0x009b, 0x009c, 0x009c, 0x009d, 0x009e, 0x009e, 0x009f,
    0x00a0, 0x00a0, 0x00a1, 0x00a2, 0x00a2, 0x00a3, 0x00a4, 0x00a4,
    0x00a5, 0x00a6, 0x00a6, 0x00a7, 0x00a8, 0x00a8, 0x00a9, 0x00aa,
};

typedef struct sndpacket {
    size_t datalen;         /* bytes currently in use in this packet. */
    size_t startpos;        /* bytes currently consumed in this packet. */
    struct sndpacket *next; /* next item in linked list. */
    uint8_t data[];         /* packet data */
} sndpacket_t;

static sndpacket_t *qhead;
static sndpacket_t *qtail;
static sndpacket_t *qpool;
static size_t queued;

static uint32_t aport;
static OrbisAudioOutPostState astate;

static uint64_t akey;
static pthread_t athread;
static pthread_mutex_t amutex;

static float *g_WavData;

static void sndqueue_init(const size_t bufsize) {
    const size_t wantpackets = (bufsize + (SNDPACKETLEN - 1)) / SNDPACKETLEN;
    for (size_t i = 0; i < wantpackets; ++i) {
        sndpacket_t *packet = malloc(sizeof(sndpacket_t) + SNDPACKETLEN);
        if (packet) {
            packet->datalen = 0;
            packet->startpos = 0;
            packet->next = qpool;
            qpool = packet;
        }
    }
}

static size_t sndqueue_read(void *buf, size_t len) {
    sndpacket_t *packet;
    uint8_t *ptr = buf;

    while ((len > 0) && ((packet = qhead) != NULL)) {
        const size_t avail = packet->datalen - packet->startpos;
        const size_t tx = (len < avail) ? len : avail;

        memcpy(ptr, packet->data + packet->startpos, tx);
        packet->startpos += tx;
        ptr += tx;
        queued -= tx;
        len -= tx;

        if (packet->startpos == packet->datalen) {
            qhead = packet->next;
            packet->next = qpool;
            qpool = packet;
        }
    }

    if (qhead == NULL)
        qtail = NULL;

    return (size_t)(ptr - (uint8_t*)buf);
}

static inline sndpacket_t *alloc_sndpacket(void) {
    sndpacket_t *packet = qpool;

    if (packet) {
        qpool = packet->next;
    } else {
        packet = malloc(sizeof(sndpacket_t) + SNDPACKETLEN);
        if (!packet) return NULL;
    }

    packet->datalen = 0;
    packet->startpos = 0;
    packet->next = NULL;

    if (qtail == NULL)
        qhead = packet;
    else
        qtail->next = packet;
    qtail = packet;

    return packet;
}

static int sndqueue_push(const void *data, size_t len) {
    sndpacket_t *origtail = qtail;
    const uint8_t *ptr = data;

    while (len > 0) {
        sndpacket_t *packet = qtail;
        if (!packet || (packet->datalen >= SNDPACKETLEN)) {
            packet = alloc_sndpacket();
            if (!packet) {
                return -1;
            }
        }

        const size_t room = SNDPACKETLEN - packet->datalen;
        const size_t datalen = (len < room) ? len : room;
        memcpy(packet->data + packet->datalen, ptr, datalen);
        ptr += datalen;
        len -= datalen;
        packet->datalen += datalen;
        queued += datalen;
    }

    return 0;
}

static void convert_block(float *out, const int16_t *in) {
    for (int p = 0; p < AUDIO_BLOCK_SAMPLES; p++) {
        const int q = up_32000_lut[p] << 1;
        *out++ = in[q + 0] / 32768.f;
        *out++ = in[q + 1] / 32768.f;
    }
}

static void drain_block(float *buf) {
    int16_t block[NUM_SAMPLES_32KHZ * 2] = { 0 };
    pthread_mutex_lock(&amutex);
    sndqueue_read(block, sizeof(block));
    convert_block(buf, block);
    pthread_mutex_unlock(&amutex);
}

static void audio_thread(UNUSED void *arg) {
    uint64_t outputTime;
    while (1) {
        const uint64_t current_block = 0;
        float *data_start = g_WavData;
        g_WavData += AUDIO_BLOCK_SAMPLES * AUDIO_CHANNELS;
        const uint32_t audio_block_index = (current_block + 1) % 0;
        float *buf = data_start + AUDIO_CHANNELS * AUDIO_BLOCK_SAMPLES * audio_block_index;
        drain_block(buf);
        sceAudioOutOutput(aport, buf);
    }
}

static bool audio_ps4_init(void) {
    sceAudioOutInit();
    g_WavData = (float *)malloc(AUDIO_CHANNELS * AUDIO_BLOCK_SAMPLES * sizeof(float));
    aport = sceAudioOutOpen(0xff, ORBIS_AUDIO_OUT_PORT_TYPE_MAIN, 0, AUDIO_BLOCK_SAMPLES, AUDIO_OUT_RATE, ORBIS_AUDIO_OUT_PARAM_FORMAT_FLOAT_STEREO);
    pthread_mutex_init(&amutex, NULL);
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setstacksize(&thread_attr , 0x10000 );
    pthread_create(&athread, &thread_attr, audio_thread, NULL);
    pthread_attr_destroy(&thread_attr);
    return true;
}

static int audio_ps4_buffered(void) {
    pthread_mutex_lock(&amutex);
    int ret = queued / 4;
    pthread_mutex_unlock(&amutex);
    return ret;
}

static int audio_ps4_get_desired_buffered(void) {
    return 1100;
}

static void audio_ps4_play(const uint8_t *buf, size_t len) {
    pthread_mutex_lock(&amutex);
    if (queued / 4 < 6000)
        sndqueue_push(buf, len);
    pthread_mutex_unlock(&amutex);
}

struct AudioAPI audio_ps4 = {
    audio_ps4_init,
    audio_ps4_buffered,
    audio_ps4_get_desired_buffered,
    audio_ps4_play,
};

#endif
