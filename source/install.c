#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>

#include "video.h"
#include "wpad.h"
#include "sys.h"

/* Constant */
#define BLOCK_SIZE	0x1000
#define CHUNKS 1000000

u32 filesize(FILE * file) {
	u32 curpos, endpos;
	
	if(file == NULL)
		return 0;
	curpos = ftell(file);
	fseek(file, 0, 2);
	endpos = ftell(file);
	fseek(file, curpos, 0);
	return endpos;
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
	printf("\t\t[+] Total parts: %d\n", numchunks);
	for(i = 0; i < numchunks; i++)
	{
		data = memalign(32, CHUNKS);
		if(data == NULL)
		{
			printf("\t[-] Error allocating memory !\n\n");
			
			printf("\tPress any button to continue .....\n");
			wpad_waitbuttons();
			return -1;
		}
		printf("\t\t   Installing part %d\n", (i + 1));
		ret = fread(data, 1, CHUNKS, fp);
		if (ret < 0) 
		{
			printf("\t[-] Error reading from SD ! (ret = %d)\n\n", ret);
			
			printf("\tPress any button to continue .....\n");
			wpad_waitbuttons();
			return -2;
		}
		else
		{
			cursize = ret;
		}
		wiilight(1);
		ret = ISFS_Write(nandfile, data, cursize);
		if(ret < 0)
		{
			printf("\t[-] Error writing to NAND ! (ret = %d)\n\n", ret);
			printf("\tPress any button to continue .....\n");
			wpad_waitbuttons();
			return ret;
		}
		free(data);
		wiilight(0);
	}
	ISFS_Close(nandfile);
	return 0;
}
