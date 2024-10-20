#include <stdio.h>
#include <stdlib.h>
#include <ogcsys.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <sys/dir.h>
#include <network.h>
#include <ogc/ios.h>

#include "debug.h"
#include "wpad.h"
#include "IPLFontWrite.h"
#include "video.h"
#include "fat_debug.h"
#include "fat2.h"
#include "libpng/pngu/pngu.h"
#include "grfx/mymenuifymod_png.h"
#include "iospatch.h"
#include "http.h"
#include "rijndael.h"
#include "network.h"
#include "gecko.h"

#define KNOWN_THEME_CONTENTS   18

#define HBC_HAXX    0x0001000148415858LL
#define HBC_JODI    0x000100014A4F4449LL
#define HBC_1_0_7   0x00010001AF1BF516LL
#define HBC_1_0_8   0x00010001af1bf516LL 
#define HBC_LULZ    0x000100014C554C5ALL
#define Priiloader  0x0000000100000002LL

#define MAX_FILELIST_LEN	1024
#define MAX_FILEPATH_LEN	256

#define MB_SIZE		        1048576.0
#define FILES_PER_PAGE		5

#define BLOCK_SIZE	0x1000
#define CHUNKS 1000000
#define MAX_SIZE_HTTP 0xFFFFFFFF


IMGCTX ctx;
int fatdevicemounted = 0;
bool Debugger;
GXRModeObj *vmode = NULL;
u32 *xfb[2] = { NULL, NULL };
int whichfb = 0;
u32 systemmenuVersion;
const char *themedir = "themes";
static s32 filecnt = 0, start = 0, selected = 0;
char textbuf[2048] = "";
char textbuf2[2048] = "";
u8 wii_common_key[16] = { 0xeb, 0xe4, 0x2a, 0x22, 0x5e, 0x85, 0x93, 0xe4, 0x48,0xd9, 0xc5, 0x45, 0x73, 0x81, 0xaa, 0xf7 };
u8 vWii_common_key[16] = { 0x30, 0xBF, 0xC7, 0x6E, 0x7C, 0x19, 0xAF, 0xBB, 0x23, 0x16, 0x33, 0x30, 0xCE, 0xD7, 0xC2, 0x8D };
typedef struct _dirent{
	char name[ISFS_MAXPATH + 1];
	int type;
	u32 ownerID;
	u16 groupID;
	u8 attributes;
	u8 ownerperm;
	u8 groupperm;
	u8 otherperm;
} dirent_t;
dirent_t *nandfilelist;
typedef struct{
	char name[128];
	char region;
	u32 version;
	u32 size;
}themeInfo;

typedef struct{
	char name[128];
	char region;
	u32 version;
	u32 size;
	bool iscsm;
	bool isapp;
}Fatfile;
bool disable_Disclaimer;
bool disable_wDance;
Fatfile *themefile = NULL;
themeInfo currentTheme;
u32 known_Versions[KNOWN_THEME_CONTENTS] = {416, 417, 418, 448, 449, 450, 454, 480, 481, 482, 486, 512, 513, 514, 518, 608, 609, 610};
char *regions[KNOWN_THEME_CONTENTS] =      {"J", "U", "E", "J", "U", "E", "K", "J", "U", "E", "K", "J", "U", "E", "K", "J", "U", "E"};
char *knownappfilenames[KNOWN_THEME_CONTENTS] = {"0000006f.app", "00000072.app", "00000075.app", "00000078.app", "0000007b.app", "0000007e.app", "00000081.app", "00000084.app", "00000087.app", "0000008a.app", "0000008d.app", "00000094.app", "00000097.app", "0000009a.app", "0000009d.app", "0000001c.app", "0000001f.app", "00000022.app"};
static vu32 *_wiilight_reg = (u32*) 0xCD0000C0;
const char *wiishoppath = "http://nus.cdn.shop.wii.com/ccs/download";             // Nus Wii
const char *wiiU_shoppath = "http://ccs.cdn.wup.shop.nintendo.net/ccs/download"; // Nus Wii U
const char *RC24path = "http://ccs.cdn.sho.rc24.xyz/ccs/download";                // Nus Emu RC24
bool priiloader_found = false;
void pngu_free_info(IMGCTX ctx);
extern void __exception_setreload(int);
void sleep(int);
void usleep(int);
char *theme_ID[] = { "AMONG1",
"AMONG2",
"ANML01",
"ANML02",
"APPLE1",
"ATHF01",
"BKGN01",
"BTMN01",
"BTMN02",
"BIGG01",
"BILLY1",
"BLKGD1",
"BLMG01",
"BLPR01",
"BLCH01",
"BOBO01",
"BDSTS1",
"BWSR01",
"BRLY01",
"CODTY1",
"CAR001",
"CARS01",
"CKMO01",
"CRTRG1",
"CLKWK1",
"CLBPN1",
"GEASS1",
"CONDT1",
"CONST1",
"DKUB01",
"DKUB02",
"DWORI1",
"DWBLE1",
"DWBLU1",
"DWBLJ1",
"DWBLK1",
"DWGRE1",
"DWORE1",
"DWPKE1",
"DWPRE1",
"DWRDE1",
"DWWHE1",
"DWYLE1",
"DKLOK1",
"DIABL1",
"DSCRD1",
"DGMN01",
"DBLZ01",
"DBLZ02",
"DRWHO1",
"DGENX1",
"EGLES1",
"ETHBD1",
"ETHBD2",
"EMOBL1",
"EMOGR1",
"EMOPK1",
"EMOPR1",
"EMORD1",
"EVDED1",
"EXBOT1",
"EYES01",
"FODDP1",
"FMGUY1",
"FANTA1",
"FCLUB1",
"FFVII1",
"FIRE01",
"FLOPO1",
"FNFNK1",
"F13TH1",
"FMTLE1",
"FUTUR1",
"GAARA1",
"GRFLD1",
"GOWAR1",
"GBUST1",
"GSUN01",
"GOTH01",
"GRTFL1",
"HDRAW1",
"HEMAN1",
"HKITY1",
"HELLK1",
"HEROS1",
"HNDRD1",
"ILLOG1",
"IMPOR1",
"ICP001",
"INBET1",
"IMMAR1",
"JSRAD1",
"JNUTR1",
"JOKER1",
"JRPRK1",
"KDICR1",
"KHRT01",
"KIRBY1",
"KIRBY2",
"KISS01",
"KORN01",
"LEOPA1",
"LIME01",
"LTOON1",
"LOST01",
"LUIGI1",
"LUIGI2",
"MWRLD1",
"MWRLD2",
"MAMSK1",
"MNMS01",
"MNHNT1",
"MARIA1",
"MARIO2",
"MKART1",
"MABEL1",
"MATRX1",
"MATRX2",
"MEGMN1",
"MTLCA1",
"MGSOL1",
"MTROD1",
"MTDSV1",
"MISTF1",
"MKOMB1",
"MUSE01",
"NARTO1",
"NMB4X1",
"NIDRM1",
"NOMRH1",
"OKAMI1",
"OSNIN1",
"OTLWS1",
"PLJAM1",
"POMAD1",
"PHWRT1",
"PIKMN1",
"PKFLD1",
"PNKWI1",
"PRSKL1",
"PREDR1",
"PARIE1",
"PARIE2",
"PARIE3",
"PSYCO1",
"PNOUT1",
"PUNSH1",
"ORTON1",
"RCLNK1",
"RC2401",
"RHYTH1",
"RMORT1",
"RCHCK1",
"RBAND1",
"SAW001",
"SHADH1",
"SILVH1",
"SMASH1",
"SNOOP1",
"SNCFT1",
"SNCRD1",
"STHPK1",
"SPAWN1",
"SPDMN1",
"SPONG1",
"SQUBL1",
"STCFT1",
"STGTE1",
"STWRS1",
"STWRS2",
"STWII1",
"STRME1",
"STRFT1",
"SHSQU1",
"SMRPG1",
"SSONI1",
"TAILS1",
"TERMR1",
"SIMPS1",
"SIMPS2",
"SIMPS3",
"TCATS1",
"TMNT01",
"TRAID1",
"TTOON1",
"TOYST1",
"TRANS1",
"TRGUN1",
"TRPTL1",
"TBLOD1",
"UDWII1",
"VEGET1",
"VISTA1",
"WALEY1",
"WARIO1",
"WSTRI1",
"WHITE1",
"WIID01",
"WIIPT1",
"WIIPT2",
"WIFIT1",
"WSPOR1",
"WIIU01",
"WINXP1",
"WOLVE1",
"WWERW1",
"XBOX01",
"YUGIO1",
"ZELDA1",
"ZELDA2",
"ZOMB01",
};
char *theme_Name[] = { "Among Us v1",
"Among Us v2",
"Animal Crossing",
"Animal Crossing v2",
"Apple",
"Aqua Teen Hunger Force",
"Bakugan",
"Batman v1",
"Batman v2",
"Notorious B.I.G.",
"Billy Mays",
"Black Gold",
"Black Mage",
"Black Pirate",
"Bleach",
"BoBoBo",
"Boondock Saints",
"Bowser",
"Broly",
"Call of Duty",
"Car",
"Cars",
"Check Mii Out",
"Chrono Trigger",
"Clock Work Orange",
"Club Penguin",
"Code Geass",
"Conduit",
"Constantine",
"Dark Umbra v1",
"Dark Umbra v2",
"Dark Wii Original",
"Dark Wii Blue E",
"Dark Wii Blue U",
"Dark Wii Blue J",
"Dark Wii Blue K",
"Dark Wii Green",
"Dark Wii Orange",
"Dark Wii Pink",
"Dark Wii Purple",
"Dark Wii Red",
"Dark Wii White",
"Dark Wii Yellow",
"Deth Klok",
"Diablo 3",
"Discord",
"Dog Man",
"Dragon Ball Z v1",
"Dragon Ball Z v2",
"Dr Who",
"De-Generation X",
"Eagles",
"Earth Bound",
"Earth Bound v2",
"Emo Blue",
"Emo Green",
"Emo Pink",
"Emo Purple",
"Emo Red",
"Evil Dead",
"Excite Bots",
"Eyes",
"Fairly Odd Parents",
"Family Guy",
"Fantasy",
"Fight Club",
"Final Fantasy 7",
"Fire Wii",
"Flower Power",
"Friday Night Funkin",
"Friday the 13th",
"Full Metal Alchemist",
"Futurama",
"Gaara",
"Garfield",
"Gears of War",
"Ghost Busters",
"Golden Sun",
"Gothic",
"Grateful Dead",
"Hand Drawn",
"Hello Kitty",
"Hell's Kitchen",
"He-Man",
"Heros",
"The Hundreds",
"Illusions of Gaia",
"Imports",
"Insane Clown Posse",
"In Betweeners",
"Its A Me Mario",
"Jet Set Radio",
"Jimmy Neutron",
"Joker",
"Jurassic Park 3",
"Kid Icarus",
"Kingdom Hearts",
"Kirby",
"Kirby Adventures",
"Kiss",
"Korn",
"Leopard OS",
"Lime Wii",
"Looney Toons",
"Lost",
"Luigi v1",
"Luigi v2",
"Mad World",
"Mad World v2",
"Majoras Mask",
"M and M's",
"Man Hunt",
"Maria",
"Mario",
"Mario Kart",
"Martin Abel Art",
"Matrix",
"Matrix Reloaded",
"MegaMan",
"Metal Gear Solid",
"Metallica",
"Metroid",
"Metroid: Samus's Visor",
"Mist Forest",
"Mortal Kombat",
"Muse",
"Naruto",
"Nightmare B4 Xmas",
"Nights into Dreams",
"No More Heros",
"Okami",
"Old School Nintendo",
"Outlaw Star",
"Pearl Jam",
"Penguins of Madagascar",
"Phoenix Wright",
"Pikmin",
"Pink Floyd",
"Pink Wii",
"Pirate Skulls",
"Predator",
"Princess Ariel v1",
"Princess Ariel v2",
"Princess Ariel v3",
"Psychedelic",
"Punch Out",
"The Punisher",
"Randy Orton",
"Ratchet and Clank",
"Reconnect 24 Red",
"Rhythm Heaven",
"Rick and Morty",
"Robot Chicken",
"Rockband 2",
"Saw",
"Shadow The Hedgehog",
"Silver The Hedgehog",
"Smash Brothers Brawl",
"Snoopy",
"Sonic Frontiers",
"Sonic Riders",
"South Park",
"Spawn",
"Spiderman",
"SpongeBob",
"Squid Billies",
"StarCraft",
"Star Gate",
"Star Wars",
"Star Wars Unleashed",
"Steel Wii",
"Storms",
"Street Fighter",
"Super Hero Squad",
"Super Mario RPG",
"Super Sonic",
"The Simpsons v1",
"The Simpsons v2",
"The Simpsons v3",
"Tails",
"The Terminator",
"Thunder Cats",
"Teenage Mutant Ninja Turtles",
"Tomb Raider",
"Toxic Toons",
"Toy Story",
"Transformers",
"Tri-Gun",
"Tropical Teal",
"True Blood",
"Ultimate Dark Wii",
"Vegeta",
"Vista",
"Walleye",
"Wario Ware",
"White Stripes",
"White Wii",
"Wiid",
"Wii Fit",
"Wii Party",
"Wii Party v2",
"Wii Sports",
"Wii U",
"Win XP OS",
"Wolverine",
"WWE Raw",
"Xbox 360",
"Yugi-oh",
"Zelda",
"Zelda: A Link to the Past",
"ZombWii",
};

void wiilight(int enable) {
    u32 val = (*_wiilight_reg & ~0x20);
    if (enable) val |= 0x20;
    *_wiilight_reg = val;
}
void system_exit_Menu(void) {
	wiilight(1);
	/* Return to the Wii system menu */
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}
int system_Exit_Priiloader() {
	WII_Initialize();
	wiilight(1);
	int ret = WII_LaunchTitle(Priiloader);
	
	if(ret < 0)
		return ret;
	wiilight(0);
	return 0;
}
void system_exit_HBC() {
	WII_Initialize();
	wiilight(1);
    int ret = WII_LaunchTitle(HBC_1_0_7);
    if(ret < 0) WII_LaunchTitle(HBC_JODI);
    if(ret < 0) WII_LaunchTitle(HBC_HAXX);
	if(ret < 0) WII_LaunchTitle(HBC_1_0_8);
	if(ret < 0) WII_LaunchTitle(HBC_LULZ);
	wiilight(0);
    //Back to system menu if all fails
    SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}
void show_banner(void) {
    PNGUPROP imgProp;
    s32 ret;
	
	extern const uint8_t mymenuifymod_png[];
	
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
void free_Png() {
	pngu_free_info(ctx);
	return;
}
static void Initialise() {
	// Initialise the video system
	VIDEO_Init();

	// This function initialises the attached controllers
	PAD_Init();
	WPAD_Init();
	WPAD_SetIdleTimeout(120);
	//WPAD_SetPowerButtonCallback((WPADShutdownCallback) ShutdownWii);
	//SYS_SetPowerCallback(ShutdownWii);

	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	vmode = VIDEO_GetPreferredMode(NULL);

	// Set up the video registers with the chosen mode
	VIDEO_Configure(vmode);

	// Allocate memory for the display in the uncached region
	xfb[0] = (u32 *) MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));
	xfb[1] = (u32 *) MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));
	VIDEO_ClearFrameBuffer(vmode, xfb[0], COLOR_BLACK);
	VIDEO_ClearFrameBuffer(vmode, xfb[1], COLOR_BLACK);

	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(xfb[0]);

	//VIDEO_SetPostRetraceCallback(InvalidatePADS);

	// Make the display visible
	VIDEO_SetBlack(FALSE);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();

	init_font();
	whichfb = 0;
	ISFS_Initialize();
	
	return;
}
void show_W_Dance(const void *input_Png) {
    PNGUPROP imgProp;
    s32 ret;
	
	
    // Select PNG data 
    ctx = PNGU_SelectImageFromBuffer(input_Png);
    if (!ctx)
        return;

    // Get image properties 
    ret = PNGU_GetImageProperties(ctx, &imgProp);
    if (ret != PNGU_OK)
        return;

    // Draw image 
    video_drawpng(ctx, imgProp, 195, 115);

     //Free image context 
    PNGU_ReleaseImageContext(ctx);
	return;
}
void W_Dance(int dancetimes) {
	extern const uint8_t w1_png[];
	extern const uint8_t w2_png[];
	extern const uint8_t w3_png[];
	extern const uint8_t w4_png[];
	extern const uint8_t w5_png[];
	extern const uint8_t w6_png[];
	extern const uint8_t w7_png[];
	extern const uint8_t w8_png[];
	extern const uint8_t w9_png[];
	extern const uint8_t w10_png[];
	extern const uint8_t w11_png[];
	extern const uint8_t w12_png[];
	int number_of_w_pictures = 12;
	const void *W_Danincin[] = { w1_png, w2_png, w3_png, w4_png, w5_png, w6_png, w7_png, w8_png, w9_png, w10_png, w11_png, w12_png };
	
	for(int y = 0; y < dancetimes; y++)
		for(int z = 0; z < number_of_w_pictures; z++) {
			DrawFrameStart();
			show_W_Dance(W_Danincin[z]);
			if(z == number_of_w_pictures - 1 && y == dancetimes - 1) {
				DrawRawFont((320 - (strlen("MyMenuifyMod")/2)), 360, "MyMenuifyMod");
				DrawRawFont((320 - (strlen("  WELCOME   ")/2)), 390, "  WELCOME   ");
			}
			DrawFrameFinish();
			free_Png();
		}
	
	return;
}
void read_MMM_Config(int device) {
	int unmounted;
	char filepath[2048];
	FILE *meta_File;
	char file_Line[1024];
	char *line;
	
	if(Debugger) {
		if(fatdevicemounted){
			unmounted = Fat_Unmount(fatdevicemounted);
			logfile("unmount [%i]\n", unmounted);
		}
		fatdevicemounted = Fat_Mount(fatdevicemounted);
	}
	if(!fatdevicemounted) 
		fatdevicemounted = Fat_Mount(device);
	if(!fatdevicemounted) {
		//sprintf(textbuf,"config on %s mounting failed", device_Name(device));
		//WriteCentre(230, "Select Device :");
		//DrawFrameFinish();
		device = USB;
		fatdevicemounted = Fat_Mount(device);
		if(!fatdevicemounted)
			return;
	}
	if(Debugger) logfile("fatdevicemounted [%i]\n", fatdevicemounted);
	sprintf(filepath, "%s:/apps/mymenuifymod/meta.xml", device_Name(fatdevicemounted));
	if(Debugger) logfile("filepath[%s]\n", filepath);
	meta_File = fopen(filepath, "rb");
	while ((line = fgets(file_Line, sizeof(file_Line), meta_File))) {
		//logfile("Line[%s]\n", line);
		if(strstr(line, "!Disclaimer")) {
			//logfile("found string '!Disclaimer' .\n");
			disable_Disclaimer = true;
		}
		else if(strstr(line, "Disclaimer")) {
			//logfile("found string 'Disclaimer' .\n");
			disable_Disclaimer = false;
		}
		if(strstr(line, "!wDance")) {
			//logfile("found string '!wDance' .\n");
			disable_wDance = true;
		}
		else if(strstr(line, "wDance")) {
			//logfile("found string 'wDance' .\n");
			disable_wDance = false;
		}
	}
	fclose(meta_File);
	//logfile("config disable disclaimer[%i]\n", disable_Disclaimer);
	//logfile("config disable wDance[%i]\n", disable_wDance);
	//if(fatdevicemounted) unmounted = Fat_Unmount(fatdevicemounted);
	//if(Debugger) fatdevicemounted = Fat_Mount(fatdevicemounted);
	return;
}
int write_MMM_Config(int device, int mode) {
	char filepath_Meta[2048];
	char filepath_NewMeta[2048];
	FILE *meta_File;
	FILE *tmpFile;
	const char *newFile = NULL;
	
	read_MMM_Config(device);
	
	
	sprintf(filepath_Meta, "%s:/apps/mymenuifymod/meta.xml", device_Name(device));
	//logfile("filepath[%s]\n", filepath_Meta);
	meta_File = fopen(filepath_Meta, "rb");
	if(!meta_File) {
		logfile("Unable to open file [%s:/apps/mymenuifymod/meta.xml]\n", device_Name(device));
		return 0;
	}
	sprintf(filepath_NewMeta, "%s:/apps/mymenuifymod/tmp", device_Name(device));
	//logfile("filepath[%s]\n", filepath_NewMeta);
	tmpFile = fopen(filepath_NewMeta, "ab");
	if(!tmpFile) {
		logfile("Unable to open file [%s:/apps/mymenuifymod/tmp]\n", device_Name(device));
		return 0;
	}
	//logfile("mode[%i]\n", mode);
	if(mode == 1) {
		if(!disable_wDance)
			newFile = "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>\n<app version='1'>\n<name>MyMenuifyMod</name>\n<version>3.0</version>\n<coder>Scooby74029</coder>\n<short_description>Theme Installer</short_description>\n<long_description>A Theme Installer for Wii and vWii .</long_description>\n<ahb_access/>\n<!Disclaimer/>\n<wDance/>\n</app>";
		if(disable_wDance)
			newFile = "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>\n<app version='1'>\n<name>MyMenuifyMod</name>\n<version>3.0</version>\n<coder>Scooby74029</coder>\n<short_description>Theme Installer</short_description>\n<long_description>A Theme Installer for Wii and vWii .</long_description>\n<ahb_access/>\n<!Disclaimer/>\n<!wDance/>\n</app>";
	}
	if(mode == 2) {
		if(!disable_Disclaimer)
			newFile = "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>\n<app version='1'>\n<name>MyMenuifyMod</name>\n<version>3.0</version>\n<coder>Scooby74029</coder>\n<short_description>Theme Installer</short_description>\n<long_description>A Theme Installer for Wii and vWii .</long_description>\n<ahb_access/>\n<Disclaimer/>\n<!wDance/>\n</app>";
		if(disable_Disclaimer)
			newFile = "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>\n<app version='1'>\n<name>MyMenuifyMod</name>\n<version>3.0</version>\n<coder>Scooby74029</coder>\n<short_description>Theme Installer</short_description>\n<long_description>A Theme Installer for Wii and vWii .</long_description>\n<ahb_access/>\n<!Disclaimer/>\n<!wDance/>\n</app>";
	}
	if(mode == 3) {
		if(!disable_wDance)
			newFile = "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>\n<app version='1'>\n<name>MyMenuifyMod</name>\n<version>3.0</version>\n<coder>Scooby74029</coder>\n<short_description>Theme Installer</short_description>\n<long_description>A Theme Installer for Wii and vWii .</long_description>\n<ahb_access/>\n<Disclaimer/>\n<wDance/>\n</app>";
		if(disable_wDance)
			newFile = "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>\n<app version='1'>\n<name>MyMenuifyMod</name>\n<version>3.0</version>\n<coder>Scooby74029</coder>\n<short_description>Theme Installer</short_description>\n<long_description>A Theme Installer for Wii and vWii .</long_description>\n<ahb_access/>\n<Disclaimer/>\n<!wDance/>\n</app>";
	}
	if(mode == 4) {
		if(!disable_Disclaimer)
			newFile = "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>\n<app version='1'>\n<name>MyMenuifyMod</name>\n<version>3.0</version>\n<coder>Scooby74029</coder>\n<short_description>Theme Installer</short_description>\n<long_description>A Theme Installer for Wii and vWii .</long_description>\n<ahb_access/>\n<Disclaimer/>\n<wDance/>\n</app>";
		if(disable_Disclaimer)
			newFile = "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>\n<app version='1'>\n<name>MyMenuifyMod</name>\n<version>3.0</version>\n<coder>Scooby74029</coder>\n<short_description>Theme Installer</short_description>\n<long_description>A Theme Installer for Wii and vWii .</long_description>\n<ahb_access/>\n<!Disclaimer/>\n<wDance/>\n</app>";
	}
	fprintf(tmpFile, newFile);
	fclose(meta_File);
	fclose(tmpFile);
	remove(filepath_Meta);
	rename(filepath_NewMeta, filepath_Meta);
	return 1;
}
bool Disclaimer() {
	u32 buttons;
	bool user_choice;
	
	for(;;) {
		DrawFrameStart();
		WriteCentre(95, "[DISCLAIMER] :");
		WriteFont(90, 155, "THIS APPLICATION COMES WITH NO    ");
		WriteFont(90, 180, "WARRANTY AT ALL, NEITHER EXPRESSED");
		WriteFont(90, 205, "NOR IMPLIED . I DO NOT TAKE ANY   ");
		WriteFont(90, 230, "RESPONSIBILITY FOR ANY DAMAGE TO  "); 
		WriteFont(90, 255, "YOUR WII CONSOLE BECAUSE OF       ");
		WriteFont(90, 280, "IMPROPER USE OF THIS SOFTWARE .   ");
		WriteCentre(390, "[A] Continue                  [B] Exit"); 
		DrawFrameFinish();
		
		buttons = wpad_waitbuttons();
		
		if((buttons == WPAD_BUTTON_B) || (buttons == WPAD_BUTTON_HOME)) {
			user_choice = true;
			break;
		}
		if(buttons == WPAD_BUTTON_A) {
			user_choice = false;
			break;
		}
	}
	return user_choice;
}
const char *getregion(u32 num) {
    switch(num) {
    case 417:
    case 449:
    case 481:
    case 513:
	case 609:
        return "U";
        break;
    case 418:
    case 450:
    case 482:
    case 514:
	case 610:
        return "E";
        break;
    case 416:
    case 448:
    case 480:
    case 512:
	case 608:
        return "J";
        break;
    case 486:
    case 454:
    case 518:
        return "K";
        break;
    default:
		return "UnDefined";
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
	case 608:
	case 609:
	case 610:
		return "vWii";
		break;
	default:
		return "UnDefined";
		break;
	}
}
void *allocate_memory(u32 size){
	return memalign(32, (size+31)&(~31) );
}
s32 __FileCmp(const void *a, const void *b){
	dirent_t *hdr1 = (dirent_t *)a;
	dirent_t *hdr2 = (dirent_t *)b;
	
	if (hdr1->type == hdr2->type){
		return strcmp(hdr1->name, hdr2->name);
	}else{
		return 0;
	}
}
s32 getdir(char *path, dirent_t **ent, u32 *cnt){
	s32 res;
	u32 num = 0;

	int i, j, k;
	
	res = ISFS_ReadDir(path, NULL, &num);
	if(res != ISFS_OK){
		if(Debugger) logfile("Error: could not get dir entry count! (result: %d)\n", res);
		return -1;
	}

	char ebuf[ISFS_MAXPATH + 1];

	char *nbuf = (char *)allocate_memory((ISFS_MAXPATH + 1) * num);
	if(nbuf == NULL){
		if(Debugger) logfile("ERROR: could not allocate buffer for name list!\n");
		return -2;
	}

	res = ISFS_ReadDir(path, nbuf, &num);
	DCFlushRange(nbuf,13*num); //quick fix for cache problems?
	if(res != ISFS_OK){
		if(Debugger) logfile("ERROR: could not get name list! (result: %d)\n", res);
		free(nbuf);
		return -3;
	}
	
	*cnt = num;
	
	*ent = allocate_memory(sizeof(dirent_t) * num);
	if(*ent==NULL){
		if(Debugger) logfile("Error: could not allocate buffer\n");
		free(nbuf);
		return -4;
	}

	for(i = 0, k = 0; i < num; i++){	    
		for(j = 0; nbuf[k] != 0; j++, k++)
			ebuf[j] = nbuf[k];
		ebuf[j] = 0;
		k++;

		strcpy((*ent)[i].name, ebuf);
		//gprintf("Name of file (%s)\n",(*ent)[i].name);
	}
	
	qsort(*ent, *cnt, sizeof(dirent_t), __FileCmp);
	
	free(nbuf);
	return 0;
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
u32 checkcustomsystemmenuversion() {
	u32 nandfilecnt = 0, filecounter = 0, knownversioncounter = 0;
	char *knownversionstr = "";
	
	getdir("/title/00000001/00000002/content",&nandfilelist,&nandfilecnt);
	for(filecounter = 0; filecounter < nandfilecnt; filecounter++) {
		for(knownversioncounter = 0; knownversioncounter < KNOWN_THEME_CONTENTS; knownversioncounter++) {
			knownversionstr = knownappfilenames[knownversioncounter];
			if(!strcmp(nandfilelist[filecounter].name, knownversionstr)) return known_Versions[knownversioncounter];
		}
	}
	return 0;
}
void draw_System_Info(s32 ios) {
	sprintf(textbuf, "IOS: %i_v%i     System Menu: %s_%s", ios, IOS_GetRevision(), getsysvernum(systemmenuVersion), getregion(systemmenuVersion));
	DrawFrameStart();
	WriteCentre(95, textbuf);
	
	return;
}
void exit_Program() {
	const char *types[] = { "System Menu", "Home Brew Channel", "PriiLoader" };
	int type = 0;
	u32 buttons;
	s32 Ios = IOS_GetVersion();
	
	for(;;) {
		draw_System_Info(Ios);
		sprintf(textbuf,"Exit To:   %s", types[type]);
		WriteFont(60, 240, textbuf);
		WriteFont(60, 300, "[Left]/[Right] Toggle Exit");
		WriteFont(60, 360, "[A] Select Exit  [B] Back");
		DrawFrameFinish();
		buttons = wpad_waitbuttons();
		
		if(buttons == WPAD_BUTTON_A) break;
		if(buttons == WPAD_BUTTON_LEFT) {
			type--;
			if(type < 0)
				type = 2;
		}
		if(buttons == WPAD_BUTTON_RIGHT) {
			type++;
			if(type > 2)
				type = 0;
		}
		if(buttons == WPAD_BUTTON_B) { type = -1; break; }
	}
	switch(type) {
		case 0:
			draw_System_Info(Ios);
			WriteCentre(220, "MyMenuifyMod      ");
			WriteCentre(260, "Exit System Menu  ");
			DrawFrameFinish();
			sleep(1);
			system_exit_Menu();
		break;
		case 1:
		default:
			draw_System_Info(Ios);
			WriteCentre(220, "MyMenuifyMod");
			WriteCentre(260, "    Exit HBC");
			DrawFrameFinish();
			sleep(1);
			system_exit_HBC();
		break;
		case 2:
			draw_System_Info(Ios);
			WriteCentre(220, "MyMenuifyMod     ");
			WriteCentre(260, "Exit PriiLoader  ");
			DrawFrameFinish();
			sleep(1);
			system_Exit_Priiloader();
		break;
	}
	return;
}
const char *content_name_no_Extension(u32 idx) {
    switch(idx)
    {
    case 417:
        return "00000072";
        break;
    case 449:
        return "0000007b";
        break;
    case 481:
        return "00000087";
        break;
    case 513:
        return "00000097";// usa
        break;
	case 609:
		return "0000001f";// usa
		break;
    case 418:
        return "00000075";
        break;
    case 450:
        return "0000007e";
        break;
    case 482:
        return "0000008a";
        break;
    case 514:
        return "0000009a";// pal
        break;
	case 610:
		return "00000022";// pal
		break;
    case 416:
        return "00000070";
        break;
    case 448:
        return "00000078";
        break;
    case 480:
        return "00000084";
        break;
    case 512:
        return "00000094";// jpn
        break;
	case 608: 
		return "0000001c";// jpn
		break;
    case 486:
        return "0000008d";// kor
        break;
    case 454:
        return "00000081";
        break;
    case 518:
        return "0000009d";// kor
        break;
    default:
        return "UNKNOWN";
        break;
    }
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
        return "00000097.app";
        break;
	case 609:
		return "0000001f.app";// usa
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
        return "0000009a.app";
        break;
	case 610:
		return "00000022.app";// pal
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
        return "00000094.app";
        break;
	case 608: 
		return "0000001c.app";// jpn
		break;
    case 486:
        return "0000008d.app";
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
void get_title_key(signed_blob *s_tik, u8 *key, bool is_vWii) {
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
	if(!is_vWii)
		aes_set_key(wii_common_key);
	else
		aes_set_key(vWii_common_key);
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
        return "0000006f";
        break;
    case 3:
        return "00000072";
        break;
    case 4:
        return "00000075";
        break;
    case 5:
        return "00000078";
        break;
    case 6:
        return "0000007b";
        break;
    case 7:
        return "0000007e";
        break;
    case 8:
        return "00000081";
        break;
    case 9:
        return "00000084";
        break;
    case 10:
        return "00000087";
        break;
    case 11:
        return "0000008a";
        break;
    case 12:
        return "0000008d";
        break;
    case 13:
        return "00000094";
        break;
    case 14:
        return "00000097";
        break;
    case 15:
        return "0000009a";
        break;
    case 16:
        return "0000009d";
        break;
	case 17:
		return "0000001c";
		break;
	case 18: 
		return "0000001f";
		break;
	case 19: 
		return "00000022";
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
        return 2;
        break;
    case 417:
        return 3;
        break;
    case 418:
        return 4;
        break;
    case 448:
        return 5;
        break;
    case 449:
        return 6;
        break;
    case 450:
        return 4;
        break;
    case 454:
        return 8;
        break;
    case 480:
        return 9;
        break;
    case 481:
        return 10;
        break;
    case 482:
        return 11;
        break;
    case 486:
        return 12;
        break;
    case 512:
        return 13;
        break;
    case 513:
        return 14;
        break;
    case 514:
        return 15;
        break;
    case 518:
        return 16;
        break;
	case 608:
		return 17;
		break;
	case 609:
		return 18;
		break;
	case 610:
		return 19;
		break;
    default:
        return -1;
        break;
    }
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
        logfile("ES_GetNumTitles: Error! (result = %d)\n", res);
        return 0;
    }
    buf = memalign(32, sizeof(u64) * tcnt);
    res = ES_GetTitles(buf, tcnt);
    if(res < 0) {
        logfile("ES_GetTitles: Error! (result = %d)\n", res);
        if (buf) free(buf);
        return 0;
    }

    icnt = 0;
    for(i = 0; i < tcnt; i++) {
        if(*((u32 *)(&(buf[i]))) == 1 && (u32)buf[i] > 200 && (u32)buf[i] < 252) {
            icnt++;
            ioses = (u8 *)realloc(ioses, sizeof(u8) * icnt);
            ioses[icnt - 1] = (u8)buf[i];
        }
    }

    ioses = (u8 *)malloc(sizeof(u8) * icnt);
    icnt = 0;

    for(i = 0; i < tcnt; i++) {
        if(*((u32 *)(&(buf[i]))) == 1 && (u32)buf[i] > 200 && (u32)buf[i] < 252) {
            icnt++;
            ioses[icnt - 1] = (u8)buf[i];
        }
    }
    free(buf);
    qsort(ioses, icnt, 1, __u8Cmp);

    *cnt = icnt;
    return ioses;
}
s32 theme_ios_menu(s32 default_ios) {
    u32 buttons;
    s32 selected_Ios = 0;
    u32 ioscount;
    u8 *list = (u8*)get_ioslist(&ioscount);
	s32 Ios = IOS_GetVersion();
	
    int i;
    for(i = 0; i < ioscount; i++)
    {
        // Default to default_ios if found, else the loaded IOS
        if(list[i] == default_ios) {
            selected_Ios = i;
            break;
        }
        if(list[i] == IOS_GetVersion()) {
            selected_Ios = i;
        }
    }

    for(;;)
    {
        draw_System_Info(Ios);
        WriteFont(40, 120, "It is recommended to choose an IOS ");
        WriteFont(40, 150, "with NAND permissions patched .(ex. 249)");
		sprintf(textbuf, "Select the IOS you want to load: IOS_%u", list[selected_Ios]);
		WriteCentre(275, textbuf);
        
		WriteFont(60, 350, "[Left]/[Right] Toggle Ios .");
		WriteFont(60, 375, "[A] Select Ios .          [B] Back .");
		WriteFont(60, 400, "[Home] Exit to  .");
		DrawFrameFinish();
        buttons = wpad_waitbuttons();
		if(buttons == WPAD_BUTTON_HOME) exit_Program();
        if(buttons == WPAD_BUTTON_LEFT) { //|| buttons == PAD_BUTTON_LEFT)
            if (selected_Ios > 0) selected_Ios--;
            else selected_Ios = ioscount - 1;
        }
        if(buttons == WPAD_BUTTON_RIGHT) { //|| buttons == PAD_BUTTON_RIGHT)
            if (selected_Ios < ioscount - 1) selected_Ios++;
            else selected_Ios = 0;
        }
        if(buttons == WPAD_BUTTON_A) break; //|| buttons == PAD_BUTTON_A) break;
		if(buttons == WPAD_BUTTON_B) {
			//filecnt = 0, start = 0, selected = 0;
			//filecnt = filelist_retrieve();
			return 0;
		}
    }
    return list[selected_Ios];
}
void options_Menu(int device) {
	u32 buttons, number_of_patches = 0, AHBPROT_Patched = 0;
	int mode = 0;
	s32 Ios = IOS_GetVersion(), default_Ios = 249;
	
	for(;;) {
				draw_System_Info(Ios);
				WriteFont(50, 150, "Options .");
				if(disable_Disclaimer) WriteFont(50, 200, "[+] Enable Disclaimer .");
				else WriteFont(50, 200, "[+] Disable Disclaimer .");
				if(disable_wDance) WriteFont(50, 230, "[1] Enable Wario Dance .");
				else WriteFont(50, 230, "[1] Disable Wario Dance .");
				WriteFont(50, 260, "[2] Reload Ios .");
				WriteFont(50, 290, "[B] Back .");
				DrawFrameFinish();
				buttons = wpad_waitbuttons();
				if(buttons == WPAD_BUTTON_B) break;
				if(buttons == WPAD_BUTTON_MINUS) {
					draw_System_Info(Ios);
					if(Debugger == true) { 
						Debugger = false;
						WriteFont(50, 375, "Debugger Disabled .");
					}
					else {
						Debugger = true;
						WriteFont(50, 375, "Debugger Enabled .");
					}	
					DrawFrameFinish();
					logfile("debugger[%d]\n", Debugger);
					sleep(1);
					break;
				}
				if(buttons == WPAD_BUTTON_PLUS) {
					if(!disable_Disclaimer) mode = 1;
					else mode = 3;
					write_MMM_Config(fatdevicemounted, mode);
					read_MMM_Config(fatdevicemounted);
					draw_System_Info(Ios);
					if(!disable_Disclaimer) 
						WriteFont(50, 375, "Disclaimer Enabled .");
					else
						WriteFont(50, 375, "Disclaimer Disabled .");
					DrawFrameFinish();
					sleep(1);
					break;
				} 
				if (buttons == WPAD_BUTTON_1) {
					if(!disable_wDance) mode = 2;
					else mode = 4;
					write_MMM_Config(fatdevicemounted, mode);
					read_MMM_Config(fatdevicemounted);
					draw_System_Info(Ios);
					if(!disable_wDance)
						WriteFont(50, 375, "Wario Dance Enabled .");
					else
						WriteFont(50, 375, "Wario Dance Disabled .");
					DrawFrameFinish();
					sleep(1);
					break;
				}
				if (buttons == WPAD_BUTTON_2) {
					logfile("reload ios here\n");
					Ios = theme_ios_menu(default_Ios);
					logfile("Ios[%i]\n", Ios);
					Wpad_Disconnect();
					Fat_Unmount(fatdevicemounted);
					fatdevicemounted = 0;
					Ios = IOS_ReloadIOS(Ios);
					//PAD_Init();
					
					if(AHBPROT_DISABLED) {
						AHBPROT_Patched = IOSPATCH_AHBPROT();
						if(!AHBPROT_Patched) {
							//if(Debugger) logfile("Unable to patch AHBPROT %i \n", AHBPROT_Patched);
							
						}
						else {
							if(Debugger) logfile("Patched AHBPROT %i \n", AHBPROT_Patched);
						}
						number_of_patches = IOSPATCH_Apply();
					}
					else 
						number_of_patches = IOSPATCH_Apply();
					if(Debugger) logfile("number_of_patches [%i] \n", number_of_patches);
					Ios = IOS_GetVersion();
					WPAD_Init();
					WPAD_SetIdleTimeout(120);
					fatdevicemounted = Fat_Mount(device);
					break;
				}
				if (buttons == WPAD_BUTTON_HOME) {
					exit_Program();
				}
			}
	return;
}
int theme_device_menu() {
	int device = SD, fat_unmount_device = -1;
	u32 buttons;
	s32 Ios = IOS_GetVersion();
	
	for(;;) {
		draw_System_Info(Ios);
		WriteCentre(150, "Select Device :");
		sprintf(textbuf," %s ", device_Name(device));
		WriteCentre(210, textbuf);
		WriteFont(90, 325, "[Left]/[Right] Toggle Device");
		WriteFont(90, 350, "[A] Select Device");
		WriteFont(90, 375, "[Home]/[B] Return To");
		WriteFont(90, 400, "[1] Options");
		DrawFrameFinish();
		
		buttons = wpad_waitbuttons();
		//if(Debugger) logfile("buttons[%i]\n", buttons);
		if(buttons == WPAD_BUTTON_B){
			exit_Program();
		}
		if(buttons == WPAD_BUTTON_A) break;
		if(buttons == WPAD_BUTTON_HOME) exit_Program();
		if(buttons == WPAD_BUTTON_LEFT) {
			device -= 1;
			if(device <= 0)
				device = 2;
		}
		if(buttons == WPAD_BUTTON_RIGHT) {
			device += 1;
			if(device > 2)
				device = 1;
		}
		if (buttons == WPAD_BUTTON_1) {
			options_Menu(device);
		}
	}
	
	DrawFrameStart();
	
	if(fatdevicemounted <= 0) {
		fatdevicemounted = Fat_Mount(device);
		sprintf(textbuf, "Mounting %s ..... Complete .", device_Name(device));
		WriteCentre(300, textbuf);
		DrawFrameFinish();
		sleep(1);
	}
	else {
		fat_unmount_device = Fat_Unmount(fatdevicemounted);
		if (fat_unmount_device < 0) {
			sprintf(textbuf, "Unmounting %s ..... Failed .", device_Name(fatdevicemounted));
			WriteCentre(300, textbuf);
			sprintf(textbuf, "[-] Unable to unmount %s .", device_Name(fatdevicemounted));
			WriteCentre(325, textbuf);
			WriteCentre(350, "Press any button to continue .");
			DrawFrameFinish();
			wpad_waitbuttons();
			fatdevicemounted = Fat_Mount(device);
		}
		else {
			fatdevicemounted = Fat_Mount(device);
			if(fatdevicemounted < 0) {
				sprintf(textbuf, "Mounting %s ..... Failed .", device_Name(device));
				WriteCentre(300, textbuf);
				sprintf(textbuf, "[-] Unable to mount %s .", device_Name(device));
				WriteCentre(325, textbuf);
				WriteCentre(350, "Press any button to continue .");
				DrawFrameFinish();
				wpad_waitbuttons();
			}
		}
	}
	
	return fatdevicemounted;
}
int theme_entrycmp(const void *p1, const void *p2) {
	Fatfile *f1 = (Fatfile *)p1;
	Fatfile *f2 = (Fatfile *)p2;
    return strcasecmp(f1->name, f2->name);
}
s32 filelist_retrieve() {
    char dirpath[256];
	u32 filelistcntr;
	
	//if(Debugger) logfile("Retrieving file list ..... \n");
    // Generate dirpath 
	sprintf(dirpath, "%s:/modthemes", device_Name(fatdevicemounted));
	
	DIR *mydir;
	mydir = opendir(dirpath);
	themedir = "modthemes";
	if(!mydir) {
		//logfile("Failed .\n\t\tunable to open %s \n", dirpath);
		sprintf(dirpath, "%s:/themes", device_Name(fatdevicemounted));
		mydir = opendir(dirpath);
		themedir = "themes";
		if(!mydir) {
			//logfile("\t\tunable to open %s \n", dirpath);
			sleep(2);
			sprintf(dirpath, "%s:/themes", device_Name(fatdevicemounted));
			Fat_MakeDir(dirpath);
			return -99;
		}
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
	//if(Debugger) logfile("Retrieving file list ..... Complete !\nfilelistcntr[%d]", filelistcntr);
	
    return filelistcntr;
}
/* retrieve size of install file */
u32 filesize(FILE *file) {
	u32 curpos, endpos;
	
	if(file == NULL)
		return 0;
	
	curpos = ftell(file);
	fseek(file, 0, 2);
	endpos = ftell(file);
	fseek(file, curpos, 0);
	
	return endpos;
}
u32 verify_content_file_Version(char * name) { 
	char filepath[256];
    FILE *fp = NULL;
    u32 length = 0, i, rtn = 0;
    u8 *themedata = NULL;
	sprintf(filepath, "%s:/%s/%s", device_Name(fatdevicemounted), themedir, name);
    fp = fopen(filepath, "rb");
    if (!fp) {
        logfile("unable to open path\n");
		return 0;
	}
    length = filesize(fp);
    themedata = allocate_memory(length);
    memset(themedata,0,length);
    fread(themedata,1,length,fp);
	fclose(fp);
	
    if(length <= 0) {
        logfile("[-] Unable to read file !! \n");
        return 0;
    }
    else {
        for(i = 0; i < length; i++)
        {
            if(themedata[i] == 83) { // S (System) wii
                if(themedata[i+6] == 52)  // 4
                {
                    if(themedata[i+8] == 48)       // 0
                    {
                        if(themedata[i+28] == 85)  // U
                        {
                            if(themedata[i+29] == 83) { // S
								rtn = 417;
								break;
							}
                        }
                        else if(themedata[i+28] == 74) // J
                        {
                            if(themedata[i+29] == 80) { //P
								rtn = 416;
								break;
							}
                        }
                        else if(themedata[i+28] == 69)  // E
                        {
                            if(themedata[i+29] == 85) { // U
								rtn = 418;
								break;
							}
                        }
                    }
                    else if(themedata[i+8] == 49)  // 1
                    {
                        if(themedata[i+31] == 85)  // U
                        {
                            if(themedata[i+32] == 83) { //S
								rtn = 449;
								break;
							}
                        }
                        else if(themedata[i+31] == 74)  //J
                        {
                            if(themedata[i+32] == 80) { //P
								rtn = 448;
								break;
							}
                        }
                        else if(themedata[i+31] == 69)  // E
                        {
                            if(themedata[i+32] == 85) { // U
								rtn = 450;
								break;
							}
                        }
                        else if(themedata[i+31] == 75)  // K
                        {
                            if(themedata[i+32] == 82) { // R
								rtn = 454;
								break;
							}
						}
                    }
					else if(themedata[i+8] == 50)  // 2
                    {
                        if(themedata[i+28] == 85)  // U
                        {
                            if(themedata[i+29] == 83) { // S
								rtn = 481;
								break;
							}
                        }
                        else if(themedata[i+28] == 74)  // J
                        {
                            if(themedata[i+29] == 80) { //P
								rtn = 480;
								break;
							}
                        }
                        else if(themedata[i+28] == 69)  // E
                        {
                            if(themedata[i+29] == 85) { // U
								rtn = 482;
								break;
							}
                        }
                        else if(themedata[i+28] == 75)  // K
                        {
                            if(themedata[i+29] == 82) { // R
								rtn = 486;
								break;
							}
                        }
                    }
                    else if(themedata[i+8] == 51)  // 3
                    {
                        if(themedata[i+28] == 85)  // U
                        {
                            if(themedata[i+29] == 83) { // S
								rtn = 513;
								break;
							}
                        }
                        else if(themedata[i+28] == 74)  //J
                        {
                            if(themedata[i+29] == 80) { //P
								rtn = 512;
								break;
							}
                        }
                        else if(themedata[i+28] == 69)  // E
                        {
                            if(themedata[i+29] == 85) { // U
								rtn = 514;
								break;
							}
                        }
                        else if(themedata[i+28] == 75)  // K
                        {
                            if(themedata[i+29] == 82) { // R
								rtn = 518;
								break;
							}
                        }
                    }
                }
            }
			else if(themedata[i] == 67) { // C (Compat) vwii
				if(themedata[i+6] == 52) {  // 4
					if(themedata[i+8] == 51) { // 3
						if(themedata[i+28] == 85)  // U
                        {
                            if(themedata[i+29] == 83) { // S
								rtn = 609;
								break;
							}
                        }
						else if(themedata[i+28] == 74)  //J
                        {
                            if(themedata[i+29] == 80) { //P
								rtn = 608;
								break;
							}
                        }
						else if(themedata[i+28] == 69)  // E
                        {
                            if(themedata[i+29] == 85) { // U
								rtn = 610;
								break;
							}
                        }
					}
				}
			}
        }
    }
	free(themedata);
	
	return rtn;
}
int find_content_Region(u32 version) {
	switch(version) {
		case 416:
		case 448:
		case 480:
		case 512:
		case 608:
			return 74;
			break;
		case 417:
		case 449:
		case 481:
		case 513:
		case 609:
			return 85;
			break;
		case 418:
		case 450:
		case 482:
		case 514:
		case 610:
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
bool warnunsignedtheme() {
	u32 buttons;
	s32 Ios = IOS_GetVersion();
	
	for(;;) {
		draw_System_Info(Ios);
		WriteCentre(150, "Unsigned Theme Detected !");

		WriteFont(30, 200, "It is recommended to use www.wiithemer.org ,");
		WriteFont(30, 225, "ModMii, or wii theme manager to build safe");
		WriteFont(30, 250, "and verified themes ,");
		WriteFont(30, 275, "Only install this file if you made it");
		WriteFont(30, 300, "or trust where it came from .");
		WriteFont(30, 325, "You may continue at own risk .");

		WriteFont(60, 375, "[A] Continue .");
		WriteFont(60, 400, "[B] Back .");
		DrawFrameFinish();
		buttons = wpad_waitbuttons();
		
		if((buttons == WPAD_BUTTON_B) || (buttons == WPAD_BUTTON_HOME)) {
			return false;
		}
		if(buttons == WPAD_BUTTON_A) {
			break;
		}
	}
	return true;
}
void nopriiloadermessage() {
	u32 buttons;
	s32 Ios = IOS_GetVersion();
	
	for(;;) {
		draw_System_Info(Ios);
		WriteCentre(150, "PriiLoader Not Detected !");

		WriteFont(60, 200, "It is recommended to have priiloader");
		WriteFont(60, 230, "installed as Boot2 or Ios before");
		WriteFont(60, 260, "using this Program .");
		WriteFont(60, 325, "You may Continue at your Own risk .");

		WriteFont(60, 375, "[A]  Continue .");
		WriteFont(60, 400, "[B]  Select Exit To .");
		DrawFrameFinish();
		buttons = wpad_waitbuttons();
		
		if((buttons == WPAD_BUTTON_B) || (buttons == WPAD_BUTTON_HOME)) {
			exit_Program();
		}
		if(buttons == WPAD_BUTTON_A) {
			break;
		}
	}
	return;
}
bool checkforpriiloader() {
	dirent_t *priiloaderfiles = NULL;
	u32 nandfilecnt;
	int filecntr, rtn;
	char *searchstr;
	
	searchstr = "title_or.tmd";
	rtn = getdir("/title/00000001/00000002/content",&priiloaderfiles,&nandfilecnt);
	if(rtn < 0)
		return false;
	for(filecntr = 0; filecntr < nandfilecnt; filecntr++) {
		if(!strcmp(priiloaderfiles[filecntr].name, searchstr))
		return true;
	}
	return false; 
}
bool Is_content_file_U8() {
	bool is_content = false;
	FILE *content_File;
	char content_file_Path[256];
	u8 *content_Data = NULL;
	int u8_magic[] = { 85, 170, 56, 45 };
	int magic_read_length = 4, content_data_Counter;
	
	sprintf(content_file_Path, "%s:/%s/%s", device_Name(fatdevicemounted), themedir, themefile[selected].name);
	content_File = fopen(content_file_Path, "rb");
	if(!content_File)
		if(Debugger) logfile("Unable to open file (%s) \n", content_file_Path);
	content_Data = allocate_memory(magic_read_length);
    memset(content_Data, 0, magic_read_length);
    fread(content_Data, 1, magic_read_length, content_File);
	fclose(content_File);
	
	for(content_data_Counter = 0; content_data_Counter < magic_read_length; content_data_Counter++) {
		if(content_Data[content_data_Counter] == u8_magic[content_data_Counter]) {
			if(content_data_Counter == magic_read_length - 1) {
				is_content = true;
			}
		}
		if(Debugger) logfile("content_Data[%i]    u8_magic[%i]\n", content_Data[content_data_Counter], u8_magic[content_data_Counter]);
	}
	return is_content;
}
bool is_content_vWii(u32 version) {
	
	if(Debugger) logfile("version[%u]\n", version);
	switch(version) {
		case 608:
		case 609:
		case 610:
			return true;
		break;
	}
	return false;
}
const char *signature_display_name(int pos) {
	switch(pos) {
		case 0:
			return "No Signature";
			break;
		case 1:
			return "Wii Themer";
			break;
		case 2:
			return "ModMii";
			break;
		case 3:
			return "Theme Thing";
			break;
		case 4:
			return "Wii Theme Manager";
	}
	return "UNKNOWN";
}
int check_file_Signature() {
	int wii_themer_signature[] = { 87, 105, 105, 95, 84, 104, 101, 109, 101, 114 }; // Wii_Themer
	int modmii_signature[] = { 77, 111, 100, 77, 105, 105 }; // ModMii
	int theme_thing_signature[] = { 84, 104, 101, 109, 101, 95, 84, 104, 105, 110, 103 }; // Theme_Thing
	int wii_theme_manager_signature[] = { 87, 105, 105, 84, 104, 101, 109, 101, 77, 97, 110, 97, 103, 101, 114 }; // WiiThemeManager
	
	int is_content_signed;
	FILE *content_File;
	char content_file_Path[2048];
	u32 content_file_Size = 0;
	u8 *content_Data = NULL;
	int content_data_counter;
	sprintf(content_file_Path, "%s:/%s/%s", device_Name(fatdevicemounted), themedir, themefile[selected].name);
	content_File = fopen(content_file_Path, "rb");
	if(!content_File) {
		if(Debugger) logfile("Unable to open file (%s) \n", content_file_Path);
		return -1;
	}
	content_file_Size = filesize(content_File);
	content_Data = allocate_memory(content_file_Size);
    memset(content_Data, 0, content_file_Size);
    fread(content_Data, 1, content_file_Size, content_File);
	
	if(content_file_Size <= 0) {
		if(Debugger) logfile("unable to get file size .(%s)(%i)\n", content_file_Path, content_file_Size);
		return -2;
	}
	for(content_data_counter = 0; content_data_counter < content_file_Size; content_data_counter++) {
		// wii themer signature
		if(content_Data[content_data_counter] == wii_themer_signature[0]) {
			if(content_Data[content_data_counter + 1] == wii_themer_signature[1])
				if(content_Data[content_data_counter + 2] == wii_themer_signature[2])
					if(content_Data[content_data_counter + 3] == wii_themer_signature[3])
						if(content_Data[content_data_counter + 4] == wii_themer_signature[4])
							if(content_Data[content_data_counter + 5] == wii_themer_signature[5])
								if(content_Data[content_data_counter + 6] == wii_themer_signature[6])
									if(content_Data[content_data_counter + 7] == wii_themer_signature[7])
										if(content_Data[content_data_counter + 8] == wii_themer_signature[8])
											if(content_Data[content_data_counter + 9] == wii_themer_signature[9]) {
												is_content_signed = 1;
												break;
											}
		}
		
		// modmii signature
		if(content_Data[content_data_counter] == modmii_signature[0]) {
			if(content_Data[content_data_counter + 1] == modmii_signature[1])
				if(content_Data[content_data_counter + 2] == modmii_signature[2])
					if(content_Data[content_data_counter + 3] == modmii_signature[3])
						if(content_Data[content_data_counter + 4] == modmii_signature[4])
							if(content_Data[content_data_counter + 5] == modmii_signature[5]) {
								is_content_signed = 2;
								break;
							}
		}
		
		// theme thing signature
		if(content_Data[content_data_counter] == theme_thing_signature[0]) {
			if(content_Data[content_data_counter + 1] == theme_thing_signature[1])
				if(content_Data[content_data_counter + 2] == theme_thing_signature[2])
					if(content_Data[content_data_counter + 3] == theme_thing_signature[3])
						if(content_Data[content_data_counter + 4] == theme_thing_signature[4])
							if(content_Data[content_data_counter + 5] == theme_thing_signature[5])
								if(content_Data[content_data_counter + 6] == theme_thing_signature[6])
									if(content_Data[content_data_counter + 7] == theme_thing_signature[7])
										if(content_Data[content_data_counter + 8] == theme_thing_signature[8])
											if(content_Data[content_data_counter + 9] == theme_thing_signature[9])
												if(content_Data[content_data_counter + 10] == theme_thing_signature[10]) {
													is_content_signed = 3;
													break;
												}
		}
		
		// wii theme manager signature
		if(content_Data[content_data_counter] == wii_theme_manager_signature[0]) {
			if(content_Data[content_data_counter + 1] == wii_theme_manager_signature[1])
				if(content_Data[content_data_counter + 2] == wii_theme_manager_signature[2])
					if(content_Data[content_data_counter + 3] == wii_theme_manager_signature[3])
						if(content_Data[content_data_counter + 4] == wii_theme_manager_signature[4])
							if(content_Data[content_data_counter + 5] == wii_theme_manager_signature[5])
								if(content_Data[content_data_counter + 6] == wii_theme_manager_signature[6])
									if(content_Data[content_data_counter + 7] == wii_theme_manager_signature[7])
										if(content_Data[content_data_counter + 8] == wii_theme_manager_signature[8])
											if(content_Data[content_data_counter + 9] == wii_theme_manager_signature[9])
												if(content_Data[content_data_counter + 10] == wii_theme_manager_signature[10])
													if(content_Data[content_data_counter + 11] == wii_theme_manager_signature[11])
														if(content_Data[content_data_counter + 12] == wii_theme_manager_signature[12])
															if(content_Data[content_data_counter + 13] == wii_theme_manager_signature[13])
																if(content_Data[content_data_counter + 14] == wii_theme_manager_signature[14]) {
																	is_content_signed = 4;
																	break;
																}
		}
		is_content_signed = 0;
	}
	fclose(content_File);
	free(content_Data);
	content_Data = NULL;
	return is_content_signed;
}
char *themeName = NULL;
char data_id[7] = {'-', '-', '-', '\0'};
char data_spin[7] = {'-', '-', '-', '\0'};
char data_content[3] = {'-', '-', '\0'};
bool check_Id_Signature(s32 version) {
	FILE *content_File;
	char content_file_Path[2048];
	u32 content_file_Size = 0;
	u8 *content_Data = NULL;
	int content_data_counter, id_offset = 0;
	
	switch(version) {
		case 417:
		case 449:
		case 481:
		case 513:
			id_offset = 119344;
			break;
		case 418:
		case 450:
		case 482:
		case 514:
			id_offset = 119376;
			break;
		case 416:
		case 448:
		case 480:
		case 512:
			id_offset = 134736;
			break;
		case 454:
		case 486:
		case 518:
			id_offset = 102832;
			break;
		case 608:
			id_offset = 134928;
			break;
		case 609:
			id_offset = 119568;
			break;
		case 610:
			id_offset = 119568;
			break;
	}
	
	sprintf(content_file_Path, "%s:/%s/%s", device_Name(fatdevicemounted), themedir, themefile[selected].name);
	content_File = fopen(content_file_Path, "rb");
	if(!content_File) {
		if(Debugger) logfile("Unable to open file (%s) \n", content_file_Path);
		return false;
	}
	
	content_file_Size = filesize(content_File);
	content_Data = allocate_memory(content_file_Size);
    memset(content_Data, 0, content_file_Size);
    fread(content_Data, 1, content_file_Size, content_File);
	for(content_data_counter = 0; content_data_counter < content_file_Size; content_data_counter++) {
		if(content_data_counter == id_offset) {
			if(content_Data[content_data_counter] != 0) {
				if(content_Data[content_data_counter] == 0x01) {
					if(content_Data[content_data_counter + 15] == 0xA0) {
						fclose(content_File);
						free(content_Data);
						content_Data = NULL;
						return true;
					}
				}
			}
			
		}
	}
	fclose(content_File);
	free(content_Data);
	content_Data = NULL;
	return false;
}
void find_theme_Info(s32 version) {
	if(Debugger) logfile("Find theme Info \n");
	
	FILE *content_File;
	char content_file_Path[2048];
	u32 content_file_Size = 0;
	u8 *content_Data = NULL;
	int content_data_counter, id_offset = 0, theme_id_len = 0;
	bool has_id_signature = check_Id_Signature(version);
	if(has_id_signature) return;
	switch(version) {
		case 417:
		case 449:
		case 481:
		case 513:
			id_offset = 119344;
			break;
		case 418:
		case 450:
		case 482:
		case 514:
			id_offset = 119408;
			break;
		case 416:
		case 448:
		case 480:
		case 512:
			id_offset = 134736;
			break;
		case 454:
		case 486:
		case 518:
			id_offset = 119584;
			break;
		case 608:
			id_offset = 134864;
			break;
		case 609:
			id_offset = 119504;
			break;
		case 610:
			id_offset = 119536;
			break;
	}
	
	sprintf(content_file_Path, "%s:/%s/%s", device_Name(fatdevicemounted), themedir, themefile[selected].name);
	content_File = fopen(content_file_Path, "rb");
	if(!content_File) {
		if(Debugger) logfile("Unable to open file (%s) \n", content_file_Path);
		return;
	}
	
	content_file_Size = filesize(content_File);
	content_Data = allocate_memory(content_file_Size);
    memset(content_Data, 0, content_file_Size);
    fread(content_Data, 1, content_file_Size, content_File);
	for(content_data_counter = 0; content_data_counter < content_file_Size; content_data_counter++) {
		if(content_data_counter == id_offset) {
			if(content_Data[content_data_counter] != 0x00) {
				data_id[0] = (char)content_Data[content_data_counter];
				data_id[1] = (char)content_Data[content_data_counter + 1];
				data_id[2] = (char)content_Data[content_data_counter + 2];
				data_id[3] = (char)content_Data[content_data_counter + 3];
				data_id[4] = (char)content_Data[content_data_counter + 4];
				data_id[5] = (char)content_Data[content_data_counter + 5];
				data_id[6] = '\0';
				if(Debugger) logfile("data_id[%s]\n", data_id);
				data_spin[0] = (char)content_Data[content_data_counter + 7];
				data_spin[1] = (char)content_Data[content_data_counter + 8];
				data_spin[2] = (char)content_Data[content_data_counter + 9];
				data_spin[3] = (char)content_Data[content_data_counter + 10];
				data_spin[4] = (char)content_Data[content_data_counter + 11];
				data_spin[5] = (char)content_Data[content_data_counter + 12];
				data_spin[6] = '\0';
				if(Debugger) logfile("data_spin[%s]\n", data_spin);
				data_content[0] = (char)content_Data[content_data_counter + 14];
				data_content[1] = (char)content_Data[content_data_counter + 15];
				data_content[2] = '\0';
				if(Debugger) logfile("data_content[%s]\n", data_content);
				break;
			}
			else {
				fclose(content_File);
				free(content_Data);
				content_Data = NULL;
				return;
			}
		}
	}
	fclose(content_File);
	free(content_Data);
	content_Data = NULL;
	for(;;) {
		if(theme_ID[theme_id_len] == NULL)
			break;
		else
			theme_id_len++;
	}
	if(Debugger) logfile("size of theme_ID[%i]\n", theme_id_len);
	for(int i = 0; i < theme_id_len; i++) {
		if(strcmp(data_id, theme_ID[i]) == 0) {
			
			if(theme_Name[i] != NULL)
				themeName = theme_Name[i];
			else 
				themeName = "---";
			if(Debugger) logfile("id[%s] name[%s] spin[%s] content[%s]\n", data_id, themeName, data_spin, data_content);
			break;
		}
	}
	if(themeName == NULL)
		themeName = "---";
	return;
}
s32 backup_original_Content(FILE * fp, const char *content_File) {
	char * data;
	s32 ret, nandfile, ios = 2;
	u32 length = 0,numchunks, cursize, i;
	char filename[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	
	sprintf(filename, "/title/00000001/%08x/content/CB.app", ios);
	nandfile = ISFS_Open(filename, ISFS_OPEN_RW);
	ISFS_Seek(nandfile, 0, SEEK_SET);
	length = filesize(fp);
	numchunks = length/CHUNKS + ((length % CHUNKS != 0) ? 1 : 0);
	if(Debugger) logfile("Installing %s\n", filename);
	if(Debugger) logfile("[+] Total parts: %d\n", numchunks);
	
	for(i = 0; i < numchunks; i++)
	{
		data = memalign(32, CHUNKS);
		if(data == NULL)
		{
			if(Debugger) logfile("\t[-] Error allocating memory !\n\n");
			return -1;
		}
		if(Debugger) logfile("Installing part %d\n", (i + 1));
		ret = fread(data, 1, CHUNKS, fp);
		if (ret < 0) 
		{
			if(Debugger) logfile("\t[-] Error reading from SD ! (ret = %d)\n\n", ret);
			//wpad_waitbuttons();
			return -2;
		}
		else
		{
			cursize = ret;
		}
		ret = ISFS_Write(nandfile, data, cursize);
		if(ret < 0)
		{
			if(Debugger) logfile("\t[-] Error writing to NAND ! (ret = %d)\n\n", ret);
			//wpad_waitbuttons();
			return ret;
		}
		free(data);
		
		if(Debugger) logfile("Complete .\n");
	}
	ISFS_Close(nandfile);
	return 0;
}
s32 InstallFile(FILE * fp) {
	char * data;
	s32 ret, nandfile, ios = 2;
	u32 length = 0,numchunks, cursize, i;
	char filename[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	u32 newtmdsize ATTRIBUTE_ALIGN(32);
	u64 newtitleid ATTRIBUTE_ALIGN(32);
	signed_blob * newtmd;
	tmd_content * newtmdc, * newtmdcontent = NULL;
	s32 Ios = IOS_GetVersion();
	newtitleid = 0x0000000100000000LL + ios;
	ES_GetStoredTMDSize(newtitleid, &newtmdsize);
	newtmd = (signed_blob *) memalign(32, newtmdsize);
	memset(newtmd, 0, newtmdsize);
	ES_GetStoredTMD(newtitleid, newtmd, newtmdsize);
	newtmdc = TMD_CONTENTS((tmd *) SIGNATURE_PAYLOAD(newtmd));
	for(i = 0; i < ((tmd *) SIGNATURE_PAYLOAD(newtmd))->num_contents; i++)
	{
		if(newtmdc[i].index == 1)
		{
			newtmdcontent = &newtmdc[i];
			if(newtmdc[i].type & 0x8000) //Shared content! This is the hard part :P.
				return -1;
			else {//Not shared content, easy
				sprintf(filename, "/title/00000001/%08x/content/%08x.app", ios, newtmdcontent->cid);
	
				break;
			}
		}
		else if(i == (((tmd *) SIGNATURE_PAYLOAD(newtmd))->num_contents) - 1)
			return -1;
	}
	free(newtmd);
	nandfile = ISFS_Open(filename, ISFS_OPEN_RW);
	ISFS_Seek(nandfile, 0, SEEK_SET);
	length = filesize(fp);
	numchunks = length/CHUNKS + ((length % CHUNKS != 0) ? 1 : 0);
	draw_System_Info(Ios);
	sprintf(textbuf, "Installing %s", themefile[selected].name);
	WriteFont(80, 135, textbuf);
	sprintf(textbuf, "[+] Total parts: %d", numchunks);
	WriteFont(80, 170, textbuf);
	DrawFrameFinish();
	for(i = 0; i < numchunks; i++)
	{
		data = memalign(32, CHUNKS);
		if(data == NULL)
		{
			logfile("\t[-] Error allocating memory !\n\n");
			
			//printf("\tPress any button to continue .....\n");
			//wpad_waitbuttons();
			return -1;
		}
		draw_System_Info(Ios);
		sprintf(textbuf, "Installing %s", themefile[selected].name);
		WriteFont(80, 135, textbuf);
		sprintf(textbuf, "[+] Total parts: %d", numchunks);
		WriteFont(80, 170, textbuf);
		sprintf(textbuf, "Installing part %d", (i + 1));
		WriteFont(80, 200, textbuf);
		DrawFrameFinish();
		ret = fread(data, 1, CHUNKS, fp);
		if (ret < 0) 
		{
			logfile("\t[-] Error reading from SD ! (ret = %d)\n\n", ret);
			sprintf(textbuf, "[-] Error reading from SD ! (ret = %d)", ret);
			WriteFont(80, 230, textbuf);
			WriteFont(80, 260, "Press any button to continue .");
			DrawFrameFinish();
			wpad_waitbuttons();
			return -2;
		}
		else
		{
			cursize = ret;
		}
		wiilight(1);
		draw_System_Info(Ios);
		sprintf(textbuf, "Installing %s", themefile[selected].name);
		WriteFont(80, 135, textbuf);
		sprintf(textbuf, "[+] Total parts: %d", numchunks);
		WriteFont(80, 170, textbuf);
		sprintf(textbuf, "Installing part %d", (i + 1));
		WriteFont(80, 200, textbuf);
		ret = ISFS_Write(nandfile, data, cursize);
		if(ret < 0)
		{
			logfile("\t[-] Error writing to NAND ! (ret = %d)\n\n", ret);
			sprintf(textbuf, "[-] Error writing to NAND ! (ret = %d)", ret);
			WriteFont(80, 230, textbuf);
			WriteFont(80, 260, "Press any button to continue .");
			DrawFrameFinish();
			wpad_waitbuttons();
			return ret;
		}
		free(data);
		
		WriteFont(80, 230, "Complete .");
		wiilight(0);
		DrawFrameFinish();
	}
	ISFS_Close(nandfile);
	return 0;
}
void theme_manage_menu() {
	if(Debugger) logfile("Selected theme :  %s\n", themefile[selected].name);
	f32 sizeoffile;
	char filepath[256];
	FILE *tmpfile;
	u32 size, buttons;
	bool install_file = false;
	bool is_content_file = false;
	
	bool acknowledge_theme_unsigned = false;
	s32 Ios = IOS_GetVersion();
	draw_System_Info(Ios);
	WriteFont(80, 140, "Gathering Info ... ");
	DrawFrameFinish();
	int content_has_signature = check_file_Signature();
	s32 install_version = verify_content_file_Version(themefile[selected].name);
	//bool is_vWii = false;
	//bool is_device_vWii = false;
	
	sprintf(filepath, "%s:/%s/%s", device_Name(fatdevicemounted), themedir, themefile[selected].name);
	tmpfile = fopen(filepath, "rb");
	if(tmpfile != NULL) {
		size = filesize(tmpfile);
		fclose(tmpfile);
		themefile[selected].size = size;
	}
	sizeoffile = themefile[selected].size/MB_SIZE;
	if(content_has_signature) 
		find_theme_Info(install_version);
	draw_System_Info(Ios);
	WriteFont(80, 140, "Gathering Info ... Complete .");
	DrawFrameFinish();
	sleep(1);
	for(;;) {
		draw_System_Info(Ios);
		sprintf(textbuf, "Theme :  %s", (themeName != NULL ? themeName : themefile[selected].name));
		WriteFont(80, 140, textbuf);
		sprintf(textbuf, "File size :  %.2f MB", sizeoffile);
		WriteFont(80, 170, textbuf);
		sprintf(textbuf, "Theme Id :  %s", data_id);
		WriteFont(80, 200, textbuf);
		sprintf(textbuf, "Spin Option   %s", data_spin);
		WriteFont(80, 230, textbuf);
		sprintf(textbuf, "Content :  000000%s.app", data_content);
		WriteFont(80, 260, textbuf);
		sprintf(textbuf, "Signature :  %s", signature_display_name(content_has_signature));
		WriteFont(80, 290, textbuf);
		WriteFont(80, 350, "[Home]  Exit To");
		WriteFont(80, 375, "[A] Install   [B]  Back");
		
		DrawFrameFinish();
		
		buttons = wpad_waitbuttons();
		
		if(buttons == WPAD_BUTTON_HOME) exit_Program();
		if(buttons == WPAD_BUTTON_A) { install_file = true; break;}
		if(buttons == WPAD_BUTTON_B) { 
			themeName = NULL;
			data_id[0] = '-';
			data_id[1] = '-';
			data_id[2] = '-';
			data_id[3] = '\0';
			data_spin[0] = '-';
			data_spin[1] = '-';
			data_spin[2] = '-';
			data_spin[3] = '\0';
			data_content[0] = '-';
			data_content[1] = '-';
			data_content[2] = '\0';
			break;
		}
	}
	if(!install_file) return;
	if(content_has_signature == 0) {
		acknowledge_theme_unsigned = warnunsignedtheme();
		if(!acknowledge_theme_unsigned) return;
	}
	draw_System_Info(Ios);
	sprintf(textbuf, "Installing %s", themefile[selected].name);
	WriteFont(80, 135, textbuf);
	// check if file is a u8 archive
	is_content_file = Is_content_file_U8();
	if(Debugger) logfile("is_content_file[%i]\n", is_content_file);
	if(!is_content_file) {
		WriteFont(80, 180, "This File is not a U8 archive !");
		WriteFont(80, 205, "Unable to use this file .");
		WriteFont(80, 335, "Press any button to return");
		WriteFont(80, 360, "to the Selection Menu !");
		DrawFrameFinish();
		wpad_waitbuttons();
		return;
	}
	
	themefile[selected].version = verify_content_file_Version(themefile[selected].name);
	if(Debugger) logfile("install theme version [%i]\n", themefile[selected].version);
	//if(themefile[selected].version > 610) themefile[selected].version = checkcustomsystemmenuversion();
	themefile[selected].region = find_content_Region(themefile[selected].version);
	if(Debugger) logfile("install theme region [%i]\n", themefile[selected].region);
	//is_vWii = is_content_vWii(themefile[selected].version);
	//if(Debugger) logfile("is_vWii[%i]\n", is_vWii);
	currentTheme.version = systemmenuVersion;
	//if(currentTheme.version > 610)  currentTheme.version = checkcustomsystemmenuversion();
	if(Debugger) logfile("current theme version [%i]\n", currentTheme.version);
	currentTheme.region = find_content_Region(systemmenuVersion);
	if(Debugger) logfile("current theme region [%i]\n", currentTheme.region);
	
	if(currentTheme.version != themefile[selected].version) { 
        sprintf(textbuf, "Installing %s - Failed", themefile[selected].name);
		WriteFont(80, 135, textbuf);
		WriteFont(80, 160, "Install can not continue !");
		WriteFont(80, 185, "The install theme version is not a match");
		WriteFont(80, 210, "for the system menu version .");
		WriteFont(80, 335, "Press any button to return");
		WriteFont(80, 360, "to the Selection Menu !");
		DrawFrameFinish();
		wpad_waitbuttons();
		return;
	}
	if(currentTheme.region != themefile[selected].region) {
		sprintf(textbuf, "Installing %s ..... Failed", themefile[selected].name);
		WriteFont(80, 135, textbuf);
		WriteFont(80, 160, "Install can not continue !");
		WriteFont(80, 185, "The install theme region is not a match");
		WriteFont(80, 210, "for the system menu region .");
		WriteFont(80, 335, "Press any button to return");
		WriteFont(80, 360, "to the Selection Menu !");
		DrawFrameFinish();
		wpad_waitbuttons();
		return;
	}
	DrawFrameFinish();
	draw_System_Info(Ios);
	sprintf(textbuf, "Installing %s", themefile[selected].name);
	WriteFont(80, 135, textbuf);
	sprintf(filepath, "%s:/%s/%s", device_Name(fatdevicemounted), themedir, themefile[selected].name);
	tmpfile = fopen(filepath, "rb");
	if(!tmpfile) {
		logfile("unable to open %s .\n", filepath);
		return;
	}
	DrawFrameFinish();
	InstallFile(tmpfile);
	fclose(tmpfile);
	draw_System_Info(Ios);
	sprintf(textbuf, "Installing %s ", themefile[selected].name);
	WriteFont(80, 135, textbuf);
	WriteFont(80, 170, "Complete .");
	DrawFrameFinish();
	free_Png();
	sleep(2);
	exit_Program();
	
	return;
}
int downloadApp() {
	//s32 rtn;
    u32 tmpversion;
    int ret;
    int counter;
	int retries = 50, retrycnt, switch_path = 0;
    char *savepath = (char*)memalign(32, 256);
	bool content_is_vWii = false;
    char *wiiserverlist[] = {"cetk", "tmd."};
	signed_blob * s_tik = NULL;
    signed_blob * s_tmd = NULL;
	u32 outlen = 0;
    u32 http_status = 0;
	char * wii_titleId = "0000000100000002";
	char *vWii_titleId = "0000000700000002";
	char *titleId;
	const char *download_Path;
	s32 Ios = IOS_GetVersion();
    tmpversion = GetSysMenuVersion();
    //logfile("dvers =%d \n", tmpversion);
    //if(tmpversion > 610) tmpversion = checkcustomsystemmenuversion();
	//if(!tmpversion) return -5;
	content_is_vWii = is_content_vWii(tmpversion);
	if(content_is_vWii) titleId = vWii_titleId;
	else titleId = wii_titleId;
    draw_System_Info(Ios);
	//sprintf(textbuf, "Theme :  %s", themefile[selected].name);
	WriteFont(80, 140, "Initializing  Network ..... ");
	DrawFrameFinish();
    draw_System_Info(Ios);
    for(retrycnt = 0; retrycnt < retries; retrycnt++) {
        ret = net_init();
		if(ret == 0) { WriteFont(80, 140, "Initializing  Network ..... Complete ."); DrawFrameFinish(); break; }
    }
	if(retrycnt >= 50) { WriteFont(80, 140, "Initializing  Network ..... Failed ."); DrawFrameFinish(); return ret; }
	draw_System_Info(Ios);
	sprintf(textbuf, "Downloading %s for System Menu v%d", getsavename(tmpversion), tmpversion);
	WriteFont(80, 140, textbuf);
	DrawFrameFinish();
	download_Path = RC24path;
	for(counter = 0; counter < 3;) {	
        int app_pos = getslot(tmpversion);
        char *path = (char*)memalign(32, 256);
		if(counter == 0) {
            sprintf(path,"%s/%s/%s", download_Path, titleId, wiiserverlist[counter]);
            logfile("path[%s]\nDowloading System Menu Cetk .... ", path);
			ret = http_request(path, MAX_SIZE_HTTP, false);
			
        }
        else if(counter == 1) {
            sprintf(path,"%s/%s/%s%d", download_Path, titleId, wiiserverlist[counter], tmpversion);
            logfile("Dowloading System Menu Tmd .... ");
			ret = http_request(path, MAX_SIZE_HTTP, false);
			
        }
        else if(counter == 2) {
            sprintf(path,"%s/%s/%s", download_Path, titleId, appfilename_noext(app_pos));
            logfile("Dowloading %s .... ", getsavename(tmpversion));
			ret = http_request(path, MAX_SIZE_HTTP, false);
			
        }
        if(ret == 0 ) {
            free(path);
			path = NULL;
            //logfile("download failed !! ret(%d)\n",ret);
            logfile("Failed !! ret(%d)\n",ret);
			download_Path = wiiU_shoppath;
			switch_path++;
			if(switch_path >= 2) {
				download_Path = wiishoppath;
			}
			if(switch_path >= 3)
				return -9;
			continue;
        }
        free(path);
		path = NULL;
        u8* outbuf = (u8*)malloc(outlen);
        if(counter == 0) ret = http_get_result(&http_status, (u8 **)&s_tik, &outlen);
        if(counter == 1) ret = http_get_result(&http_status, (u8 **)&s_tmd, &outlen);
        if(counter == 2) ret = http_get_result(&http_status, &outbuf, &outlen);
        logfile("\nDecrypting files ....");
		
        //set aes key
        u8 key[16];
        u16 index;
        get_title_key(s_tik, key, content_is_vWii);
        aes_set_key(key);
        u8* outbuf2 = (u8*)malloc(outlen);
        if(counter == 2) {
            if(outlen > 0) {//suficientes bytes
                index = 01;
                //then decrypt buffer
                decrypt_buffer(index,outbuf,outbuf2,outlen);
				logfile("Complete !! \n\n");
                sprintf(savepath,"%s:/%s/%s", device_Name(fatdevicemounted), themedir, getsavename(tmpversion));
				logfile("Saving file .....");
                ret = Fat_SaveFile(savepath, (void *)&outbuf2, outlen);
				//printf("Saving file ..... ");
            }
        }
        logfile("Complete !! \n\n");
        if(outbuf != NULL) {
            free(outbuf);
			outbuf = NULL;
		}
		if(outbuf2 != NULL) {
            free(outbuf2);
			outbuf2 = NULL;
		}
		counter++;
    }
	net_deinit();
	
	sleep(2);
	
	if(!Fat_CheckFile(savepath)) {
		if(savepath != NULL)
            free(savepath);
			savepath = NULL;
		return -99;
	}
	
	if(savepath != NULL) {
        free(savepath);
		savepath = NULL;
	}
    return 1;
}
void backup_content_to_Nand(const char *content_File) {
	if(Debugger) logfile("backup_content_to_Nand()\n");
	if(Debugger) logfile("content_File[%s]\n", content_File);
	dirent_t *priiloaderfiles = NULL;
	u32 nandfilecnt;
	int filecntr, rtn;
	char searchstr[256];
	char content_to_backup_Path[256];
	bool content_Backedup = false;
	FILE *ContentFile;
	
	sprintf(searchstr, "%s_bak.app", content_File);
	if(Debugger) logfile("searchstr[%s]\n", searchstr);
	rtn = getdir("/title/00000001/00000002/content",&priiloaderfiles,&nandfilecnt);
	if(rtn < 0)
		return;
	for(filecntr = 0; filecntr < nandfilecnt; filecntr++) {
		if(Debugger) logfile("prii_name[%s]\n", priiloaderfiles[filecntr].name);
		if(!strcmp(priiloaderfiles[filecntr].name, searchstr)) {
			if(Debugger) logfile("prii_name[%s] matched\n", priiloaderfiles[filecntr].name);
			content_Backedup = true;
			break;
		}
	}
	if(!content_Backedup) {
		sprintf(content_to_backup_Path, "%s:/%s/%s", device_Name(fatdevicemounted), themedir, getsavename(systemmenuVersion));
		if(Debugger) logfile("content_to_backup_Path[%s]\n", content_to_backup_Path);
		ContentFile = fopen(content_to_backup_Path, "rb");
		if(!ContentFile) {
			if(Debugger) logfile("unable to open %s .\n", content_to_backup_Path);
			return;
		}
		backup_original_Content(ContentFile, content_File);
		fclose(ContentFile);
	}
	
	return;
}
void theme_list_menu() {
	u32 cnt, buttons;
	s32 index;
	int success = 0;
	char filepath[256];
	//FILE *tmpfile;
	int list_start_line = 165;
	s32 Ios = IOS_GetVersion();
	
	if(fatdevicemounted <= 0) return;
	
	filecnt = filelist_retrieve();
	
	for(;;) {
		draw_System_Info(Ios);
		sprintf(textbuf, "[%i] Theme files . Select a Theme :", (filecnt <= 0 ? 0 : filecnt));
		WriteCentre(120, textbuf);
		if(filecnt <= 0) {
			WriteCentre(230, "[-] No Files Found .");
			DrawFrameFinish();
			return;
		}
		else {
			for (cnt = start; cnt < filecnt; cnt++) {
				// Files per page limit 
				if ((cnt - start) >= FILES_PER_PAGE)
				break;
				
				sprintf(textbuf, "  ->  %s", themefile[cnt].name);
				sprintf(textbuf2, "   %s", themefile[cnt].name);
				// Selected file 
				(cnt == selected) ? WriteFont(20, list_start_line, textbuf) : WriteFont(20, list_start_line, textbuf2);
				list_start_line += 25;
			}
			list_start_line = 165;
		}
		
		WriteFont(60, 310, "[Up]/[Down] Navigate Menu");
		WriteFont(60, 335, "[A] Select Theme .  [B] Back");
		WriteFont(60, 360, "[+] Download/Install Original Theme");
		WriteFont(60, 385, "[Home] Return To");
		
		DrawFrameFinish();
		
		buttons = wpad_waitbuttons();
		
		if (buttons == WPAD_BUTTON_UP) { 
			selected--; 
			//logfile("1selected[%i]  list_start_line[%i]\n", selected, list_start_line);
		}
		else if (buttons == WPAD_BUTTON_DOWN) { selected++; }
		else if (buttons == WPAD_BUTTON_B) {
			filecnt = 0;
			selected = 0;
			start = 0;
			return;
		}
		else if (buttons == WPAD_BUTTON_A) {
			if(filecnt <= 0)
				return;
			theme_manage_menu();
		}
		else if(buttons == WPAD_BUTTON_HOME) exit_Program();
		else if(buttons == WPAD_BUTTON_LEFT) { selected -= FILES_PER_PAGE; cnt -= 5;}
		else if(buttons == WPAD_BUTTON_RIGHT) { selected += FILES_PER_PAGE; cnt += 5;}
		//logfile("1selected[%i]  cnt[%i]\n", selected, cnt);
		
		if(buttons == WPAD_BUTTON_PLUS) {
			
			sprintf(filepath, "%s:/%s/%s", device_Name(fatdevicemounted), themedir, getsavename(systemmenuVersion));
			if(!Fat_CheckFile(filepath)) {
				success = downloadApp();
				if(success <= 0) {
					if(Debugger) logfile("unable to download .\n");
					return;
				}
				else {
					backup_content_to_Nand(content_name_no_Extension(systemmenuVersion));
				}
			}
			else {if(Debugger) logfile("Found [%s].\n", filepath);}
		}
		
		if (selected <= -1) {
			selected = filecnt - 1;
			list_start_line = 165;
		}
		if (selected >= filecnt) {
			selected = 0;
			list_start_line = 165;
		}
		//logfile("2selected[%i]  cnt[%i]\n", selected, cnt);
		// List scrolling 
		index = (selected - start);

		if (index >= FILES_PER_PAGE) {
			start += index - (FILES_PER_PAGE - 1);
			list_start_line = 165;
		}
		if (index <= -1) {
			start += index;
			list_start_line = 165;
		}
		//logfile("3selected[%i]  cnt[%i]\n\n", selected, cnt);
	}
	
	return;
}
int main() {
	bool Exit_App = false;
	//u32 number_of_patches = 0, AHBPROT_Patched = 0;
	s32 Ios = 0;
	
	__exception_setreload(5);
	
	Ios = IOS_GetVersion();
	/*if(Debugger) {
		fatdevicemounted = Fat_Mount(SD);
		if(!fatdevicemounted) {
			fatdevicemounted = Fat_Mount(USB);
			logfile("Debugging of MyMenuifyMod Started \n");
		}
	}*/
	if(AHBPROT_DISABLED) {
		//AHBPROT_Patched = 
		IOSPATCH_AHBPROT();
		/*if(!AHBPROT_Patched) {
			if(Debugger) logfile("Unable to patch AHBPROT %i \n", AHBPROT_Patched);
			
		}
		else {
			if(Debugger) logfile("Patched AHBPROT %i \n", AHBPROT_Patched);
		}*/
		//number_of_patches = 
		IOSPATCH_Apply();
	}
	else 
		//number_of_patches = 
		IOSPATCH_Apply();
	//if(Debugger) logfile("number_of_patches [%i] \n", number_of_patches);
	Initialise();
	if(!fatdevicemounted) {
		fatdevicemounted = Fat_Mount(SD);
	}
	if(!fatdevicemounted) {
		fatdevicemounted = Fat_Mount(USB);
	}
	if(!fatdevicemounted) {
		draw_System_Info(Ios);
		WriteCentre(220, "Unable to mount Sd or Usb .");
		WriteCentre(260, "Exiting ...");
		DrawFrameFinish();
		sleep(1);
		exit_Program();
	}
	systemmenuVersion = GetSysMenuVersion();
	if(Debugger) logfile("systemmenuversion = %d\n", systemmenuVersion);
	if(systemmenuVersion > 610) {
		if(Debugger) logfile("custom systemmenuversion = %d\n", systemmenuVersion);
		draw_System_Info(Ios);
		WriteCentre(220, "Custom System Version Detected .");
		WriteCentre(240, "Unable to Install Themes .");
		WriteCentre(260, "Exiting .");
		DrawFrameFinish();
		exit_Program();
	} 
	priiloader_found = checkforpriiloader();
	if(!priiloader_found) 
		nopriiloadermessage();
	read_MMM_Config(fatdevicemounted);
	if(!disable_Disclaimer) {
		Exit_App = Disclaimer();
		if(Exit_App) 
			exit_Program();
	}
	
	if(!disable_wDance) 
		W_Dance(4);
	else {
		draw_System_Info(Ios);
		WriteCentre(220, "MyMenuifyMod");
		WriteCentre(260, "WELCOME");
		DrawFrameFinish();
	}
	sleep(1);
	for(;;) {
		
		fatdevicemounted = theme_device_menu();
		
		theme_list_menu();
	}
	return 0;
}
