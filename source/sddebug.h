#ifndef _FAT_DEBUG
#define _FAT_DEBUG
	#include "fat.h"
	
	extern int fatdevicemounted;
	
	void logfile(const char*, ...);
	const char *device_Name(int);
	
#endif