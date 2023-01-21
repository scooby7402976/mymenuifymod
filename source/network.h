#ifndef _NETWORK_H_
#define _NETWORK_H_

#ifdef __cplusplus
extern "C" {
#endif
#define NusHostname "nus.cdn.shop.wii.com"
#define NusPath "/ccs/download/"
#define NusPort 80
/* Prototypes */
char *Network_GetIP(void);
s32 Network_Init(void);
s32 Network_Connect(void);
s32 Network_Request(const char *, u32 *);
s32 Network_Read(void *, u32);
s32 Network_Write(void *, u32);
void NetworkInit();
void NetworkShutdown();
#ifdef __cplusplus
}
#endif
#endif
