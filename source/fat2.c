#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <ogcsys.h>
#include <fat.h>
#include <dirent.h>
#include <sys/stat.h> //for mkdir
#include <sdcard/gcsd.h>
#include <sdcard/wiisd_io.h>
#include <string.h>
#include <unistd.h>

#include "fat2.h"
#include "fat_debug.h"

const DISC_INTERFACE *interface;
const char *fat_device;

u32 Fat_Mount(int device) {
	s32 startup_return, mount_return;
	if (device == SD){
		interface = &__io_wiisd;
		fat_device = DEV_MOUNT_SD;
	}
	else if (device == USB){
		interface = &__io_usbstorage;
		fat_device = DEV_MOUNT_USB;
	}
	// Initialize storage interface
	startup_return = interface->startup();
	if (!startup_return)
		return -1;
	// Mount device
	mount_return = fatMountSimple(fat_device, interface);
	if (!mount_return){
		return -3;
	}
	return device;
}
s32 Fat_Unmount(int dev) {
	s32 ret;
	// Unmount device
	if(dev == SD)  {
		fatUnmount(DEV_UnmOUNT_SD);
	}
	else if(dev == USB) {
		fatUnmount(DEV_UnmOUNT_USB);
	}
	// Shutdown SDHC interface
	ret = interface->shutdown();
	if (!ret)
		return -1;
	return 0;
}
s32 Fat_ReadFile(const char *filepath, void **outbuf) {
	FILE *fp     = NULL;
	void *buffer = NULL;
	u32 filelen;
	s32 ret;
	/* Open file */
	fp = fopen(filepath, "rb");
	if (!fp)
		goto err;
	/* Get filesize */
	fseek(fp, 0, SEEK_END);
	filelen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	/* Allocate memory */
	buffer = malloc(filelen);
	if (!buffer)
		goto err;
	/* Read file */
	ret = fread(buffer, 1, filelen, fp);
	if (ret != filelen)
		goto err;
	/* Set pointer */
	*outbuf = buffer;
	goto out;
err:
	/* Free memory */
	if (buffer)
		free(buffer);
	/* Error code */
	ret = -1;
out:
	/* Close file */
	if (fp)
		fclose(fp);
	return ret;
}
bool Fat_MakeDir(const char *dirname) {
	DIR *dir;
	dir = opendir(dirname);
	if(dir) {
		closedir(dir);
		return false;
	}
	else {
		mkdir(dirname, S_IREAD | S_IWRITE);
		return true;
	}
}
bool Fat_CheckDir(const char *dirname) {
	DIR *dir;
	
	dir = opendir(dirname);
	
	if(dir) {
		closedir(dir);
		return true;
	}
	else return false;
}
bool Fat_CheckFile(const char *filepath) {
	FILE *fp = NULL;
	/* Open file */
	fp = fopen(filepath, "rb");
	if (!fp)
		return false;
	fclose(fp);
	return true;
}
s32 Fat_SaveFile(const char *filepath, void **outbuf, u32 outlen) {
	s32 ret;
	FILE *fd;
	fd = fopen(filepath, "wb");
	if(fd){
		ret = fwrite(*outbuf, 1, outlen, fd);
		fclose(fd);
		//logfile(" FWRITE: %d ",ret);
	}
	else{
		ret = -1;
	}
	return ret;
}
