#ifdef TARGET_PS4

#include <stdio.h>
#include <string.h>
#include <orbis/libkernel.h>
#include <piglet.h>

#include "gfx_window_manager_api.h"

#define DISPLAY_WIDTH 1920
#define DISPLAY_HEIGHT 1080

#ifdef VERSION_EU
    #define FRAMERATE 25
#else
    #define FRAMERATE 30
#endif

static EGLDisplay s_display = EGL_NO_DISPLAY;
static EGLSurface s_surface = EGL_NO_SURFACE;
static EGLContext s_context = EGL_NO_CONTEXT;

static const uint64_t frametime = 1000000 / FRAMERATE;

void gfx_ps4_init(const char *game_name, bool start_in_fullscreen) {
    ScePglConfig pgl_config;
    SceWindow render_window = { 0, DISPLAY_WIDTH, DISPLAY_HEIGHT };
    EGLConfig config = NULL;
    EGLint num_configs;

    EGLint attribs[] = {
      EGL_RED_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE, 8,
      EGL_ALPHA_SIZE, 8,
      EGL_DEPTH_SIZE, 0,
      EGL_STENCIL_SIZE, 0,
      EGL_SAMPLE_BUFFERS, 0,
      EGL_SAMPLES, 0,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_NONE,
    };

    EGLint ctx_attribs[] = {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE,
    };

    EGLint window_attribs[] = {
      EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
      EGL_NONE,
    };

    int major, minor;
    int ret;

    memset(&pgl_config, 0, sizeof(pgl_config));
    {
      pgl_config.size = sizeof(pgl_config);
      pgl_config.flags = SCE_PGL_FLAGS_USE_COMPOSITE_EXT | SCE_PGL_FLAGS_USE_FLEXIBLE_MEMORY | 0x60;
      pgl_config.processOrder = 1;
      pgl_config.systemSharedMemorySize = 0x1000000;
      pgl_config.videoSharedMemorySize = 0x3000000;
      pgl_config.maxMappedFlexibleMemory = 0xFFFFFFFF;
      pgl_config.drawCommandBufferSize = 0x100000;
      pgl_config.lcueResourceBufferSize = 0x1000000;
      pgl_config.dbgPosCmd_0x40 = DISPLAY_WIDTH;
      pgl_config.dbgPosCmd_0x44 = DISPLAY_HEIGHT;
      pgl_config.dbgPosCmd_0x48 = 0;
      pgl_config.dbgPosCmd_0x4C = 0;
      pgl_config.unk_0x5C = 2;
    }

    s_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (s_display == EGL_NO_DISPLAY) {
      printf("eglGetDisplay failed.\n");
      goto err;
    }

    if (!eglInitialize(s_display, &major, &minor)) {
      ret = eglGetError();
      printf("eglInitialize failed: 0x%08X\n", ret);
      goto err;
    }
    printf("EGL version major:%d, minor:%d\n", major, minor);

    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
      ret = eglGetError();
      printf("eglBindAPI failed: 0x%08X\n", ret);
      goto err;
    }

    if (!eglSwapInterval(s_display, 0)) {
      ret = eglGetError();
      printf("eglSwapInterval failed: 0x%08X\n", ret);
      goto err;
    }

    if (!eglChooseConfig(s_display, attribs, &config, 1, &num_configs)) {
      ret = eglGetError();
      printf("eglChooseConfig failed: 0x%08X\n", ret);
      goto err;
    }
    if (num_configs != 1) {
      printf("No available configuration found.\n");
      goto err;
    }

    s_surface = eglCreateWindowSurface(s_display, config, &render_window, window_attribs);
    if (s_surface == EGL_NO_SURFACE) {
      ret = eglGetError();
      printf("eglCreateWindowSurface failed: 0x%08X\n", ret);
      goto err;
    }

    s_context = eglCreateContext(s_display, config, EGL_NO_CONTEXT, ctx_attribs);
    if (s_context == EGL_NO_CONTEXT) {
      ret = eglGetError();
      printf("eglCreateContext failed: 0x%08X\n", ret);
      goto err;
    }

    if (!eglMakeCurrent(s_display, s_surface, s_surface, s_context)) {
      ret = eglGetError();
      printf("eglMakeCurrent failed: 0x%08X\n", ret);
      goto err;
    }

    printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
    printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));

    return;

err:
	  return;
}

void gfx_ps4_set_keyboard_callbacks(bool (*on_key_down)(int scancode), bool (*on_key_up)(int scancode), void (*on_all_keys_up)()) {
}

void gfx_ps4_set_fullscreen_changed_callback(void (*on_fullscreen_changed)(bool is_now_fullscreen)) {
}

void gfx_ps4_set_fullscreen(bool enable) {
}

void gfx_ps4_main_loop(void (*run_one_game_iter)()) {
    uint64_t t = 0;
    t = sceKernelGetProcessTime();
    run_one_game_iter();
    t = sceKernelGetProcessTime() - t;
    if (t < frametime)
        sceKernelUsleep(frametime - t);
}

void gfx_ps4_get_dimensions(uint32_t *width, uint32_t *height) {
    *width = DISPLAY_WIDTH;
    *height = DISPLAY_HEIGHT;
}

void gfx_ps4_handle_events() {
}

bool gfx_ps4_start_frame() {
    return true;
}

void gfx_ps4_swap_buffers_begin() {
		eglSwapBuffers(s_display, s_surface);
}

void gfx_ps4_swap_buffers_end() {
    glFinish();
}

double gfx_ps4_get_time() {
    uint64_t time = sceKernelGetProcessTime();

    return (double) time / (1.0 * 1000.0 * 1000.0);
}

struct GfxWindowManagerAPI gfx_ps4 = {
    gfx_ps4_init,
    gfx_ps4_set_keyboard_callbacks,
    gfx_ps4_set_fullscreen_changed_callback,
    gfx_ps4_set_fullscreen,
    gfx_ps4_main_loop,
    gfx_ps4_get_dimensions,
    gfx_ps4_handle_events,
    gfx_ps4_start_frame,
    gfx_ps4_swap_buffers_begin,
    gfx_ps4_swap_buffers_end,
    gfx_ps4_get_time
};

#endif
