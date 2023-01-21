#include <stdio.h>
#include <ogcsys.h>
#include <stdlib.h>
#include <ogc/system.h>
#include <malloc.h>

#define HBC_HAXX    0x0001000148415858LL
#define HBC_JODI    0x000100014A4F4449LL
#define HBC_1_0_7   0x00010001AF1BF516LL
#define Menu        0x0000000100000002LL      

/* Variables */
static const char certs_fs[] ATTRIBUTE_ALIGN(32) = "/sys/cert.sys";
static vu32 *_wiilight_reg = (u32*) 0xCD0000C0;

void wiilight(int enable) {// Toggle wiilight (thanks Bool for wiilight source) 
    u32 val = (*_wiilight_reg & ~0x20);
    if (enable) val |= 0x20;
    *_wiilight_reg = val;
}

void sys_init(void) {
	/* Initialize video subsytem */
	VIDEO_Init();
}

void sys_loadmenu(void) {
	/* Return to the Wii system menu */
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

void sysHBC() {
	WII_Initialize();

    int ret = WII_LaunchTitle(HBC_1_0_7);
    if(ret < 0) WII_LaunchTitle(HBC_JODI);
    if(ret < 0) WII_LaunchTitle(HBC_HAXX);
	if(ret < 0) WII_LaunchTitle(Menu);
    //Back to system menu if all fails
   // SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}
s32 sys_getcerts(signed_blob **certs, u32 *len) {
	static signed_blob certificates[0x280] ATTRIBUTE_ALIGN(32);
	s32 fd, ret;

	/* Open certificates file */
	fd = IOS_Open(certs_fs, 1);
	if (fd < 0)
		return fd;

	/* Read certificates */
	ret = IOS_Read(fd, certificates, sizeof(certificates));

	/* Close file */
	IOS_Close(fd);

	/* Set values */
	if (ret > 0) {
		*certs = certificates;
		*len   = sizeof(certificates);
	}

	return ret;
}
s32 __u8Cmp(const void *a, const void *b) {
    return *(u8 *)a-*(u8 *)b;
}
u8 *get_ioslist(u32 *cnt) {
    u64 *buf = 0;
    s32 i, res;
    u32 tcnt = 0, icnt;
    u8 *ioses = NULL;

    //Get stored IOS versions.
    res = ES_GetNumTitles(&tcnt);
    if(res < 0) {
        printf("\nES_GetNumTitles: Error! (result = %d)\n", res);
        return 0;
    }
    buf = memalign(32, sizeof(u64) * tcnt);
    res = ES_GetTitles(buf, tcnt);
    if(res < 0) {
        printf("\nES_GetTitles: Error! (result = %d)\n", res);
        if (buf) free(buf);
        return 0;
    }

    icnt = 0;
    for(i = 0; i < tcnt; i++) {
        if(*((u32 *)(&(buf[i]))) == 1 && (u32)buf[i] > 2 && (u32)buf[i] < 0x100) {
            icnt++;
            ioses = (u8 *)realloc(ioses, sizeof(u8) * icnt);
            ioses[icnt - 1] = (u8)buf[i];
        }
    }

    ioses = (u8 *)malloc(sizeof(u8) * icnt);
    icnt = 0;

    for(i = 0; i < tcnt; i++) {
        if(*((u32 *)(&(buf[i]))) == 1 && (u32)buf[i] > 2 && (u32)buf[i] < 0x100) {
            icnt++;
            ioses[icnt - 1] = (u8)buf[i];
        }
    }
    free(buf);
    qsort(ioses, icnt, 1, __u8Cmp);

    *cnt = icnt;
    return ioses;
}
