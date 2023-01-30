#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <fat.h>
#include <sys/dir.h>
#include <ogc/isfs.h>
#include <malloc.h>
#include <wiiuse/wpad.h>
#include <network.h>
#include <gccore.h>
#include <sys/errno.h>
#include <sdcard/wiisd_io.h>
#include <dirent.h>
#include <stdarg.h>

#include "globals.h"
#include "sys.h"
#include "video.h"
#include "install.h"
#include "wpad.h"
#include "fat2.h"
#include "themes.h"
#include "http.h"
#include "rijndael.h"
#include "libpng/pngu/pngu.h"
#include "grfx/mymenuifymod_png.h"
#include "iospatch.h"

// Variables 
extern const uint8_t mymenuifymod_png[];
IMGCTX ctx;
int fatdevicemounted = 0;
u32 systemmenuVersion;
themeStats currenttheme;
Fatfile *themefile = NULL;
static s32 filecnt = 0, start = 0, selected = 0; //, pagecount = 1;
u8 commonkey[16] = { 0xeb, 0xe4, 0x2a, 0x22, 0x5e, 0x85, 0x93, 0xe4, 0x48,0xd9, 0xc5, 0x45, 0x73, 0x81, 0xaa, 0xf7 };
dirent_t *nandfilelist;
//dirent_t *neeklist;
//u32 neekcount,cntr;
const char *wiishoppath = "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/";
void pngu_free_info(IMGCTX ctx);
void sleep(int);
void usleep(int);
void __exception_setreload(int t);

void Disclaimer(void) {
	u32 buttons;
	
	for(;;) {
		con_clear();
		printf("\t\t[-] [DISCLAIMER]:\n\n");

		printf("\t\t\tTHIS APPLICATION COMES WITH NO WARRANTY AT ALL,\n");
		printf("\t\t\tNEITHER EXPRESSED NOR IMPLIED.\n");
		printf("\t\t\tI DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE IN YOUR\n");
		printf("\t\t\tWII CONSOLE BECAUSE OF IMPROPER USE OF THIS SOFTWARE.\n\n");

		printf("\t\t[A] button to continue.\n");
		printf("\t\t[B] button to restart your Wii.\n");
		
		buttons = wpad_waitbuttons();
		
		if((buttons == BUTTON_B) || (buttons == BUTTON_HOME)) {
			printf("\n\t\tExiting ..... \n");
			sleep(1);
			exit(0);
		}
		if(buttons == BUTTON_A) {
			break;
		}
	}
	return;
}
void show_banner(void) {
    PNGUPROP imgProp;
    s32 ret;

    // Select PNG data 
    ctx = PNGU_SelectImageFromBuffer(mymenuifymod_png);
    if (!ctx)
        return;

    // Get image properties 
    ret = PNGU_GetImageProperties(ctx, &imgProp);
    if (ret != PNGU_OK)
        return;

    // Draw image 
    video_drawpng(ctx, imgProp, 0, 0);

     //Free image context 
    PNGU_ReleaseImageContext(ctx);
	return;
}

void freeBanner() {
	pngu_free_info(ctx);
	return;
}
u32 GetSysMenuVersion() {
    //Get sysversion from TMD
    u64 TitleID = 0x0000000100000002LL;
    u32 tmd_size;
	u32 version;
	
    s32 r = ES_GetTMDViewSize(TitleID, &tmd_size);
    if(r<0)
    {
        //logfile("error getting TMD views Size. error %d\n",r);
        return r;
    }

    tmd_view *rTMD = (tmd_view*)memalign( 32, (tmd_size+31)&(~31) );
    if( rTMD == NULL )
    {
        //logfile("error making memory for tmd views\n");
        return 0;
    }
    memset(rTMD,0, (tmd_size+31)&(~31) );
    r = ES_GetTMDView(TitleID, (u8*)rTMD, tmd_size);
    if(r<0)
    {
        //logfile("error getting TMD views. error %d\n",r);
        free( rTMD );
        return r;
    }
    version = rTMD->title_version;
    if(rTMD)
    {
        free(rTMD);
    }
    return version;
}
const char *getregion(u32 num) {
    switch(num) {
    case 289:
    case 417:
    case 449:
    case 481:
    case 513:
        return "U";
        break;
    case 290:
    case 418:
    case 450:
    case 482:
    case 514:
        return "E";
        break;
    case 288:
    case 416:
    case 448:
    case 480:
    case 512:
        return "J";
        break;
    case 486:
    case 454:
    case 518:
        return "K";
        break;
    default:
		return "UNKNOWN";
    break;
    }
}
const char *getsysvernum(u32 num) {
    switch(num)
    {
    case 416:
    case 417:
    case 418:
        return "4.0";
        break;
    case 448:
    case 449:
    case 450:
	case 454:
        return "4.1";
        break;
    case 480:
    case 481:
    case 482:
	case 486:
        return "4.2";
        break;
    case 512:
    case 513:
    case 514:
	case 518:
        return "4.3";
        break;
	default:
		return "UNKNOWN";
		break;
	}
}
const char *device_Name(int input_pos) {
	
	switch(input_pos) {
		case 1:
			return "sd";
		break;
		case 2:
			return "usb";
		break;
		case 3: 
			return "usb2";
		break;
		default:
			return "UNKNOWN";
		break;
	}
}
void logfile(const char *format, ...) {
	
	char buffer[256];
	char path[256];
	va_list args;
	va_start (args, format);
	vsprintf (buffer,format, args);
	FILE *f = NULL;
	
	sprintf(path, "%s:/mymenuifymod.log", device_Name(fatdevicemounted));
	f = fopen(path, "a");
	if (!f) {
		printf("Error writing log\n");
		return;
	}
	fputs(buffer, f);
	fclose(f);
	va_end (args);
	return;
}
const char *getsavename(u32 idx) {
    switch(idx)
    {
    case 417:
        return "00000072.app";
        break;
    case 449:
        return "0000007b.app";
        break;
    case 481:
        return "00000087.app";
        break;
    case 513:
        return "00000097.app";// usa
        break;
    case 418:
        return "00000075.app";
        break;
    case 450:
        return "0000007e.app";
        break;
    case 482:
        return "0000008a.app";
        break;
    case 514:
        return "0000009a.app";// pal
        break;
    case 416:
        return "00000070.app";
        break;
    case 448:
        return "00000078.app";
        break;
    case 480:
        return "00000084.app";
        break;
    case 512:
        return "00000094.app";// jpn
        break;
    case 486:
        return "0000008d.app";// kor
        break;
    case 454:
        return "00000081.app";
        break;
    case 518:
        return "0000009d.app";// kor
        break;
    default:
        return "UNKNOWN";
        break;
    }
}
void get_title_key(signed_blob *s_tik, u8 *key) {
    static u8 iv[16] ATTRIBUTE_ALIGN(0x20);
    static u8 keyin[16] ATTRIBUTE_ALIGN(0x20);
    static u8 keyout[16] ATTRIBUTE_ALIGN(0x20);

    const tik *p_tik;
    p_tik = (tik*) SIGNATURE_PAYLOAD(s_tik);
    u8 *enc_key = (u8 *) &p_tik->cipher_title_key;
    memcpy(keyin, enc_key, sizeof keyin);
    memset(keyout, 0, sizeof keyout);
    memset(iv, 0, sizeof iv);
    memcpy(iv, &p_tik->titleid, sizeof p_tik->titleid);

    aes_set_key(commonkey);
    aes_decrypt(iv, keyin, keyout, sizeof keyin);

    memcpy(key, keyout, sizeof keyout);
}
static void decrypt_buffer(u16 index, u8 *source, u8 *dest, u32 len) {
    static u8 iv[16];
    memset(iv, 0, 16);
    memcpy(iv, &index, 2);
    aes_decrypt(iv, source, dest, len);
}
const char *appfilename_noext(int ind) {
    switch(ind)
    {
    case 0:
        return "";
        break;
    case 1:
        return "";
        break;
    case 2:
        return "0000003f";
        break;
    case 3:
        return "00000042";
        break;
    case 4:
        return "00000045";
        break;
    case 5:
        return "0000006f";
        break;
    case 6:
        return "00000072";
        break;
    case 7:
        return "00000075";
        break;
    case 8:
        return "00000078";
        break;
    case 9:
        return "0000007b";
        break;
    case 10:
        return "0000007e";
        break;
    case 11:
        return "00000081";
        break;
    case 12:
        return "00000084";
        break;
    case 13:
        return "00000087";
        break;
    case 14:
        return "0000008a";
        break;
    case 15:
        return "0000008d";
        break;
    case 16:
        return "00000094";
        break;
    case 17:
        return "00000097";
        break;
    case 18:
        return "0000009a";
        break;
    case 19:
        return "0000009d";
        break;
    default:
        return "ERROR";
        break;
    }
}
int getslot(int num) {
    switch(num)
    {
    case 416:
        return 5;
        break;
    case 417:
        return 6;
        break;
    case 418:
        return 7;
        break;
    case 448:
        return 8;
        break;
    case 449:
        return 9;
        break;
    case 450:
        return 10;
        break;
    case 454:
        return 11;
        break;
    case 480:
        return 12;
        break;
    case 481:
        return 13;
        break;
    case 482:
        return 14;
        break;
    case 486:
        return 15;
        break;
    case 512:
        return 16;
        break;
    case 513:
        return 17;
        break;
    case 514:
        return 18;
        break;
    case 518:
        return 19;
        break;
    default:
        return -1;
        break;
    }
}
u32 checkcustomsystemmenuversion() {
	u32 nandfilecnt = 0, filecounter = 0, knownversioncounter = 0;
	char *knownversionstr = null;
	
	getdir("/title/00000001/00000002/content",&nandfilelist,&nandfilecnt);
	for(filecounter = 0; filecounter < nandfilecnt; filecounter++) {
		for(knownversioncounter = 0; knownversioncounter < KNOWN_SYSTEMMENU_VERSIONS; knownversioncounter++) {
			knownversionstr = knownappfilenames[knownversioncounter];
			if(strcmp(nandfilelist[filecounter].name, knownversionstr) == 0) return known_Versions[knownversioncounter];
		}
	}
	return 0;
}
int downloadApp() {
	//s32 rtn;
    u32 tmpversion;
    int ret;
    int counter;
	int retries = 50, retrycnt;
    char *savepath = (char*)memalign(32, 256);
	
    char *wiiserverlist[2] = {"cetk", "tmd."};
	signed_blob * s_tik = NULL;
    signed_blob * s_tmd = NULL;
	u32 outlen = 0;
       u32 http_status = 0;
		
    tmpversion = GetSysMenuVersion();
    logfile("dvers =%d \n", tmpversion);
    if(tmpversion > 518) tmpversion = checkcustomsystemmenuversion();
	
    con_clear();
    printf("Initializing  Network ..... ");
    for(retrycnt = 0; retrycnt < retries; retrycnt++) {
        ret = net_init();
		if(ret == 0) { printf("Complete .\n"); break; }
    }
	if(retrycnt == 50) { printf("Failed .\n"); return ret; }
	
	printf("\nDownloading %s for System Menu v%d \n", getsavename(tmpversion), tmpversion);

	for(counter = 0; counter < 3; counter++) {	
        int app_pos = getslot(tmpversion);
        char *path = (char*)memalign(32, 256);
		if(counter == 0) {
            sprintf(path,"%s%s", wiishoppath, wiiserverlist[counter]);
            printf("Dowloading System Menu Cetk .... ");
			ret = http_request(path, 1<<31, false);
        }
        else if(counter == 1) {
            sprintf(path,"%s%s%d", wiishoppath, wiiserverlist[counter], tmpversion);
            printf("Dowloading System Menu Tmd .... ");
			ret = http_request(path, 1<<31, false);
        }
        else if(counter == 2) {
            sprintf(path,"%s%s",wiishoppath , appfilename_noext(app_pos));
            printf("Dowloading %s .... ", getsavename(tmpversion));
			ret = http_request(path, 1<<31, true);
        }
        if(ret == 0 ) {
            free(path);
            logfile("download failed !! ret(%d)\n",ret);
            printf("Failed !! ret(%d)\n",ret);
            sleep(3);
            return -1;
        }
        
        u8* outbuf = (u8*)malloc(outlen);
        if(counter == 0) ret = http_get_result(&http_status, (u8 **)&s_tik, &outlen);
        if(counter == 1) ret = http_get_result(&http_status, (u8 **)&s_tmd, &outlen);
        if(counter == 2) ret = http_get_result(&http_status, &outbuf, &outlen);
        printf("\nDecrypting files ....");
		
        //set aes key
        u8 key[16];
        u16 index;
        get_title_key(s_tik, key);
        aes_set_key(key);
        u8* outbuf2 = (u8*)malloc(outlen);
        if(counter == 2) {
            if(outlen > 0) {//suficientes bytes
                index = 01;
                //then decrypt buffer
                decrypt_buffer(index,outbuf,outbuf2,outlen);
                sprintf(savepath,"%s:/themes/%s", device_Name(fatdevicemounted), getsavename(tmpversion));
                ret = Fat_SaveFile(savepath, (void *)&outbuf2, outlen);
            }
        }
        printf("Complete !! \n\n");
        if(outbuf != NULL)
            free(outbuf);
    }
	net_deinit();
	
	sleep(2);
	
	if(!Fat_CheckFile(savepath)) {
		if(savepath != NULL)
            free(savepath);
		return -99;
	}
	
	if(savepath != NULL)
        free(savepath);
    return 1;
}
int theme_entrycmp(const void *p1, const void *p2) {
	Fatfile *f1 = (Fatfile *)p1;
	Fatfile *f2 = (Fatfile *)p2;
    return strcasecmp(f1->name, f2->name);
}
int installregion(u32 inputversion) {
	switch(inputversion) {
		case 416:
		case 448:
		case 480:
		case 512:
			return 74;
			break;
		case 417:
		case 449:
		case 481:
		case 513:
			return 85;
			break;
		case 418:
		case 450:
		case 482:
		case 514:
			return 69;
			break;
		case 454:
		case 486:
		case 518:
			return 75;
			break;
	}
	return 0;
}
u32 findinstallthemeversion(char * name) { 
	char filepath[256];
    FILE *fp = null;
    u32 length, i, rtn = 0;
    u8 *data;
	
	sprintf(filepath, "%s:/themes/%s", device_Name(fatdevicemounted), name);
    fp = fopen(filepath, "rb");
    if (!fp) {
        printf("unable to open path\n");
		return 0;
	}
    length = filesize(fp);
    data = allocate_memory(length);
    memset(data,0,length);
    fread(data,1,length,fp);
	fclose(fp);
	
    if(length <= 0) {
        printf("[-] Unable to read file !! \n");
		//logfile("[-] Unable to read file !! \n");
        return 0;
    }
    else {
        for(i = 0; i < length; i++)
        {
            if(data[i] == 83)
            {
                if(data[i+6] == 52)  // 4
                {
                    if(data[i+8] == 48)  // 0
                    {
                        if(data[i+28] == 85)  // usa
                        {
                            rtn = 417;
                           // rtn.region = 85;
                            break;
                        }
                        else if(data[i+28] == 74)  //jap
                        {
                            rtn = 416;
                            //rtn.region = 74;
                            break;
                        }
                        else if(data[i+28] == 69)  // pal
                        {
                            rtn = 418;
                            //rtn.region = 69;
                            break;
                        }
                    }
                    else if(data[i+8] == 49)  // 4.1
                    {
                        if(data[i+31] == 85)  // usa
                        {
                            rtn = 449;
                           // rtn.region = 85;
                            break;
                        }
                        else if(data[i+31] == 74)  //jap
                        {
                            rtn = 448;
                            //rtn.region = 74;
                            break;
                        }
                        else if(data[i+31] == 69)  // pal
                        {
                            rtn = 450;
                            //rtn.region = 69;
                            break;
                        }
                        else if(data[i+31] == 75)  // kor
                        {
                            rtn = 454;
                            //rtn.region = 75;
                            break;
                        }
                    }
					else if(data[i+8] == 50)  // 4.2
                    {
                        if(data[i+28] == 85)  // usa
                        {
                            rtn = 481;
                            //rtn.region = 85;
                            break;
                        }
                        else if(data[i+28] == 74)  // jap
                        {
                            rtn = 480;
                            //rtn.region = 74;
                            break;
                        }
                        else if(data[i+28] == 69)  // pal
                        {
                            rtn = 482;
                            //rtn.region = 69;
                            break;
                        }
                        else if(data[i+28] == 75)  // kor
                        {
                            rtn = 486;
                            //rtn.region = 75;
                            break;
                        }
                    }
                    else if(data[i+8] == 51) // 4.3
                    {
                        if(data[i+28] == 85)  // usa
                        {
                            rtn = 513;
                            //rtn.region = 85;
                            break;
                        }
                        else if(data[i+28] == 74)  //jap
                        {
                            rtn = 512;
                            //rtn.region = 74;
                            break;
                        }
                        else if(data[i+28] == 69)  // pal
                        {
                            rtn = 514;
                            //rtn.region = 69;
                            break;
                        }
                        else if(data[i+28] == 75)  // kor
                        {
                            rtn = 518;
                            //rtn.region = 75;
                            break;
                        }
                    }
                }
            }
        }
    }
	free(data);
	
	return rtn;
}
s32 filelist_retrieve() {
    char dirpath[MAX_FILELIST_LEN];
	u32 filelistcntr;
	
	printf("\t\tRetrieving file list ..... ");
    // Generate dirpath 
	sprintf(dirpath, "%s:/themes", device_Name(fatdevicemounted));
	
	DIR *mydir;
	mydir = opendir(dirpath);
	if(!mydir) {
		printf("unable to open %s \n", dirpath);
		sleep(2);
		return -99;
	}
	filelistcntr = 0;
    struct dirent *entry = NULL;
    while((entry = readdir(mydir))) {  // If we get EOF, the expression is 0 and the loop stops.
		if(strncmp(entry->d_name, ".", 1) == 0 || strncmp(entry->d_name, "..", 2) == 0)
		continue;
		else
		filelistcntr += 1;
    }
	themefile = allocate_memory(sizeof(Fatfile) * filelistcntr);
	rewinddir(mydir);
	filelistcntr = 0;
	while((entry = readdir(mydir))) {
		if(strncmp(entry->d_name, ".", 1) == 0 || strncmp(entry->d_name, "..", 2) == 0)
		continue;
		else
		{	
			themefile[filelistcntr].iscsm = false;
			themefile[filelistcntr].isapp = false;
			
			strcpy(themefile[filelistcntr].name, entry->d_name);
			if(!strcasecmp(entry->d_name+strlen(entry->d_name)-4, ".csm"))
				themefile[filelistcntr].iscsm = true;
			if(!strcasecmp(entry->d_name+strlen(entry->d_name)-4, ".app"))
				themefile[filelistcntr].isapp = true;
			filelistcntr += 1;
		}
	}
	qsort(themefile, filelistcntr, sizeof(Fatfile), theme_entrycmp);
	closedir(mydir);
	printf("Complete !");
	
    return filelistcntr;
}

void set_highlight(bool highlight) {
    if (highlight)
    {
        printf("\x1b[%u;%um", 47, false);
        printf("\x1b[%u;%um", 30, false);
    }
    else
    {
        printf("\x1b[%u;%um", 37, false);
        printf("\x1b[%u;%um", 40, false);
    }
}
int theme_ios_menu(int default_ios) {
    u32 buttons;// = wpad_waitbuttons();
    int selection = 0;
    u32 ioscount;
    u8 *list = get_ioslist(&ioscount);

    int i;
    for(i = 0; i < ioscount; i++)
    {
        // Default to default_ios if found, else the loaded IOS
        if(list[i] == default_ios) {
            selection = i;
            break;
        }
        if(list[i] == IOS_GetVersion()) {
            selection = i;
        }
    }

    for(;;)
    {
        con_clear();
		//printf("\x1B[%d;%dH",4,1);	// move console cursor to y/x
		printf("  Current IOS : %d_r%d						 System Menu : %s_%s\n\n", IOS_GetVersion(), IOS_GetRevision(), getsysvernum(systemmenuVersion), getregion(systemmenuVersion));
        printf("\t\tIt is recommended to choose an IOS with NAND\n");
        printf("\t\tpermissions patched, like IOS249(d2x56) .\n\n");
		printf("\t\tSelect the IOS you want to load: IOS     \b\b\b\b");
        set_highlight(true);
        printf(" %u \n\n", list[selection]);
        set_highlight(false);
		printf("\t\t[Left]/[Right] Toggle Ios .\n");
		printf("\t\t[A] Select Ios .\t[B] Return to Device Menu .\n");
		printf("\t\t[Home] Return to System Menu .\n\n");
        buttons = wpad_waitbuttons();
		if(buttons == BUTTON_HOME) sys_loadmenu();
        if(buttons == BUTTON_LEFT) { //|| buttons == PAD_BUTTON_LEFT)
            if (selection > 0) selection -= 1;
            else selection = ioscount - 1;
        }
        if(buttons == BUTTON_RIGHT) { //|| buttons == PAD_BUTTON_RIGHT)
            if (selection < ioscount - 1) selection += 1;
            else selection = 0;
        }
        if(buttons == BUTTON_A) break; //|| buttons == PAD_BUTTON_A) break;
		if(buttons == BUTTON_B) {
			filecnt = 0, start = 0, selected = 0;
			filecnt = filelist_retrieve();
			return 0;
		}
    }
    return list[selection];
}
void theme_device_menu() {
	int device = 1;
	u32 buttons;
	
	for(;;) {
		con_clear();
		printf("  Current IOS : %d_r%d						 System Menu : %s_%s\n\n", IOS_GetVersion(), IOS_GetRevision(), getsysvernum(systemmenuVersion), getregion(systemmenuVersion));
		printf("\t\tSelect source device :      \b\b\b\b");
		set_highlight(true);
		printf(" %s \n\n\n\n\n\n\n\n", device_Name(device));
		set_highlight(false);
		printf("\t\t[Left]/[Right] Toggle Device .\n");
		printf("\t\t[A] Select Device .\t[B] Return to HBC .\n");
		printf("\t\t[Home] Return to System Menu .\n\n");
		
		buttons = wpad_waitbuttons();
		
		if(buttons == BUTTON_B) {
			printf("\t\tExiting ..... \n");
			sleep(1);
			sysHBC();//exit(0);
		}
		if(buttons == BUTTON_A) break;
		if(buttons == BUTTON_HOME) sys_loadmenu();
		if(buttons == BUTTON_LEFT) {
			device -= 1;
			if(device <= 0) device = DEVICES_MAX;
		}
		if(buttons == BUTTON_RIGHT) {
			device += 1;
			if(device >= 4) device = 1;
		}
	}
	
	printf("\t\tMounting %s ..... ", device_Name(device));
	if(debug) Fat_Unmount(1);
	fatdevicemounted = Fat_Mount(device);
	if(fatdevicemounted < 0) {
		printf("failed .\n\n");
		printf("\t\t[-]   Unable to mount %s .\n", device_Name(device));
		printf("\t\tPress any button to continue .");
		buttons = wpad_waitbuttons();
	}
	else printf("complete . \n\n");
	
	return;
}
void theme_manage_menu() {
	f32 sizeoffile;
	char filepath[256];
	FILE *tmpfile;
	u32 buttons, size;
	int success;
	char *Actions[3] = {"Install  ", "Uninstall", "Delete   "};
	char *confirmActions[2] = {"No ", "Yes"};
	int action = 0;
	int confirmaction = 0;
	
	sprintf(filepath, "%s:themes/%s", device_Name(fatdevicemounted), themefile[selected].name);
	tmpfile = fopen(filepath, "rb");
	if(tmpfile != null) {
		size = filesize(tmpfile);
		fclose(tmpfile);
		themefile[selected].size = size;
	}
	sizeoffile = themefile[selected].size/MB_SIZE;
	
	for(;;) {
		con_clear();
		printf("  Current IOS : %d_r%d						 System Menu : %s_%s\n\n", IOS_GetVersion(), IOS_GetRevision(), getsysvernum(systemmenuVersion), getregion(systemmenuVersion));
		printf("\t\t[+] Theme :\t%s\n", themefile[selected].name);
		printf("\t\t - File size :\t%.2f MB\n\n", sizeoffile);
		printf("\t\t[*] Action :           \b\b\b\b\b\b\b\b\b");
		set_highlight(true);
		printf(" %s ", Actions[action]);
		set_highlight(false);
		printf(" Theme .\n\n");
		printf("\t\t[Left]/[Right] Toggle Action .\n");
		printf("\t\t[A]          \b\b\b\b\b\b\b\b\b");
		printf("%s", Actions[action]);
		printf(" Theme .\t[B] Return to Theme Menu .\n");
		printf("\t\t[Home] Return to System Menu .\n");
		
		buttons = wpad_waitbuttons();
		
		if(buttons == BUTTON_B) return;
		if(buttons == BUTTON_A) break;
		if(buttons == BUTTON_HOME) sys_loadmenu();
		if(buttons == BUTTON_LEFT) {
			action -= 1;
			if(action < 0) action = 2;
		}
		if(buttons == BUTTON_RIGHT) {
			action += 1;
			if(action > 2) action = 0;
		}
	}
	
	con_clear();
	if(action == 2) {
		
		for(;;) {
			con_clear();
			printf("  Current IOS : %d r%d						 System Menu : %s_%s\n\n", IOS_GetVersion(), IOS_GetRevision(), getsysvernum(systemmenuVersion), getregion(systemmenuVersion));
			printf("\t\tDelete Theme %s . Are you sure ?     \b\b\b", themefile[selected].name);
			set_highlight(true);
			printf(" %s \n\n", confirmActions[confirmaction]);
			set_highlight(false);
			
			printf("\t\t[Left]/[Right] Toggle Action .\n");
			printf("\t\t[A] Select Action .\t[B] Return to Theme Menu .\n");
			printf("\t\t[Home] Return to System Menu .\n");
			buttons = wpad_waitbuttons();
			if(buttons == BUTTON_B) return;
			if(buttons == BUTTON_A) break;
			if(buttons == BUTTON_HOME) sys_loadmenu();
			if((buttons == BUTTON_LEFT) || (buttons == BUTTON_RIGHT)) {
				confirmaction ^= 1;
			}
		}
		if(confirmaction) {
			remove(filepath);
			filecnt = 0, start = 0, selected = 0;
			filecnt = filelist_retrieve();
		}
		return;
	}
	currenttheme.version = systemmenuVersion;
	currenttheme.region = currentthemeregion();
	
	if(action == 0) { 
		printf("Installing %s ..... \n\n", themefile[selected].name);
		themefile[selected].version = findinstallthemeversion(themefile[selected].name);
		themefile[selected].region = installregion(themefile[selected].version);
	}
	//printf("cur .version(%d) .region(%c) \n", currenttheme.version, currenttheme.region);
	//printf("inst .version(%d) .region(%c) \n", themefile[selected].version, themefile[selected].region);
	if(action == 0) sprintf(filepath, "%s:themes/%s", device_Name(fatdevicemounted), themefile[selected].name);
	else if(action == 1) sprintf(filepath, "%s:/themes/%s", device_Name(fatdevicemounted), getsavename(systemmenuVersion));
	//printf("path [%s]\n", filepath);
	if(action == 1) {
		if(!Fat_CheckFile(filepath)) {
			success = downloadApp();
			if(success <= 0) {
				printf("unable to download .\n\nPress any button to continue .");
				buttons = wpad_waitbuttons();
				return;
			}
		}
		strcpy(themefile[selected].name, getsavename(systemmenuVersion));
		themefile[selected].version = findinstallthemeversion(themefile[selected].name);
		themefile[selected].region = installregion(themefile[selected].version);
		con_clear();
		printf("Installing %s - Original System Menu Theme ..... \n\n", themefile[selected].name);
	}
	if((currenttheme.version != themefile[selected].version) || (currenttheme.region != themefile[selected].region)) {
		if(currenttheme.version != themefile[selected].version) 
        printf("\n\nInstall can not continue !\nThe install theme version is not a match\nfor the system menu version .\n\nPlease press any button to Exit to HBC !\n");
		if(currenttheme.region != themefile[selected].region)
		printf("\n\nInstall can not continue !\nThe install theme region is not a match\nfor the system menu region .\n\nPlease press any button to Exit to HBC !\n");
		wpad_waitbuttons();
        exit(0);
	}
	tmpfile = fopen(filepath, "rb");
	if(!tmpfile) {
		printf("\t\tunable to open %s .\n", filepath);
		return;
	}	
	InstallFile(tmpfile);
	fclose(tmpfile);
	
	if(action == 0) printf("\nInstalling %s ..... Complete .\n\n", themefile[selected].name);
	else if(action == 1) printf("\nInstalling %s - Original System Menu Theme .... Complete .\n\n", themefile[selected].name);
	printf("Press any button to exit to System Menu . \n");
	buttons = wpad_waitbuttons();
	sys_loadmenu();
}
void theme_list_menu() {
	u32 cnt, buttons;
	s32 index;
	int defaultios = 249, success = 0, ios = 0;
	char filepath[256];
	FILE *tmpfile;
	
	if(fatdevicemounted <= 0) return;
	
	filecnt = filelist_retrieve();
	
	
	
	for(;;) {
		con_clear();
		printf("\n  Current IOS : %d_r%d						 System Menu : %s_%s\n", IOS_GetVersion(), IOS_GetRevision(), getsysvernum(systemmenuVersion), getregion(systemmenuVersion));
		printf("\t\t[ %d ] Theme files . Select a Theme :\n\n", filecnt);
		if(!filecnt) {
			printf("\t\t[-] No Files Found .\n");
			return;
		}
		else
		for (cnt = start; cnt < filecnt; cnt++) {
            // Files per page limit 
            if ((cnt - start) >= FILES_PER_PAGE)
                break;

            // Selected file 
            (cnt == selected) ? printf("\t\t -> ") : printf("\t\t    ");
            fflush(stdout);

            // Print filename
            printf(" %s \n", themefile[cnt].name);
        }
		printf("\n");
		printf("\t\t[Up]/[Down]/[Left]/[Right] Toggle Theme .\n");
		printf("\t\t[A] Select Theme .  [B] Select Device Menu .\n");
		printf("\t\t[Minus-] Reload Ios .\n");
		printf("\t\t[Plus+] Download/Install Original Menu .\n");
		printf("\t\t[Home] Return to System Menu .\n");
		
		buttons = wpad_waitbuttons();
		
		if (buttons == BUTTON_UP) selected -= 1;
		else if (buttons == BUTTON_DOWN) selected += 1;
		else if (buttons == BUTTON_B) {
			filecnt = 0, start = 0, selected = 0;
			Fat_Unmount(fatdevicemounted);
			return;
		}
		else if (buttons == BUTTON_A) {
			theme_manage_menu(themefile[selected].name);
		}
		else if(buttons == BUTTON_HOME) sys_loadmenu();
		else if(buttons == BUTTON_LEFT) selected -= 9;
		else if(buttons == BUTTON_RIGHT) selected += 9;
		if(buttons == BUTTON_MINUS) {
			filecnt = 0, start = 0, selected = 0;
			int tmpdevice = fatdevicemounted;
			
			con_clear();
			ios = theme_ios_menu(defaultios);
			if(ios != 0) {
				Fat_Unmount(fatdevicemounted);
				Wpad_Disconnect();
				ISFS_Deinitialize();
				IOS_ReloadIOS(ios);
				if(AHBPROT_DISABLED) {
					IOSPATCH_AHBPROT();
					IOSPATCH_Apply();
				}
				wpad_init();
				//PAD_Init();
				ISFS_Initialize();
				fatdevicemounted = Fat_Mount(tmpdevice);
				if(fatdevicemounted <= 0) break;
				themefile = NULL;
				filecnt = filelist_retrieve();
			}
		}
		if(buttons == BUTTON_PLUS) {
			success = downloadApp();
			if(success <= 0) {
				printf("unable to download .\n\nPress any button to continue .");
				buttons = wpad_waitbuttons();
				return;
			}
			currenttheme.version = systemmenuVersion;
			currenttheme.region = currentthemeregion();
			sprintf(filepath, "%s:/themes/%s", device_Name(fatdevicemounted), getsavename(systemmenuVersion));
			strcpy(themefile[selected].name, getsavename(systemmenuVersion));
			themefile[selected].version = findinstallthemeversion(themefile[selected].name);
			themefile[selected].region = installregion(themefile[selected].version);
			con_clear();
			printf("Installing %s - Original System Menu Theme ..... \n\n", themefile[selected].name);
			if((currenttheme.version != themefile[selected].version) || (currenttheme.region != themefile[selected].region)) {
				if(currenttheme.version != themefile[selected].version) 
				printf("\n\nInstall can not continue !\nThe install theme version is not a match\nfor the system menu version .\n\nPlease press any button to Exit to HBC !\n");
				if(currenttheme.region != themefile[selected].region)
				printf("\n\nInstall can not continue !\nThe install theme region is not a match\nfor the system menu region .\n\nPlease press any button to Exit to HBC !\n");
				wpad_waitbuttons();
				sysHBC();
			}
			tmpfile = fopen(filepath, "rb");
			if(!tmpfile) {
				printf("\t\tunable to open %s .\n", filepath);
				return;
			}	
			InstallFile(tmpfile);
			fclose(tmpfile);
			printf("\nInstalling %s - Original System Menu Theme .... Complete .\n\n", themefile[selected].name);
			printf("Press any button to exit to Device Menu . \n");
			buttons = wpad_waitbuttons();
			filecnt = 0, start = 0, selected = 0;
			filecnt = filelist_retrieve();
			return;
		}
		if (selected <= -1)
			selected = filecnt - 1;
		if (selected >= filecnt)
			selected = 0;

		// List scrolling 
		index = (selected - start);

		if (index >= FILES_PER_PAGE)
			start += index - (FILES_PER_PAGE - 1);
		if (index <= -1)
			start += index;
	}
	
	return;
}
bool checkforpriiloader() {
	dirent_t *priiloaderfiles = NULL;
	u32 nandfilecnt;
	int filecntr;
	char *searchstr;
	
	searchstr = "title_or.tmd";
	getdir("/title/00000001/00000002/content",&priiloaderfiles,&nandfilecnt);
	for(filecntr = 0; filecntr < nandfilecnt; filecntr++) {
		if(strcmp(priiloaderfiles[filecntr].name, searchstr) == 0)
		return true;
	}
	return false; 
}
void menu_loop() { 
	
	if(debug) {
		fatdevicemounted = Fat_Mount(1);
	}
	
	if(!checkforpriiloader()) {
		
		printf("\t\tPriiloader not detected ! Press any button to exit .");
		u32 buttons = wpad_waitbuttons();
		exit(0);
	}
	
	systemmenuVersion = GetSysMenuVersion();
	if(systemmenuVersion > 518) {
		// check installed .app file if custom version number
		systemmenuVersion = checkcustomsystemmenuversion();
	}
	
	con_clear();
	printf("  Current IOS : %d_r%d						 System Menu : %s_%s\n\n", IOS_GetVersion(), IOS_GetRevision(), getsysvernum(systemmenuVersion), getregion(systemmenuVersion));
	printf("\t\t[*] Welcome to MyMenuifyMod!\n\n\t\tBuilt for all System menu versions 4.0 - 4.3!\n\n");
	sleep(2);
	
	for(;;) {
		theme_device_menu();
		
		theme_list_menu();
	}
}
int main(int argc, char **argv) {
	
	__exception_setreload(5);
	
	//int ios = IOS_GetVersion();
	//IOS_ReloadIOS(ios);
	if(AHBPROT_DISABLED) {
		IOSPATCH_AHBPROT();
		IOSPATCH_Apply();
	}
	/* Initialize subsystems */
    sys_init();
	 
    /* Set video mode */
    video_setmode();

    /* Initialize console */
    con_init(CONSOLE_XCOORD, CONSOLE_YCOORD, CONSOLE_WIDTH, CONSOLE_HEIGHT);
	
	// Draw banner
    show_banner();
	
	// Initialize Wiimote 
	wpad_init();
	//PAD_Init();
	
	ISFS_Initialize();
	
	Disclaimer();
	
	menu_loop();
	
	return 0;
}
