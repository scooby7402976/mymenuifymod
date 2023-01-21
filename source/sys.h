#ifndef _SYS_H_
#define _SYS_H_

/* Prototypes */
void wiilight(int);
void sys_init(void);
void sys_loadmenu(void);
void sysHBC();
s32  sys_getcerts(signed_blob **, u32 *);
s32 __u8Cmp(const void *, const void *);
u8 *get_ioslist(u32 *);
#endif
