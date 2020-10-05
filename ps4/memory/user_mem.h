#ifndef _USER_MEM_H
#define _USER_MEM_H

#define MEM_SIZE (0xA0000000) /* 2600 MiB */
#define MEM_ALIGN (16UL * 1024)
#define ORBIS_KERNEL_MAP_FIXED 0x10

typedef void* OrbisMspace;

typedef struct OrbisMallocManagedSize {
  unsigned short sz;
  unsigned short ver;
  unsigned int reserv;
  size_t maxSysSz;
  size_t curSysSz;
  size_t maxUseSz;
  size_t curUseSz;
} OrbisMallocManagedSize;

#if defined(__cplusplus)
extern "C" {
#endif
OrbisMspace sceLibcMspaceCreate(const char *, void *, size_t, unsigned int);
int sceLibcMspaceDestroy(OrbisMspace);
void *sceLibcMspaceMalloc(OrbisMspace, size_t);
void *sceLibcMspaceCalloc(OrbisMspace, size_t, size_t);
void *sceLibcMspaceRealloc(OrbisMspace, void *, size_t);
void *sceLibcMspaceReallocalign(OrbisMspace, void *, size_t, size_t);
void *sceLibcMspaceMemalign(OrbisMspace, size_t, size_t);
size_t sceLibcMspaceMallocUsableSize(void *);
int sceLibcMspacePosixMemalign(OrbisMspace, void **, size_t, size_t);
int sceLibcMspaceFree(OrbisMspace, void *);
int sceLibcMspaceMallocStats(OrbisMspace msp, OrbisMallocManagedSize *);
int sceLibcMspaceMallocStatsFast(OrbisMspace, OrbisMallocManagedSize *);
int sceKernelReserveVirtualRange(void **, size_t, int, size_t);
int sceKernelMapNamedSystemFlexibleMemory(void **, size_t, int, int, const char *);
int sceKernelReleaseFlexibleMemory(void *, size_t);
int sceKernelMunmap(void *, size_t);

int malloc_init(void);
int malloc_finalize(void);
char *strdup(const char *s);
char *strndup(const char *s, size_t n);
int asprintf(char **bufp, const char *format, ...);
int vasprintf(char **bufp, const char *format, va_list ap);
void get_user_mem_size(size_t *max_mem, size_t *cur_mem);
#if defined(__cplusplus)
}
#endif

#endif // _USER_MEM_H
