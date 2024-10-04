/**
 * CleanRip - IPLFontWrite.h
 * Copyright (C) 2010 emu_kidid
 *
 * CleanRip homepage: http://code.google.com/p/cleanrip/
 * email address: emukidid@gmail.com
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 **/

#ifndef IPLFontWrite_H
#define IPLFontWrite_H

#include "video.h"

#define back_framewidth vmode->fbWidth
#define back_frameheight vmode->xfbHeight

void init_font(void);
void WriteFont(int, int, const char *);
void WriteFontHL(int, int, int, int, const char *, unsigned int *);
int GetTextSizeInPixels(const char *);
void WriteCentre(int, const char *);
void WriteCentreHL(int, const char *);
void DrawRawFont(int, int, char *);
extern char txtbuffer[2048];
extern char txtbuffer2[2048];
extern unsigned int blit_lookup_inv[4];
extern unsigned int blit_lookup[4];
extern unsigned int blit_lookup_norm[4];

#endif
