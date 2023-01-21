#ifndef THEMES_H
#define THEMES_H

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

typedef struct{
	char name[128];
	char region;
	u32 version;
	u32 size;
}themeStats;

typedef struct{
	char name[128];
	char region;
	u32 version;
	u32 size;
	bool iscsm;
	bool isapp;
}Fatfile;

void *allocate_memory(u32 size);
int currentthemeregion();
s32 __FileCmp(const void *a, const void *b);
s32 read_file(char *filepath, char **buffer, u32 *filesize);
s32 getdir(char *path, dirent_t **ent, u32 *cnt);

#endif// THEMES_H
