#ifndef _FAT_H_
#define _FAT_H_

#ifdef __cplusplus
	extern "C" {
		#endif
		#define SD		1
		#define USB		2
		#define DEV_MOUNT_SD  "SD"
		#define DEV_MOUNT_USB "USB"
		#define DEV_UnmOUNT_SD  "SD:"
		#define DEV_UnmOUNT_USB "USB:"
		/* Prototypes */
		u32 Fat_Mount(int);
		s32 Fat_Unmount(int);
		s32 Fat_ReadFile(const char *, void **);
		bool Fat_MakeDir(const char *);
		bool Fat_CheckDir(const char *);
		bool Fat_CheckFile(const char *);
		s32 Fat_SaveFile(const char *, void **, u32);
		#ifdef __cplusplus
	}
	#endif
#endif
