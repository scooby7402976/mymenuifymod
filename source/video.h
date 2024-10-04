#ifndef _VIDEO_H_
#define _VIDEO_H_

#include "libpng/pngu/pngu.h"
extern GXRModeObj *vmode;	/*** Graphics Mode Object ***/
extern u32 *xfb[2];			/*** Framebuffers ***/
extern int whichfb;			/*** Frame buffer toggle ***/

/* Prototypes */
void video_drawpng(IMGCTX, PNGUPROP, u16, u16);
void DrawFrameStart();
void DrawFrameFinish();

#endif
