#include <stdio.h>
#include <ogcsys.h>
#include <malloc.h>

#include "video.h"

extern void show_banner();
void video_clear(s32 color) {
	VIDEO_ClearFrameBuffer(vmode, xfb[whichfb], color);
}
void video_drawpng(IMGCTX ctx, PNGUPROP imgProp, u16 x, u16 y) {
	PNGU_DECODE_TO_COORDS_YCbYCr(ctx, x, y, imgProp.imgWidth, imgProp.imgHeight, vmode->fbWidth, vmode->xfbHeight, xfb[whichfb]);
}
void DrawFrameStart() {
	whichfb ^= 1;
	show_banner();
}
void DrawFrameFinish() {
	VIDEO_SetNextFramebuffer(xfb[whichfb]);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
}
