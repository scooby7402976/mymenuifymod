#ifndef globals_h
#define globals_h

#define DEVICES_MAX         3
#define null                0
#define MB_SIZE		        1048576.0
/* filelist constants */
#define FILES_PER_PAGE		10

/* console constants */
#define CONSOLE_XCOORD		40
#define CONSOLE_YCOORD		102
#define CONSOLE_WIDTH		550
#define CONSOLE_HEIGHT		350

/* Constants */
#define MAX_FILELIST_LEN	1024
#define MAX_FILEPATH_LEN	256
#define MAX_Uneek_filelist  25

#define BUTTON_UP		    2048
#define BUTTON_DOWN         1024
#define BUTTON_LEFT         256
#define BUTTON_RIGHT        512
#define BUTTON_B            4
#define BUTTON_A            8
#define BUTTON_HOME         128
#define BUTTON_1            2
#define BUTTON_2            1
#define BUTTON_PLUS         4096
#define BUTTON_MINUS        16

#define debug               1

#define MAX_THEMES                  500
#define KNOWN_SYSTEMMENU_VERSIONS   15

u32 known_Versions[KNOWN_SYSTEMMENU_VERSIONS] = {416, 417, 418, 448, 449, 450, 454, 480, 481, 482, 486, 512, 513, 514, 518};
char *regions[KNOWN_SYSTEMMENU_VERSIONS] = {"J", "U", "E", "J", "U", "E", "K", "J", "U", "E", "K", "J", "U", "E", "K"};
char *knownappfilenames[KNOWN_SYSTEMMENU_VERSIONS] = {"00000070.app", "00000072.app", "00000075.app", "00000078.app", "0000007b.app", "0000007e.app", "00000081.app", "00000084.app", "00000087.app", "0000008a.app", "0000008d.app", "00000094.app", "00000097.app", "0000009a.app", "0000009d.app"};

















#endif
// globals_h