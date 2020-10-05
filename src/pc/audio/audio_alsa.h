#ifndef AUDIO_ALSA_H
#define AUDIO_ALSA_H

#include "../compat.h"

#if (defined(__linux__) || defined(__BSD__)) && !defined(TARGET_PS4)
extern struct AudioAPI audio_alsa;
#define HAVE_ALSA 1
#else
#define HAVE_ALSA 0
#endif

#endif
