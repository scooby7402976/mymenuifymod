#include <stdio.h>
#include <stdlib.h>
#include <ogcsys.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <sys/dir.h>
#include "fat_debug.h"

const char *device_Name(int input_pos) {
	
	switch(input_pos) {
		case 1:
			return "SD";
		break;
		case 2:
			return "USB";
		break;
		default:
			return "UNKNOWN";
		break;
	}
}
void logfile(const char *format, ...) {
	
	char buffer[2048];
	char path[2048];
	va_list args;
	va_start (args, format);
	vsprintf (buffer,format, args);
	FILE *f = NULL;
	
	sprintf(path, "%s:/mymenuifymod.log", device_Name(fatdevicemounted));
	f = fopen(path, "a");
	if (!f) {
		return;
	}
	fputs(buffer, f);
	fclose(f);
	va_end (args);
	return;
}