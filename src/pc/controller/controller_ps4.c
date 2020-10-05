#if defined(TARGET_PS4)

#include <orbis/Pad.h>
#include <orbis/UserService.h>

#include <ultra64.h>

#include "controller_api.h"

#define DEADZONE 20

static int32_t controller_ps4_handle;

static void controller_ps4_init(void) {
    OrbisUserServiceUserId userId;

    scePadInit();
    sceUserServiceGetInitialUser(&userId);
    controller_ps4_handle = scePadOpen(userId, ORBIS_PAD_PORT_TYPE_STANDARD, 0, NULL);
    if (controller_ps4_handle == ORBIS_PAD_ERROR_ALREADY_OPENED)
        controller_ps4_handle = scePadGetHandle(userId, ORBIS_PAD_PORT_TYPE_STANDARD, 0);
}

static void controller_ps4_read(OSContPad *pad) {
    OrbisPadData data;
    scePadReadState(controller_ps4_handle, &data);

    if (data.buttons & ORBIS_PAD_BUTTON_UP)
        pad->button |= U_JPAD;
    if (data.buttons & ORBIS_PAD_BUTTON_DOWN)
        pad->button |= D_JPAD;
    if (data.buttons & ORBIS_PAD_BUTTON_LEFT)
        pad->button |= L_JPAD;
    if (data.buttons & ORBIS_PAD_BUTTON_RIGHT)
        pad->button |= R_JPAD;
    if (data.buttons & ORBIS_PAD_BUTTON_OPTIONS)
        pad->button |= START_BUTTON;
    if (data.buttons & ORBIS_PAD_BUTTON_L1)
        pad->button |= Z_TRIG;
    if (data.buttons & ORBIS_PAD_BUTTON_R1)
        pad->button |= R_TRIG;
    if (data.buttons & ORBIS_PAD_BUTTON_CROSS)
        pad->button |= A_BUTTON;
    if (data.buttons & ORBIS_PAD_BUTTON_CIRCLE)
        pad->button |= B_BUTTON;
    if (data.buttons & ORBIS_PAD_BUTTON_SQUARE)
        pad->button |= B_BUTTON;

    pad->stick_x = (s8)((s32) data.leftStick.x - 128);
    pad->stick_y = (s8)(-((s32) data.leftStick.y - 127));

    uint32_t magnitude_sq = (uint32_t)(pad->stick_x * pad->stick_x) + (uint32_t)(pad->stick_y * pad->stick_y);
    if (magnitude_sq < (uint32_t)(DEADZONE * DEADZONE)) {
        pad->stick_x = 0;
        pad->stick_y = 0;
    }

    if (data.rightStick.x < 40)
        pad->button |= L_CBUTTONS;
    if (data.rightStick.x > 200)
        pad->button |= R_CBUTTONS;
    if (data.rightStick.y < 40)
        pad->button |= U_CBUTTONS;
    if (data.rightStick.y > 200)
        pad->button |= D_CBUTTONS;

}

struct ControllerAPI controller_ps4 = {
    controller_ps4_init,
    controller_ps4_read,
};

#endif
