#include <stdio.h>
#include <stdlib.h>
#include <ogcsys.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <sys/dir.h>
#include <network.h>
#include <ogc/ios.h>
#include <stdbool.h>
#include <stddef.h>
#include <gctypes.h>
#include <ogc/es.h>
#include <ogc/ipc.h>
#include <ogc/isfs.h>

#include "wpad.h"
#include "IPLFontWrite.h"
#include "video.h"
#include "sddebug.h"
#include "fat.h"
#include "libpng/pngu/pngu.h"
#include "iospatch.h"
#include "http.h"
#include "rijndael.h"
#include "network.h"

#define KNOWN_THEME_CONTENTS   18
#define MB_SIZE		           1048576.0
#define FILES_PER_PAGE		   5
#define BLOCK_SIZE	           0x1000
#define CHUNKS                 1000000
#define MAX_SIZE_HTTP          0xFFFFFFFF
#define MAGIC_WORD_ADDRESS     0x8132FFFB
#define MAGIC_WORD_ADDRESS2    0x817FEFF0 //0x8132FFFB

IMGCTX ctx;
int fatdevicemounted = 0;
bool Debugger = false;
GXRModeObj *vmode = NULL;
u32 *xfb[2] = { NULL, NULL };
int whichfb = 0;
u32 system_version;
char textbuf[256] = "";
bool system_is_vWii = false;
const char *themedir = "themes";
static s32 filecnt = 0, start = 0, selected = 0;
char textbuf2[256] = "";
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
Fatfile *themefile = NULL;
themeInfo currentTheme;
u32 known_Versions[KNOWN_THEME_CONTENTS] = {416, 417, 418, 448, 449, 450, 454, 480, 481, 482, 486, 512, 513, 514, 518, 608, 609, 610};
char *regions[KNOWN_THEME_CONTENTS] =      {"J", "U", "E", "J", "U", "E", "K", "J", "U", "E", "K", "J", "U", "E", "K", "J", "U", "E"};
char *knownappfilenames[KNOWN_THEME_CONTENTS] = {"0000006f.app", "00000072.app", "00000075.app", "00000078.app", "0000007b.app", "0000007e.app", "00000081.app", "00000084.app", "00000087.app", "0000008a.app", "0000008d.app", "00000094.app", "00000097.app", "0000009a.app", "0000009d.app", "0000001c.app", "0000001f.app", "00000022.app"};
char *known_backup_names[KNOWN_THEME_CONTENTS] = {"0000006f_bkup.app", "00000072_bkup.app", "00000075_bkup.app", "00000078_bkup.app", "0000007b_bkup.app", "0000007e_bkup.app", "00000081_bkup.app", "00000084_bkup.app", "00000087_bkup.app", "0000008a_bkup.app", "0000008d_bkup.app", "00000094_bkup.app", "00000097_bkup.app", "0000009a_bkup.app", "0000009d_bkup.app", "0000001c_bkup.app", "0000001f_bkup.app", "00000022_bkup.app"};
static vu32 *_wiilight_reg = (u32*) 0xCD0000C0;
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
"AODRK1",
"ARSNFC",
"FALCON",
"BKGN01",
"BRCLNA",
"BTMN01",
"BTMN02",
"BIGG01",
"BILLY1",
"BLKGD1",
"BLMG01",
"BLPR01",
"BLPR02",
"BLCH01",
"BWBETA",
"BOBO01",
"BDSTS1",
"BSTRD1",
"BWSR01",
"BRLY01",
"BILLS1",
"CODTY1",
"CAR001",
"PNTHR1",
"CARS01",
"CSTLV1",
"CELTIC",
"CHNSAW",
"CKMO01",
"BEARS1",
"HAWKS1",
"BULLS1",
"CRTRG1",
"CLKWK1",
"CLBPN1",
"GEASS1",
"CONDT1",
"CONST1",
"CWBYS1",
"DKUB01",
"DKUB02",
"DKWI+1",
"DWORI1",
"DWBLU1",
"DWBLE1",
"DWBLJ1",
"DWBLK1",
"DWGRU1",
"DWGRE1",
"DWGRJ1",
"DWGRK1",
"DWORU1",
"DWORE1",
"DWORJ1",
"DWORK1",
"DWPKU1",
"DWPKE1",
"DWPKJ1",
"DWPKK1",
"DWPRU1",
"DWPRE1",
"DWPRJ1",
"DWPRK1",
"DWRDU1",
"DWRDE1",
"DWRDJ1",
"DWRDK1",
"DWWHU1",
"DWWHE1",
"DWWHJ1",
"DWWHK1",
"DWYLU1",
"DWYLE1",
"DWYLJ1",
"DWYLK1",
"DNOTE1",
"DKLOK1",
"LIONS1",
"DIABL1",
"DSCRD1",
"DGMN01",
"DOLPH1",
"DBLZ01",
"DBLZ02",
"DRWHO1",
"DUKES1",
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
"FMTLU1",
"FMTLE1",
"FMTLJ1",
"FMTLK1",
"FUTUR1",
"GAARA1",
"GRFLD1",
"GOWAR1",
"GBUST1",
"GSUN01",
"GOTH01",
"GRTFL1",
"PCKRS1",
"HDRAW1",
"HKITY1",
"HELLK1",
"HEMAN1",
"HEROS1",
"HNDRD1",
"ILLOG1",
"IMPOR1",
"INBET1",
"COLTS1",
"ICP001",
"IMMAR1",
"JNSBOB",
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
"MNMS01",
"MWRLD1",
"MWRLD2",
"MAMSK1",
"UNITED",
"MNHNT1",
"MARIA1",
"MARIO2",
"MKART1",
"MABEL1",
"MATRX1",
"MATRX2",
"MEGMN1",
"MOHAR1",
"MGSOL1",
"MTLCA1",
"MTROD1",
"METPR1",
"MTDSV1",
"METS01",
"MISTF1",
"MODMII",
"MONKEY",
"MONOW1",
"CANAD1",
"MKOMB1",
"MUSE01",
"NARTO1",
"SAINT1",
"NEPAT1",
"RANGR1",
"YANKE1",
"NMB4X1",
"NIDRM1",
"NRVNA1",
"NOMRH1",
"RAIDR1",
"OASIS1",
"OKAMI1",
"OSNIN1",
"OTLWS1",
"OZZY01",
"PLJAM1",
"POMAD1",
"PHWRT1",
"PIKMN1",
"PKFLD1",
"PNKWI1",
"PRSKL1",
"PENGUN",
"STELR1",
"PIZTR1",
"PREDR1",
"PARIE1",
"PARIE2",
"PARIE3",
"PSYCO1",
"PNOUT1",
"PUNSH1",
"ORTON1",
"RCLNK1",
"RC2402",
"RC2401",
"RESEVE",
"RMYST1",
"RHYTH1",
"RMORT1",
"RCHCK1",
"RBAND1",
"RBLOD1",
"SAW001",
"SCRFC1",
"SECOM1",
"SEINF1",
"SENDO1",
"SHADH1",
"SHANA1",
"SHNDWN",
"SILVH1",
"SMASH1",
"SMOKE1",
"SNOOP1",
"SONIC3",
"SNCFT1",
"SNCRD1",
"SONOA1",
"STHPK1",
"SPAWN1",
"SPICE1",
"SPDMN1",
"SPONG1",
"SQUBL1",
"STCFT1",
"STGTE1",
"STWRS1",
"STWRS2",
"STWII1",
"STRMU1",
"STRME1",
"STRMJ1",
"STRMK1",
"STRFT1",
"SHSQU1",
"SMARB3",
"SMRPG1",
"SMSUN1",
"SPAPM1",
"SSONI1",
"SIMPS1",
"SIMPS2",
"SIMPS3",
"TAILS1",
"TERMR1",
"TERNG1",
"TCATS1",
"TMNT01",
"TRAID1",
"LEAFS1",
"TOTDR1",
"TTOON1",
"TOYST1",
"TRANS1",
"TRLOM1",
"TRGUN1",
"TRPTL1",
"TBLOD1",
"UDWII1",
"CANUCK",
"VEGET1",
"VISTA1",
"WALEY1",
"WARIO1",
"CAPTLS",
"WSTRI1",
"WHITE1",
"WIID01",
"WIFIT1",
"WIIPT1",
"WIIPT2",
"WSPOR1",
"WIIU01",
"WIN701",
"WINXP1",
"WOLVE1",
"WWERW1",
"XBOX01",
"GYOSHI",
"YUGIO1",
"ZELDA1",
"ZELDA2",
"ZELDA3",
"ZELDA4",
"ZOMB01"
};
char *theme_Name[] = { "Among Us v1",
"Among Us v2",
"Animal Crossing",
"Animal Crossing v2",
"Apple",
"Aqua Teen Hunger Force",
"Army of Darkness",
"Arsenal FC",
"Atlanta Falcons",
"Bakugan",
"FC Barcelona",
"Batman v1",
"Batman v2",
"Notorious B.I.G.",
"Billy Mays",
"Black Gold",
"Black Mage",
"Black Pirate",
"Black Pirate v2",
"Bleach",
"Blue Wii Beta",
"BoBoBo",
"Boondock Saints",
"Boston Red Socks",
"Bowser",
"Broly",
"Buffalo Bills",
"Call of Duty",
"Car",
"Carolina Panthers",
"Cars",
"Castlevania",
"Celtic FC",
"Chainsaw Man",
"Check Mii Out",
"Chicago Bears",
"Chicago Black Hawks",
"Chicago Bulls",
"Chrono Trigger",
"Clock Work Orange",
"Club Penguin",
"Code Geass",
"Conduit",
"Constantine",
"Dallas Cowboys",
"Dark Umbra v1",
"Dark Umbra v2",
"Dark Wii Plus",
"Dark Wii Original",
"Dark Wii Blue U",
"Dark Wii Blue E",
"Dark Wii Blue J",
"Dark Wii Blue K",
"Dark Wii Green U",
"Dark Wii Green E",
"Dark Wii Green J",
"Dark Wii Green K",
"Dark Wii Orange U",
"Dark Wii Orange E",
"Dark Wii Orange J",
"Dark Wii Orange K",
"Dark Wii Pink U",
"Dark Wii Pink E",
"Dark Wii Pink J",
"Dark Wii Pink K",
"Dark Wii Purple U",
"Dark Wii Purple E",
"Dark Wii Purple J",
"Dark Wii Purple K",
"Dark Wii Red U",
"Dark Wii Red E",
"Dark Wii Red J",
"Dark Wii Red K",
"Dark Wii White U",
"Dark Wii White E",
"Dark Wii White J",
"Dark Wii White K",
"Dark Wii Yellow U",
"Dark Wii Yellow E",
"Dark Wii Yellow J",
"Dark Wii Yellow K",
"Death Note",
"Deth Klok",
"Detroit Lions",
"Diablo 3",
"Discord",
"Dog Man",
"Dolphins",
"Dragon Ball Z v1",
"Dragon Ball Z v2",
"Dr Who",
"Dukes of Hazzard",
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
"Full Metal Alchemist U",
"Full Metal Alchemist E",
"Full Metal Alchemist J",
"Full Metal Alchemist K",
"Futurama",
"Gaara",
"Garfield",
"Gears of War",
"Ghost Busters",
"Golden Sun",
"Gothic",
"Grateful Dead",
"Green Bay Packers",
"Hand Drawn",
"Hello Kitty",
"Hell's Kitchen",
"He-Man",
"Heros",
"The Hundreds",
"Illusions of Gaia",
"Imports",
"In Betweeners",
"Indianapolis Colts",
"Insane Clown Posse",
"Its A Me Mario",
"Jay & Silent Bob",
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
"M & M's",
"Mad World",
"Mad World v2",
"Majoras Mask",
"Manchester United",
"Man Hunt",
"Maria",
"Mario",
"Mario Kart",
"Martin Abel Art",
"Matrix",
"Matrix Reloaded",
"MegaMan",
"Melancholy of Haruhi",
"Metal Gear Solid",
"Metallica",
"Metroid",
"Metroid Prime",
"Metroid: Samus's Visor",
"Mets",
"Mist Forest",
"ModMii",
"Monkeys",
"Monopol - Wii",
"Montreal Canadians",
"Mortal Kombat",
"Muse",
"Naruto",
"New Orleans Saints",
"New England Patriots",
"New York Rangers",
"New York Yankees",
"Nightmare B4 Xmas",
"Nights into Dreams",
"Nirvana",
"No More Heros",
"Oakland Raiders",
"Oasis",
"Okami",
"Old School Nintendo",
"Outlaw Star",
"Ozzy",
"Pearl Jam",
"Penguins of Madagascar",
"Phoenix Wright",
"Pikmin",
"Pink Floyd",
"Pink Wii",
"Pirate Skulls",
"Pittsburgh Penguins",
"Pittsburgh Steelers",
"Pizza Tower",
"Predator",
"Princess Ariel v1",
"Princess Ariel v2",
"Princess Ariel v3",
"Psychedelic",
"Punch Out",
"The Punisher",
"Randy Orton",
"Ratchet and Clank",
"Reconnect 24 Blue",
"Reconnect 24 Red",
"Resident Evil 4",
"Rey Mysterio",
"Rhythm Heaven",
"Rick and Morty",
"Robot Chicken",
"Rockband 2",
"Rondo of Blood",
"Saw",
"ScarFace",
"Secrets of Mana",
"Seinfeld",
"Sendo World",
"Shadow The Hedgehog",
"Shakugan no Shana",
"ShineDown",
"Silver The Hedgehog",
"Smash Brothers Brawl",
"Smokers",
"Snoopy",
"Sonic 3",
"Sonic Frontiers",
"Sonic Riders",
"Sons of Anarchy",
"South Park",
"Spawn",
"Spice & Wolf",
"Spiderman",
"SpongeBob",
"Squid Billies",
"StarCraft",
"Star Gate",
"Star Wars",
"Star Wars Unleashed",
"Steel Wii",
"Storms U",
"Storms E",
"Storms J",
"Storms K",
"Street Fighter",
"Super Hero Squad",
"Super Mario Brothers 3",
"Super Mario RPG",
"Super Mario Sunshine",
"Super Paper Mario",
"Super Sonic",
"The Simpsons v1",
"The Simpsons v2",
"The Simpsons v3",
"Tails",
"The Terminator",
"Terra Nigma",
"Thunder Cats",
"Teenage Mutant Ninja Turtles",
"Tomb Raider",
"Toronto Maple Leafs",
"Total Drama Action",
"Toxic Toons",
"Toy Story",
"Transformers",
"Trials of Mana",
"Tri-Gun",
"Tropical Teal",
"True Blood",
"Ultimate Dark Wii",
"Vancouver Canucks",
"Vegeta",
"Vista",
"Walleye",
"Wario Ware",
"Washington Capitals",
"White Stripes",
"White Wii",
"Wiid",
"Wii Fit",
"Wii Party",
"Wii Party v2",
"Wii Sports",
"Wii U",
"Windows 7",
"Win XP OS",
"Wolverine",
"WWE Raw",
"Xbox 360",
"Yoshi",
"Yugi-oh",
"Zelda",
"Zelda: A Link to the Past",
"Zelda: Minish Cap",
"Zelda v2",
"ZombWii"
};

void wiilight(int enable) {
    u32 val = (*_wiilight_reg & ~0x20);
    if (enable) val |= 0x20;
    *_wiilight_reg = val;
}
void system_exit_Menu(void) {
	wiilight(1);
	// Return to the Wii system menu
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}
int system_Exit_Priiloader() {
	wiilight(1);
	//retarded that this is the only way without touching the settings of priiloader or load the dol...
	//logfile("magic word is %x\n",*(vu32*)MAGIC_WORD_ADDRESS);
	*(vu32*)MAGIC_WORD_ADDRESS = 0x4461636f; // "Daco" , causes priiloader to skip autoboot and load the priiloader menu
	//*(vu32*)MAGIC_WORD_ADDRESS = 0x50756e65; // "Pune" , causes priiloader to skip autoboot and load Sys Menu
	*(vu32*)MAGIC_WORD_ADDRESS2 = *(vu32*)MAGIC_WORD_ADDRESS;
	DCFlushRange((void*)MAGIC_WORD_ADDRESS, 4);
	DCFlushRange((void*)MAGIC_WORD_ADDRESS2, 4);
	//logfile("magic word changed to %x\n",*(vu32*)MAGIC_WORD_ADDRESS);

	SYS_ResetSystem(SYS_RETURNTOMENU,0,0);
	wiilight(0);
	return 0;
}
void system_exit_HBC() {
	wiilight(1);
	exit(0);
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
bool read_Settings() {
	
	char filepath[256];
	FILE *settings_File;
	
	sprintf(filepath, "%s:/apps/mymenuifymod/settings.txt", device_Name(fatdevicemounted));
	if(Debugger) logfile("filepath[%s]\n", filepath);
	settings_File = fopen(filepath, "rb");
	if(!settings_File) return false;
	fclose(settings_File);
	
	return true;
}
bool write_Settings() {
	char filepath[512];
	FILE *settings_File;
	const char *file_contents = NULL;
	
	sprintf(filepath, "%s:/apps/mymenuifymod/settings.txt", device_Name(fatdevicemounted));
	logfile("filepath[%s]\n", filepath);
	settings_File = fopen(filepath, "ab");
	if(!settings_File) {
		logfile("Unable to open file [%s:/apps/mymenuifymod/settings.txt]\n", device_Name(fatdevicemounted));
		return false;
	}
	file_contents = "Disclaimer disabled";
	fprintf(settings_File, file_contents);
	fclose(settings_File);
	
	return true;
}
bool Disclaimer() {
	u32 buttons;
	bool user_choice;
	
	for(;;) {
		DrawFrameStart();
		WriteCentre(40, "[DISCLAIMER] :");
		WriteFont(50, 110, "THIS APPLICATION COMES WITH NO    ");
		WriteFont(50, 140, "WARRANTY AT ALL, NEITHER EXPRESSED");
		WriteFont(50, 170, "NOR IMPLIED . I DO NOT TAKE ANY   ");
		WriteFont(50, 200, "RESPONSIBILITY FOR ANY DAMAGE TO  "); 
		WriteFont(50, 230, "YOUR WII/WIIU CONSOLE BECAUSE OF  ");
		WriteFont(50, 260, "IMPROPER USE OF THIS SOFTWARE .   ");
		WriteFont(50, 350, "[A] Continue                        [B] Exit"); 
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
const char *get_display_region(u32 num) {
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
const char *get_display_version(u32 num) {
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
s32 GetTMD(u64 tid, signed_blob **outbuf, u32 *outlen) {
	void *p_tmd = NULL;

	u32 len;
	s32 ret;

	/* Get TMD size */
	ret = ES_GetStoredTMDSize(tid, &len);
	if (ret < 0)
		return ret;

	/* Allocate memory */
	p_tmd = allocate_memory(len);
	if (!p_tmd)
		return -1;

	/* Read TMD */
	ret = ES_GetStoredTMD(tid, p_tmd, len);
	if (ret < 0)
		goto err;

	/* Set values */
	*outbuf = p_tmd;
	*outlen = len;

	return 0;

err:
	/* Free memory */
	free(p_tmd);

	return ret;
}
s32 GetVersion(u64 tid, u16 *outbuf, bool *vWii) {
	signed_blob *p_tmd = NULL;
	tmd      *tmd_data = NULL;

	u32 len;
	s32 ret;

	/* Get title TMD */
	ret = GetTMD(tid, &p_tmd, &len);
	if (ret < 0)
		return ret;

	/* Retrieve TMD info */
	tmd_data = (tmd *)SIGNATURE_PAYLOAD(p_tmd);

	/* Set values */
	*outbuf = tmd_data->title_version;
	*vWii = (bool)tmd_data->vwii_title;
	/* Free memory */
	free(p_tmd);

	return 0;
}
u32 Get_system_version() {
    //Get sysversion from TMD
    u64 TitleID = 0x0000000100000002LL;
	u16 version;
	bool is_vWii;
	GetVersion(TitleID, &version, &is_vWii);
	if(Debugger) logfile("is_vWii[%d]\n",is_vWii);
	system_is_vWii = is_vWii;
    return version;
}
u32 check_custom_system_version() {
	u32 nandfilecnt = 0, filecounter = 0, knownversioncounter = 0;
	char *knownversionstr = NULL;
	
	getdir("/title/00000001/00000002/content",&nandfilelist,&nandfilecnt);
	for(filecounter = 0; filecounter < nandfilecnt; filecounter++) {
		for(knownversioncounter = 0; knownversioncounter < KNOWN_THEME_CONTENTS; knownversioncounter++) {
			knownversionstr = knownappfilenames[knownversioncounter];
			if(strcmp(nandfilelist[filecounter].name, knownversionstr) == 0) return known_Versions[knownversioncounter];
		}
	}
	return 0;
}
char* find_theme_Content() {
	u32 nandfilecnt = 0, filecounter = 0, knownversioncounter = 0;
	char *knownversionstr = "";
	
	getdir("/title/00000001/00000002/content",&nandfilelist,&nandfilecnt);
	for(filecounter = 0; filecounter < nandfilecnt; filecounter++) {
		if(Debugger) logfile("file = %s\n", nandfilelist[filecounter].name);
		for(knownversioncounter = 0; knownversioncounter < KNOWN_THEME_CONTENTS; knownversioncounter++) {
			knownversionstr = knownappfilenames[knownversioncounter];
			if(!strcmp(nandfilelist[filecounter].name, knownversionstr)) return nandfilelist[filecounter].name;
		}
	}
	return "ERROR";
}
void draw_System_Info() {
	s32 Ios = IOS_GetVersion();
	sprintf(textbuf, "IOS: %i", Ios);       
	DrawFrameStart();
	WriteFont(50, 75, textbuf);
	sprintf(textbuf, "System Menu: %s_%s v%u", get_display_version(system_version), get_display_region(system_version), system_version);
	WriteFont(50, 50, textbuf);
	return;
}
void exit_Program() {
	const char *types[] = { "System Menu", "Home Brew Channel", "PriiLoader" };
	int type = 0;
	u32 buttons;
	
	for(;;) {
		draw_System_Info();
		sprintf(textbuf,"Exit To:   %s", types[type]);
		WriteFont(50, 110, textbuf);
		WriteFont(50, 320, "[Left]/[Right] Toggle Exit");
		WriteFont(50, 350, "[A] Select Exit  [B] Back");
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
		if(buttons == WPAD_BUTTON_B) return;
	}
	switch(type) {
		case 0:
			draw_System_Info();
			WriteFont(200, 180, "MyMenuifyMod      ");
			WriteFont(200, 220, "Exit System Menu  ");
			DrawFrameFinish();
			sleep(1);
			system_exit_Menu();
		break;
		case 1:
		default:
			draw_System_Info();
			WriteFont(200, 180, "MyMenuifyMod      ");
			WriteFont(200, 220, "Exit HBC          ");
			DrawFrameFinish();
			sleep(1);
			system_exit_HBC();
		break;
		case 2:
			draw_System_Info();
			WriteFont(200, 180, "MyMenuifyMod      ");
			WriteFont(200, 220, "Exit Priiloader   ");
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
        return "0000006f";
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
char *getsavename(u32 idx) {
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
        return "0000006f.app";
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
	u8 wii_common_key[16] = { 0xeb, 0xe4, 0x2a, 0x22, 0x5e, 0x85, 0x93, 0xe4, 0x48,0xd9, 0xc5, 0x45, 0x73, 0x81, 0xaa, 0xf7 };
	u8 vWii_common_key[16] = { 0x30, 0xBF, 0xC7, 0x6E, 0x7C, 0x19, 0xAF, 0xBB, 0x23, 0x16, 0x33, 0x30, 0xCE, 0xD7, 0xC2, 0x8D };
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
	return;
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
void options_Menu(int device) {
	u32 buttons;
	
	s32 selected_option = 0;
	const char *selected_options[] = {"Disable", "Enable "};
	const char *current_option[] = {"Disabled", "Enabled"};
	bool bugger;
	
	if(Debugger) bugger = false;
	else bugger = true;
	
	for(;;) {
		draw_System_Info();
		
		sprintf(textbuf,"Debugging : %s", current_option[Debugger]);
		WriteFont(75, 120, textbuf);
		if(selected_option == 0) 
			sprintf(textbuf,"->  %s Debugging ", selected_options[bugger]);
		else
			sprintf(textbuf,"    %s Debugging ", selected_options[bugger]);
		WriteFont(50, 170, textbuf);
		WriteFont(50, 300, "[B] Back");
		DrawFrameFinish();
		
		buttons = wpad_waitbuttons();
		if(buttons == WPAD_BUTTON_UP) {
			
		}
		if(buttons == WPAD_BUTTON_DOWN) {
			
		}
		if((buttons == WPAD_BUTTON_LEFT) || (buttons == WPAD_BUTTON_RIGHT)) {
			if(selected_option == 0)
				bugger ^= bugger;
		}
		if(buttons == WPAD_BUTTON_A) break;
		if(buttons == WPAD_BUTTON_B) return;
		if(buttons == WPAD_BUTTON_MINUS) {
			
		}
		if(buttons == WPAD_BUTTON_PLUS) {

		} 
		if (buttons == WPAD_BUTTON_1) {
				
		}
		if (buttons == WPAD_BUTTON_2) {
			
		}
		if (buttons == WPAD_BUTTON_HOME) {
			exit_Program();
		}	
	}
	if(selected_option == 0) {
		draw_System_Info();
		if(bugger == Debugger) return;
		if(bugger == true) {
			Debugger = true;
			WriteFont(50, 230, "Debugger Enabled .");
		}
		else {
			Debugger = false;
			WriteFont(50, 230, "Debugger Disabled .");
		}	
		DrawFrameFinish();
		logfile("debugger[%d]\n", Debugger);
		sleep(1);
	}
	return;
}
int theme_device_menu() {
	int device = SD, fat_unmount_device = -1;
	u32 buttons;
	
	for(;;) {
		draw_System_Info();
		WriteFont(50, 110, "Select Device :");
		sprintf(textbuf," %s ", device_Name(device));
		WriteFont(50, 160, textbuf);
		WriteFont(50, 280, "[Left]/[Right] Toggle Device");
		WriteFont(50, 305, "[A] Select Device");
		WriteFont(50, 330, "[Home]/[B] Return To");
		WriteFont(50, 355, "[1] Options");
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
		WriteCentre(160, textbuf);
		DrawFrameFinish();
		sleep(1);
	}
	else {
		fat_unmount_device = Fat_Unmount(fatdevicemounted);
		if (fat_unmount_device < 0) {
			sprintf(textbuf, "Unmounting %s ..... Failed .", device_Name(fatdevicemounted));
			WriteCentre(160, textbuf);
			sprintf(textbuf, "[-] Unable to unmount %s .", device_Name(fatdevicemounted));
			WriteCentre(190, textbuf);
			WriteCentre(220, "Press any button to continue .");
			DrawFrameFinish();
			wpad_waitbuttons();
			fatdevicemounted = Fat_Mount(device);
		}
		else {
			fatdevicemounted = Fat_Mount(device);
			if(fatdevicemounted < 0) {
				sprintf(textbuf, "Mounting %s ..... Failed .", device_Name(device));
				WriteCentre(160, textbuf);
				sprintf(textbuf, "[-] Unable to mount %s .", device_Name(device));
				WriteCentre(190, textbuf);
				WriteCentre(220, "Press any button to continue .");
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
// retrieve size of install file 
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
		WriteCentre(120, "Unsigned Theme Detected !");

		WriteFont(40, 160, "It is recommended to use www.wiithemer.org,");
		WriteFont(40, 185, "ModMii, or wii theme manager to build safe");
		WriteFont(40, 210, "and verified themes ,");
		WriteFont(40, 235, "Only install this file if you made it");
		WriteFont(40, 260, "or trust where it came from .");
		WriteFont(40, 285, "You may continue at own risk .");

		WriteFont(50, 325, "[A] Continue .");
		WriteFont(50, 350, "[B] Back .");
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
	
	for(;;) {
		draw_System_Info();
		WriteCentre(90, "PriiLoader Not Detected !");
		WriteCentre(120, "-------------------------------");
		WriteFont(50, 170, "It is recommended to have Priiloader");
		WriteFont(50, 200, "installed as Boot2 or Ios before");
		WriteFont(50, 230, "using this Program .");
		WriteFont(50, 270, "You may Continue at your Own risk .");

		WriteFont(50, 325, "[A]  Continue .");
		WriteFont(50, 350, "[B]  Exit .");
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
int signature_Offset = 0;
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
	signature_Offset = content_data_counter;
	return is_content_signed;
}
char *themeName = NULL;
char data_id[7] = {'-', '-', '-', '\0'};
char data_spin[7] = {'-', '-', '-', '\0'};
char data_content[3] = {'-', '-', '\0'};
bool check_Id_Signature(s32 version, int offset) {
	FILE *content_File;
	char content_file_Path[2048];
	u32 content_file_Size = 0;
	u8 *content_Data = NULL;
	
	offset = offset - 16;
	
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
	fclose(content_File);
	if(content_Data[offset] != 0) {
		free(content_Data);
		content_Data = NULL;
		return true;
	}
		
	free(content_Data);
	content_Data = NULL;
	return false;
}
void find_theme_Info(s32 version, int offset) {
	if(Debugger) logfile("Find theme Info \n");
	
	FILE *content_File;
	char content_file_Path[2048];
	u32 content_file_Size = 0;
	u8 *content_Data = NULL;
	int theme_id_len = 0;
	offset = offset - 16;
	check_Id_Signature(version, offset);
	
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
	fclose(content_File);
	if(content_Data[offset] != 0x00) {
		data_id[0] = (char)content_Data[offset];
		data_id[1] = (char)content_Data[offset + 1];
		data_id[2] = (char)content_Data[offset + 2];
		data_id[3] = (char)content_Data[offset + 3];
		data_id[4] = (char)content_Data[offset + 4];
		data_id[5] = (char)content_Data[offset + 5];
		data_id[6] = '\0';
		if(Debugger) logfile("data_id[%s]\n", data_id);
		data_spin[0] = (char)content_Data[offset + 7];
		data_spin[1] = (char)content_Data[offset + 8];
		data_spin[2] = (char)content_Data[offset + 9];
		data_spin[3] = (char)content_Data[offset + 10];
		data_spin[4] = (char)content_Data[offset + 11];
		data_spin[5] = (char)content_Data[offset + 12];
		data_spin[6] = '\0';
		if(Debugger) logfile("data_spin[%s]\n", data_spin);
		data_content[0] = (char)content_Data[offset + 14];
		data_content[1] = (char)content_Data[offset + 15];
		data_content[2] = '\0';
		if(Debugger) logfile("data_content[%s]\n", data_content);	
	}
	else {
		fclose(content_File);
		free(content_Data);
		content_Data = NULL;
		return;
	}
		
	
	
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
s32 InstallFile(FILE * fp, bool one_button_install, bool has_theme_id) {
	char * data;
	s32 ret = -1, nandfile;
	u32 length = 0,numchunks, cursize, i;
	char filename[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	
	char *contentname;
	if(one_button_install) contentname = getsavename(system_version);
	
	char* content_name = stpcpy(filename, "/title/00000001/00000002/content/");
	sprintf(content_name, "%s", find_theme_Content());
	if(Debugger) logfile("filename[%s] length[%i]\n", filename, strlen(filename));
	nandfile = ISFS_Open(filename, ISFS_OPEN_RW);
	ISFS_Seek(nandfile, 0, SEEK_SET);
	length = filesize(fp);
	numchunks = length/CHUNKS + ((length % CHUNKS != 0) ? 1 : 0);
	draw_System_Info();
	
	if(has_theme_id) sprintf(textbuf, "Installing %s", (one_button_install == true ? contentname : themeName));
	else sprintf(textbuf, "Installing %s", (one_button_install == true ? contentname : themefile[selected].name));
	WriteFont(50, 135, textbuf);
	sprintf(textbuf, "[+] Total parts: %d", numchunks);
	WriteFont(75, 170, textbuf);
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
		draw_System_Info();
		if(has_theme_id) sprintf(textbuf, "Installing %s", (one_button_install == true ? contentname : themeName));
		else sprintf(textbuf, "Installing %s", (one_button_install == true ? contentname : themefile[selected].name));
		WriteFont(50, 135, textbuf);
		sprintf(textbuf, "[+] Total parts: %d", numchunks);
		WriteFont(75, 170, textbuf);
		sprintf(textbuf, "Installing part %d", (i + 1));
		WriteFont(75, 200, textbuf);
		DrawFrameFinish();
		ret = fread(data, 1, CHUNKS, fp);
		if (ret < 0) 
		{
			logfile("\t[-] Error reading from SD ! (ret = %d)\n\n", ret);
			sprintf(textbuf, "[-] Error reading from SD ! (ret = %d)", ret);
			WriteFont(75, 230, textbuf);
			WriteFont(75, 260, "Press any button to continue .");
			DrawFrameFinish();
			wpad_waitbuttons();
			return -2;
		}
		else
		{
			cursize = ret;
		}
		wiilight(1);
		draw_System_Info();
		if(has_theme_id) sprintf(textbuf, "Installing %s", (one_button_install == true ? contentname : themeName));
		else sprintf(textbuf, "Installing %s", (one_button_install == true ? contentname : themefile[selected].name));
		WriteFont(50, 135, textbuf);
		sprintf(textbuf, "[+] Total parts: %d", numchunks);
		WriteFont(75, 170, textbuf);
		sprintf(textbuf, "Installing part %d", (i + 1));
		WriteFont(75, 200, textbuf);
		ret = ISFS_Write(nandfile, data, cursize);
		if(ret < 0)
		{
			logfile("\t[-] Error writing to NAND ! (ret = %d)\n\n", ret);
			sprintf(textbuf, "[-] Error writing to NAND ! (ret = %d)", ret);
			WriteFont(75, 230, textbuf);
			WriteFont(75, 260, "Press any button to continue .");
			DrawFrameFinish();
			wpad_waitbuttons();
			wiilight(0);
			return ret;
		}
		free(data);
		
		WriteFont(75, 230, "Complete .");
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
	u32 size, buttons, x;
	bool install_file = false;
	bool is_content_file = false;
	bool found_backup_name = false;
	bool acknowledge_theme_unsigned = false;
	//s32 Ios = IOS_GetVersion();
	draw_System_Info();
	WriteFont(50, 140, "Gathering Info ... ");
	DrawFrameFinish();
	int content_has_signature = check_file_Signature();
	if(content_has_signature == 0) {
		for(x = 0; x < KNOWN_THEME_CONTENTS; x++) {
			if(!strcmp(known_backup_names[x], themefile[selected].name)) found_backup_name = true;
		}
		if(!found_backup_name) {
			acknowledge_theme_unsigned = warnunsignedtheme();
			if(!acknowledge_theme_unsigned) return;
		}
	}
	u32 install_version = verify_content_file_Version(themefile[selected].name);
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
	if(content_has_signature) {
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
		find_theme_Info(install_version, signature_Offset);
	}
	//draw_System_Info(Ios);
	//WriteFont(80, 140, "Gathering Info ... Complete .");
	//DrawFrameFinish();
	for(;;) {
		draw_System_Info();
		sprintf(textbuf, "Theme :  %s", (themeName != NULL ? themeName : themefile[selected].name));
		WriteFont(50, 110, textbuf);
		sprintf(textbuf, "File size :  %.2f MB", sizeoffile);
		WriteFont(50, 140, textbuf);
		if(themeName != NULL) {
			sprintf(textbuf, "Theme Id :  %s", data_id);
			WriteFont(50, 170, textbuf);
			sprintf(textbuf, "Channel Outline Spin Option   %s", data_spin);
			WriteFont(50, 200, textbuf);
			sprintf(textbuf, "Base Content : 000000%s.app    %s_%s", data_content, get_display_version(install_version), get_display_region(install_version));
			WriteFont(50, 230, textbuf);
		}
		if(content_has_signature != 0) {
			sprintf(textbuf, "Signature :  %s", signature_display_name(content_has_signature));
			WriteFont(50, 260, textbuf);
		}
		WriteFont(50, 325, "[Home]  Exit To");
		WriteFont(50, 350, "[A] Install   [B]  Back");
		
		DrawFrameFinish();
		
		buttons = wpad_waitbuttons();
		
		if(buttons == WPAD_BUTTON_HOME) return exit_Program();
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
	
	
	draw_System_Info();
	sprintf(textbuf, "Installing %s", (themeName != NULL ? themeName : themefile[selected].name));
	WriteFont(50, 135, textbuf);
	// check if file is a u8 archive
	is_content_file = Is_content_file_U8();
	if(Debugger) logfile("is_content_file[%i]\n", is_content_file);
	if(!is_content_file) {
		WriteFont(50, 175, "This File is not a U8 archive !");
		WriteFont(50, 200, "Unable to use this file .");
		WriteFont(50, 325, "Press any button to return");
		WriteFont(50, 350, "to the Selection Menu !");
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
	currentTheme.version = system_version;
	//if(currentTheme.version > 610)  currentTheme.version = checkcustomsystemmenuversion();
	if(Debugger) logfile("current theme version [%i]\n", currentTheme.version);
	currentTheme.region = find_content_Region(system_version);
	if(Debugger) logfile("current theme region [%i]\n", currentTheme.region);
	
	if(currentTheme.version != themefile[selected].version) { 
        sprintf(textbuf, "Installing %s - Failed", (themeName != NULL ? themeName : themefile[selected].name));
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
		sprintf(textbuf, "Installing %s ..... Failed", (themeName != NULL ? themeName : themefile[selected].name));
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
	draw_System_Info();
	sprintf(textbuf, "Installing %s", (themeName != NULL ? themeName : themefile[selected].name));
	WriteFont(50, 135, textbuf);
	sprintf(filepath, "%s:/%s/%s", device_Name(fatdevicemounted), themedir, themefile[selected].name);
	tmpfile = fopen(filepath, "rb");
	if(!tmpfile) {
		logfile("unable to open %s .\n", filepath);
		return;
	}
	DrawFrameFinish();
	InstallFile(tmpfile, false, (themeName != NULL ? true : false));
	fclose(tmpfile);
	draw_System_Info();
	sprintf(textbuf, "Installing %s ", (themeName != NULL ? themeName : themefile[selected].name));
	WriteFont(50, 135, textbuf);
	WriteFont(50, 170, "Complete .");
	DrawFrameFinish();
	free_Png();
	sleep(2);
	exit_Program();
	
	return;
}
int downloadApp() {
	//s32 rtn;
    u32 tmpversion;
    s32 ret;
    int counter;
	int retries = 50, retrycnt, switch_path = 0;
	char *savePath = memalign(32, 128);
    char *wiiserverlist[] = {"cetk", "tmd."};
	signed_blob * s_tik = NULL;
    signed_blob * s_tmd = NULL;
	u32 outlen = 0;
    u32 http_status = 0;
	char * wii_titleId = "0000000100000002";
	char *vWii_titleId = "0000000700000002";
	const char *wiishoppath = "http://nus.cdn.shop.wii.com/ccs/download";             // Nus Wii
	const char *wiiU_shoppath = "http://ccs.cdn.wup.shop.nintendo.net/ccs/download"; // Nus Wii U
	const char *RC24path = "http://ccs.cdn.sho.rc24.xyz/ccs/download";                // Nus Emu RC24
	char *titleId;
	const char *download_Path;
    tmpversion = Get_system_version();
	
    if(Debugger) logfile("dvers =%d \n", tmpversion);
    //if(tmpversion > 610) tmpversion = checkcustomsystemmenuversion();
	//if(!tmpversion) return -5;
	//content_is_vWii = is_content_vWii(tmpversion);
	if(system_is_vWii) titleId = vWii_titleId;
	else titleId = wii_titleId;
    draw_System_Info();
	//sprintf(textbuf, "Theme :  %s", themefile[selected].name);
	WriteFont(50, 140, "Initializing  Network ..... ");
	DrawFrameFinish();
	sleep(1);
    draw_System_Info();
    for(retrycnt = 0; retrycnt < retries; retrycnt++) {
        ret = net_init();
		if(ret == 0) { WriteFont(50, 140, "Initializing  Network ..... Complete ."); DrawFrameFinish(); break; }
		if(retrycnt >= 50) { WriteFont(50, 140, "Initializing  Network ..... Failed ."); DrawFrameFinish(); break; }
    }
	if(retrycnt >= 50) return ret;
	sleep(1);
	draw_System_Info();
	WriteFont(50, 140, "Downloading Content from");
	sprintf(textbuf, "System Menu v%d", tmpversion);
	WriteFont(50, 170, textbuf);
	DrawFrameFinish();
	sleep(2);
	
	download_Path = wiishoppath;
	for(counter = 0; counter < 3;) {	
        int app_pos = getslot(tmpversion);
        char *path = (char*)memalign(32, 256);
		if(counter == 0) {
            sprintf(path,"%s/%s/%s", download_Path, titleId, wiiserverlist[counter]);
            if(Debugger) logfile("path[%s]\nDowloading Ticket .... ", path);
			draw_System_Info();
			WriteFont(50, 140, "Downloading Content from");
			sprintf(textbuf, "System Menu v%d", tmpversion);
			WriteFont(50, 170, textbuf);
			WriteFont(75, 220, "Dowloading Ticket .... ");
			DrawFrameFinish();
			//sleep(1);
			ret = http_request(path, MAX_SIZE_HTTP, false);
			
        }
        else if(counter == 1) {
            sprintf(path,"%s/%s/%s%d", download_Path, titleId, wiiserverlist[counter], tmpversion);
            if(Debugger) logfile("Dowloading Tmd .... ");
			draw_System_Info();
			WriteFont(50, 140, "Downloading Content from");
			sprintf(textbuf, "System Menu v%d", tmpversion);
			WriteFont(50, 170, textbuf);
			WriteFont(75, 220, "Dowloading Tmd .... ");
			DrawFrameFinish();
			//sleep(1);
			ret = http_request(path, MAX_SIZE_HTTP, false);
			
        }
        else if(counter == 2) {
            sprintf(path,"%s/%s/%s", download_Path, titleId, appfilename_noext(app_pos));
            if(Debugger) logfile("Dowloading %s .... ", getsavename(tmpversion));
			draw_System_Info();
			WriteFont(50, 140, "Downloading Content from");
			sprintf(textbuf, "System Menu v%d", tmpversion);
			WriteFont(50, 170, textbuf);
			sprintf(textbuf, "Downloading %s .... ", getsavename(tmpversion));
			WriteFont(75, 220, textbuf);
			DrawFrameFinish();
			//sleep(1);
			ret = http_request(path, MAX_SIZE_HTTP, false);
			
        }
        if(ret == 0 ) {
            free(path);
			path = NULL;
            //logfile("download failed !! ret(%d)\n",ret);
            if(Debugger) logfile("Failed !! ret(%d)\n",ret);
			draw_System_Info();
			WriteFont(75, 220, "Downloading .... Failed .");
			DrawFrameFinish();
			sleep(1);
			download_Path = wiiU_shoppath;
			switch_path++;
			if(switch_path >= 2) {
				download_Path = RC24path;
			}
			if(switch_path >= 3)
				return -9;
			continue;
        }
        free(path);
		path = NULL;
		
        u8* outbuf = (u8*)malloc(outlen);
        if(counter == 0) {
			ret = http_get_result(&http_status, (u8 **)&s_tik, &outlen);
			draw_System_Info();
			WriteFont(50, 140, "Downloading Content from");
			sprintf(textbuf, "System Menu v%d", tmpversion);
			WriteFont(50, 170, textbuf);
			WriteFont(75, 220, "Dowloading Ticket .... Complete .");
			DrawFrameFinish();
			sleep(1);
		}
        if(counter == 1) {
			ret = http_get_result(&http_status, (u8 **)&s_tmd, &outlen);
			draw_System_Info();
			WriteFont(50, 140, "Downloading Content from");
			sprintf(textbuf, "System Menu v%d", tmpversion);
			WriteFont(50, 170, textbuf);
			WriteFont(75, 220, "Dowloading Tmd .... Complete .");
			DrawFrameFinish();
			sleep(1);
		}
        if(counter == 2) {
			ret = http_get_result(&http_status, &outbuf, &outlen);
			draw_System_Info();
			WriteFont(50, 140, "Downloading Content from");
			sprintf(textbuf, "System Menu v%d", tmpversion);
			WriteFont(50, 170, textbuf);
			sprintf(textbuf, "Downloading %s ....", getsavename(tmpversion));
			WriteFont(75, 220, textbuf);
			WriteFont(75, 250, "Complete .");
			DrawFrameFinish();
			sleep(1);
		}
        if(Debugger) logfile("\nDecrypting files ....");
		if(counter == 2) {
			draw_System_Info();
			WriteFont(50, 140, "Downloading Content from");
			sprintf(textbuf, "System Menu v%d", tmpversion);
			WriteFont(50, 170, textbuf);
			WriteFont(75, 220, "Decrypting File ....");
			DrawFrameFinish();
			sleep(1);
		}
		
        //set aes key
        u8 key[16];
        u16 index;
        get_title_key(s_tik, key, system_is_vWii);
        aes_set_key(key);
        u8* outbuf2 = (u8*)malloc(outlen);
        if(counter == 2) {
            if(outlen > 0) {//suficientes bytes
                index = 01;
                //then decrypt buffer
                decrypt_buffer(index,outbuf,outbuf2,outlen);
				if(Debugger) logfile("Complete !! \n\n");
				draw_System_Info();
				WriteFont(50, 140, "Downloading Content from");
				sprintf(textbuf, "System Menu v%d", tmpversion);
				WriteFont(50, 170, textbuf);
				WriteFont(75, 220, "Decrypting File .... Complete .");
				DrawFrameFinish();
				sleep(2);
				draw_System_Info();
				WriteFont(50, 140, "Downloading Content from");
				sprintf(textbuf, "System Menu v%d", tmpversion);
				WriteFont(50, 170, textbuf);
				sprintf(textbuf, "Saving %s to Storage Device ... ", content_name_no_Extension(tmpversion));
				WriteFont(75, 220, textbuf);
				DrawFrameFinish();
				
				if(Debugger) logfile("Saving file .....");
				sprintf(savePath, "%s:/themes/%s_bkup.app", device_Name(fatdevicemounted), content_name_no_Extension(system_version));
                ret = Fat_SaveFile(savePath, (void *)&outbuf2, outlen);
				
				draw_System_Info();
				WriteFont(50, 140, "Downloading Content from");
				sprintf(textbuf, "System Menu v%d", tmpversion);
				WriteFont(50, 170, textbuf);
				sprintf(textbuf, "Saving %s to Storage Device ...", content_name_no_Extension(tmpversion));
				WriteFont(75, 220, textbuf);
				WriteFont(75, 250, "Complete .");
				DrawFrameFinish();
				sleep(2);
            }
        }
        if(Debugger) logfile("Complete !! \n\n");
		
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
	
	if(savePath != NULL) {
		free(savePath);
		savePath = NULL;
	}
	
	sleep(2);
	
    return 1;
}
void theme_list_menu() {
	u32 cnt, buttons; 
	s32 index;
	int success = 0;
	int list_start_line = 135;
	bool original_content_backed_Up = false;
	char filename[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	FILE * saved_original_theme;
	if(fatdevicemounted <= 0) return;
	
	filecnt = filelist_retrieve();
	
	for(;;) {
		draw_System_Info();
		sprintf(textbuf, "[%i] Theme files . Select a Theme :", (filecnt <= 0 ? 0 : filecnt));
		WriteFont(50, 105, textbuf);
		if(filecnt <= 0) {
			WriteCentre(200, "[-] No Files Found .");
			DrawFrameFinish();
		}
		else {
			for (cnt = start; cnt < filecnt; cnt++) {
				// Files per page limit 
				if ((cnt - start) >= FILES_PER_PAGE)
				break;
				
				sprintf(textbuf, "  ->  %s", themefile[cnt].name);
				sprintf(textbuf2, "      %s", themefile[cnt].name);
				// Selected file 
				(cnt == selected) ? WriteFont(50, list_start_line, textbuf) : WriteFont(20, list_start_line, textbuf2);
				list_start_line += 25;
			}
			list_start_line = 135;
		}
		
		WriteFont(50, 275, "[Up]/[Down] Navigate Menu");
		WriteFont(50, 300, "[A] Select Theme .  [B] Back");
		WriteFont(50, 325, "[+] Download/Install Original Theme");
		WriteFont(50, 350, "[Home] Back");
		
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
			sprintf(filename, "%s:/themes/%s_bkup.app", device_Name(fatdevicemounted), content_name_no_Extension(system_version));
			original_content_backed_Up = Fat_CheckFile(filename);
			if(Debugger) logfile("original_content_backed up[%i].\n", original_content_backed_Up);
			if(!original_content_backed_Up) {
				success = downloadApp();
				if(success <= 0) {
					if(Debugger) logfile("unable to download .\n");
					return;
				}
			}
			saved_original_theme = fopen(filename, "rb");
			if(!saved_original_theme) return;
			InstallFile(saved_original_theme, true, (themeName != NULL ? true : false));
			exit_Program();
			filecnt = filelist_retrieve();
		}
		
		if (selected <= -1) {
			selected = filecnt - 1;
			list_start_line = 135;
		}
		if (selected >= filecnt) {
			selected = 0;
			list_start_line = 135;
		}
		//logfile("2selected[%i]  cnt[%i]\n", selected, cnt);
		// List scrolling 
		index = (selected - start);

		if (index >= FILES_PER_PAGE) {
			start += index - (FILES_PER_PAGE - 1);
			list_start_line = 135;
		}
		if (index <= -1) {
			start += index;
			list_start_line = 135;
		}
		//logfile("3selected[%i]  cnt[%i]\n\n", selected, cnt);
	}
	
	return;
}
int main() {
	bool Exit_App = false, settingsfile = false;
	u32 number_of_patches = 0, AHBPROT_Patched = 0;
	//s32 Ios = 0;
	
	__exception_setreload(5);
	
	//Ios = IOS_GetVersion();
	if(Debugger) {
		fatdevicemounted = Fat_Mount(SD);
		logfile("Debugging of MyMenuifyMod Started [%i]\n", fatdevicemounted);
	}
	if(AHBPROT_DISABLED) {
		AHBPROT_Patched = IOSPATCH_AHBPROT();
		if(!AHBPROT_Patched) {
			if(Debugger) logfile("Unable to patch AHBPROT %i \n", AHBPROT_Patched);
			
		}
		else {
			if(Debugger) logfile("Patched AHBPROT %i \n", AHBPROT_Patched);
		}
		number_of_patches = IOSPATCH_Apply();
	}
	else 
		number_of_patches = IOSPATCH_Apply();
	if(Debugger) logfile("number_of_patches [%i] \n", number_of_patches);
	
	Initialise();
	
	system_version = Get_system_version();
	if(Debugger) logfile("systemmenuversion = %i\n", system_version);
	if(system_version > 610) {
		if(Debugger) logfile("custom systemmenuversion = %i\n", system_version);
		
		draw_System_Info();
		sprintf(textbuf, "Custom System Version Detected: v%i", system_version);
		WriteCentre(220, textbuf);
		system_version = check_custom_system_version();
		sprintf(textbuf, "Real Version: v%i", system_version);
		WriteCentre(240, textbuf);
		DrawFrameFinish();
		sleep(2);
	} 
	
	settingsfile = read_Settings();
	if(!settingsfile) {
		Exit_App = Disclaimer();
		if(Exit_App) {
			exit_Program();
			exit(0);
		}
		else 
			settingsfile = write_Settings();
	}
	
	priiloader_found = checkforpriiloader();
	if(!priiloader_found) 
		nopriiloadermessage();
	
	draw_System_Info();
	WriteCentre(175, "MyMenuifyMod");
	WriteCentre(225, "   WELCOME  ");
	DrawFrameFinish();
	
	sleep(2);
	for(;;) {
		
		fatdevicemounted = theme_device_menu();
		
		theme_list_menu();
	}
	return 0;
}
