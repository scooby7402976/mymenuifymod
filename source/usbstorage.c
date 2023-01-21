/*-------------------------------------------------------------

usbstorage_starlet.c -- USB mass storage support, inside starlet
Copyright (C) 2009 Kwiirk

If this driver is linked before libogc, this will replace the original
usbstorage driver by svpe from libogc
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.      The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.      Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.      This notice may not be removed or altered from any source
distribution.

-------------------------------------------------------------*/

#include <gccore.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

/* IOCTL commands */
#define UMS_BASE                        (('U'<<24)|('M'<<16)|('S'<<8))
#define USB_IOCTL_UMS_INIT                      (UMS_BASE+0x1)
#define USB_IOCTL_UMS_GET_CAPACITY      (UMS_BASE+0x2)
#define USB_IOCTL_UMS_READ_SECTORS      (UMS_BASE+0x3)
#define USB_IOCTL_UMS_WRITE_SECTORS             (UMS_BASE+0x4)
#define USB_IOCTL_UMS_READ_STRESS               (UMS_BASE+0x5)
#define USB_IOCTL_UMS_SET_VERBOSE               (UMS_BASE+0x6)
#define USB_IOCTL_UMS_UNMOUNT                   (UMS_BASE+0x10)
#define USB_IOCTL_UMS_WATCHDOG                  (UMS_BASE+0x80)

#define UMS_HEAPSIZE                    0x1000

/* Variables */
static char fs[] ATTRIBUTE_ALIGN(32) = "/dev/usb2";
static char fs2[] ATTRIBUTE_ALIGN(32) = "/dev/usb/ehc";

static s32 hid = -1, fd = -1;
static u32 sector_size;

extern void* SYS_AllocArena2MemLo(u32 size,u32 align);
static void *mem2_ptr=NULL;

inline s32 __USBStorage_isMEM2Buffer(const void *buffer) {
    u32 high_addr = ((u32)buffer) >> 24;

    return (high_addr == 0x90) || (high_addr == 0xD0);
}


s32 USBStorage_GetCapacity(u32 *_sector_size) {
    if (fd > 0) {
        s32 ret;

        ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_GET_CAPACITY, ":i", &sector_size);

        if (ret && _sector_size)
            *_sector_size = sector_size;

        return ret;
    }

    return IPC_ENOENT;
}

s32 USBStorage_Init(void) {
    s32 ret;

    /* Already open */
    if (fd > 0)
        return 0;

    /* Create heap */
    if (hid < 0) {
        hid = iosCreateHeap(UMS_HEAPSIZE);
        if (hid < 0)
            return IPC_ENOMEM;
    }

    /* Open USB device */
    fd = IOS_Open(fs, 0);
        if (fd < 0)
        fd = IOS_Open(fs2, 0);
    if (fd < 0)
        return fd;

    /* Initialize USB storage */
    ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_INIT, ":");
    if (ret<0) goto err;

    /* Get device capacity */
    ret = USBStorage_GetCapacity(NULL);
    if (!ret)
        goto err;

    return 0;

err:
    /* Close USB device */
    if (fd > 0) {
        IOS_Close(fd);
        fd = -1;
    }

    return -1;
}

/** Hermes **/
s32 USBStorage_Watchdog(u32 on_off) {
    if (fd >= 0) {
        s32 ret;

        ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_WATCHDOG, "i:", on_off);

        return ret;
    }

    return IPC_ENOENT;
}

s32 USBStorage_Umount(void) {
    if (fd >= 0) {
        s32 ret;
        ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_UNMOUNT, ":");
        return ret;
    }

    return IPC_ENOENT;
}

void USBStorage_Deinit(void) {
    /* Close USB device */
    if (fd > 0) {
        IOS_Close(fd);
        fd = -1;
    }
}

s32 USBStorage_ReadSectors(u32 sector, u32 numSectors, void *buffer) {

        void *buf = (void *)buffer;
        u32   len = (sector_size * numSectors);

        s32 ret;

        /* Device not opened */
        if (fd < 0)
                return fd;
    if(!mem2_ptr) mem2_ptr=SYS_AllocArena2MemLo(2048*256,32);
        /* MEM1 buffer */
        if (!__USBStorage_isMEM2Buffer(buffer)) {
                /* Allocate memory */
                buf = mem2_ptr; //iosAlloc(hid, len);
                if (!buf)
                        return IPC_ENOMEM;
        }

        /* Read data */
        ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_READ_SECTORS, "ii:d", sector, numSectors, buf, len);

        /* Copy data */
        if (buf != buffer) {
                memcpy(buffer, buf, len);
                //iosFree(hid, buf);
        }

        return ret;
}

s32 USBStorage_WriteSectors(u32 sector, u32 numSectors, const void *buffer) {

        void *buf = (void *)buffer;
        u32   len = (sector_size * numSectors);

        s32 ret;

        /* Device not opened */
        if (fd < 0)
                return fd;
        if(!mem2_ptr) mem2_ptr = SYS_AllocArena2MemLo(2048*256,32);


        /* MEM1 buffer */
        if (!__USBStorage_isMEM2Buffer(buffer)) {
                /* Allocate memory */
                buf = mem2_ptr; //buf = iosAlloc(hid, len);
                if (!buf)
                        return IPC_ENOMEM;

                /* Copy data */
                memcpy(buf, buffer, len);
        }

        /* Write data */
        ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_WRITE_SECTORS, "ii:d", sector, numSectors, buf, len);

        /* Free memory */
        if (buf != buffer)
                iosFree(hid, buf);

        return ret;
}


#define DEVICE_TYPE_WII_UMS (('W'<<24)|('U'<<16)|('M'<<8)|'S')


bool umsio_Startup() {
    return USBStorage_Init() == 0;
}

bool umsio_IsInserted() {
    return true; // allways true
}
bool umsio_ReadSectors(sec_t sector, sec_t numSectors, u8 *buffer) {
    u32 cnt = 0;
    s32 ret;
    /* Do reads */
    while (cnt < numSectors) {
        u32   sectors = (numSectors - cnt);

        /* Read sectors is too big */
        if (sectors > 32)
            sectors = 32;

        /* USB read */
        ret = USBStorage_ReadSectors(sector + cnt, sectors, &buffer[cnt*512]);
        if (ret < 0)
            return false;

        /* Increment counter */
        cnt += sectors;
    }

    return true;
}

bool umsio_WriteSectors(sec_t sector, sec_t numSectors, const u8* buffer) {
    u32 cnt = 0;
    s32 ret;

    /* Do writes */
    while (cnt < numSectors) {
        u32   sectors = (numSectors - cnt);

        /* Write sectors is too big */
        if (sectors > 32)
            sectors = 32;

        /* USB write */
        ret = USBStorage_WriteSectors(sector + cnt, sectors, &buffer[cnt * 512]);
        if (ret < 0)
            return false;

        /* Increment counter */
        cnt += sectors;
    }

    return true;
}
bool umsio_ClearStatus(void) {
    return true;
}

bool umsio_Shutdown() {
    USBStorage_Deinit();
    return true;
}
const DISC_INTERFACE __io_wiiums = {
    DEVICE_TYPE_WII_UMS,
    FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
    (FN_MEDIUM_STARTUP)&umsio_Startup,
    (FN_MEDIUM_ISINSERTED)&umsio_IsInserted,
    (FN_MEDIUM_READSECTORS)&umsio_ReadSectors,
    (FN_MEDIUM_WRITESECTORS)&umsio_WriteSectors,
    (FN_MEDIUM_CLEARSTATUS)&umsio_ClearStatus,
    (FN_MEDIUM_SHUTDOWN)&umsio_Shutdown
};
