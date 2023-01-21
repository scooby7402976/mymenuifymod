#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <fat.h>
#include <sys/dir.h>
#include <ogc/isfs.h>
#include <malloc.h>
#include <unistd.h>
#include <network.h>
#include <ogc/lwp_watchdog.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <fcntl.h>

#include "themes.h"
#include "manager.h"


extern u32 systemmenuVersion;

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
		logfile("Error: could not get dir entry count! (result: %d)\n", res);
		return -1;
	}

	char ebuf[ISFS_MAXPATH + 1];

	char *nbuf = (char *)allocate_memory((ISFS_MAXPATH + 1) * num);
	if(nbuf == NULL){
		logfile("ERROR: could not allocate buffer for name list!\n");
		return -2;
	}

	res = ISFS_ReadDir(path, nbuf, &num);
	DCFlushRange(nbuf,13*num); //quick fix for cache problems?
	if(res != ISFS_OK){
		logfile("ERROR: could not get name list! (result: %d)\n", res);
		free(nbuf);
		return -3;
	}
	
	*cnt = num;
	
	*ent = allocate_memory(sizeof(dirent_t) * num);
	if(*ent==NULL){
		logfile("Error: could not allocate buffer\n");
		free(nbuf);
		return -4;
	}

	for(i = 0, k = 0; i < num; i++){	    
		for(j = 0; nbuf[k] != 0; j++, k++)
			ebuf[j] = nbuf[k];
		ebuf[j] = 0;
		k++;

		strcpy((*ent)[i].name, ebuf);
		//logfile("Name of file (%s)\n",(*ent)[i].name);
	}
	
	qsort(*ent, *cnt, sizeof(dirent_t), __FileCmp);
	
	free(nbuf);
	return 0;
}
s32 read_file(char *filepath, char **buffer, u32 *filesize){
	s32 Fd;
	int ret;
	
	
	if(buffer == NULL){
		logfile("NULL Pointer\n");
		return -1;
	}

	Fd = ISFS_Open(filepath, ISFS_OPEN_READ);
	if (Fd < 0){
		logfile("\n   * ISFS_Open %s failed %d", filepath, Fd);
		//Pad_WaitButtons();
		return Fd;
	}

	fstats *status;
	status = allocate_memory(sizeof(fstats));
	if (status == NULL){
		logfile("Out of memory for status\n");
		return -1;
	}
	
	ret = ISFS_GetFileStats(Fd, status);
	if (ret < 0){
		logfile("ISFS_GetFileStats failed %d\n", ret);
		ISFS_Close(Fd);
		free(status);
		return -1;
	}
	
	*buffer = allocate_memory(status->file_length);
	if (*buffer == NULL){
		logfile("Out of memory for buffer\n");
		ISFS_Close(Fd);
		free(status);
		return -1;
	}
		
	ret = ISFS_Read(Fd, *buffer, status->file_length);
	if (ret < 0){
		printf("ISFS_Read failed %d\n", ret);
		ISFS_Close(Fd);
		free(status);
		free(*buffer);
		return ret;
	}
	ISFS_Close(Fd);

	*filesize = status->file_length;
	free(status);

	return 0;
}
int currentthemeregion(){
	int rtn = 0;
	
	switch(systemmenuVersion)
	{
		case 416: rtn = 74;
		break;
		case 417: rtn = 85;
		break;
		case 418: rtn = 69;
		break;
		case 448: rtn = 74;
		break;
		case 449: rtn = 85;
		break;
		case 450: rtn = 69;
		break;
		case 454: rtn = 75;
		break;
		case 480: rtn = 74;
		break;
		case 481: rtn = 85;
		break;
		case 482: rtn = 69;
		break;
		case 486: rtn = 75;
		break;
		case 512: rtn = 74;
		break;
		case 513: rtn = 85;
		break;
		case 514: rtn = 69;
		break;
		case 518: rtn = 75;
		break;
	}
	return rtn;
}
