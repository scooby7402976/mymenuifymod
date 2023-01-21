#include <stdio.h>
#include <ogcsys.h>

#include "wpad.h"

/* Constants */
#define MAX_WIIMOTES	4

void usleep(int);

s32 wpad_init(void) {
	/* Initialize Wiimote subsystem */
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
	return 0;
}
void Wpad_Disconnect(void) {
    u32 cnt;
    /* Disconnect Wiimotes */
    for (cnt = 0; cnt < MAX_WIIMOTES; cnt++)
        WPAD_Disconnect(cnt);
    /* Shutdown Wiimote subsystem */
    WPAD_Shutdown();
}
u32 wpad_getbuttons(void) {
	u32 buttons = 0, cnt;
	/* Scan pads */
	WPAD_ScanPads();
	/* Get pressed buttons */
	for (cnt = 0; cnt < MAX_WIIMOTES; cnt++)
		buttons |= WPAD_ButtonsDown(cnt);
	return buttons;
}

u32 wpad_waitbuttons(void) {
	u32 buttons = 0;
	/* Wait for button pressing */
	while (!buttons) {
		buttons = wpad_getbuttons();
		VIDEO_WaitVSync();
	}
	return buttons;
}
void waitforbuttonpress(u32 *out, u32 *outGC) {
    u32 pressed = 0;
    u32 pressedGC = 0;
    while (true) {
        WPAD_ScanPads();
        pressed = WPAD_ButtonsDown(0) | WPAD_ButtonsDown(1) | WPAD_ButtonsDown(2) | WPAD_ButtonsDown(3);
        PAD_ScanPads();
        pressedGC = PAD_ButtonsDown(0) | PAD_ButtonsDown(1) | PAD_ButtonsDown(2) | PAD_ButtonsDown(3);
        if(pressed || pressedGC) {
            if (pressedGC) {
                // Without waiting you can't select anything
                usleep (20000);
            }
            if (out) *out = pressed;
            if (outGC) *outGC = pressedGC;
            return;
        }
    }
}
