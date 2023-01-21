#ifndef _HTTP_H_
#define _HTTP_H_

#include <gctypes.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
	HTTPR_OK,
	HTTPR_ERR_CONNECT,
	HTTPR_ERR_REQUEST,
	HTTPR_ERR_STATUS,
	HTTPR_ERR_TOOBIG,
	HTTPR_ERR_RECEIVE
} http_res;

s32 tcp_socket (void);
s32 tcp_connect (char *host, const u16 port);
char * tcp_readln (const s32 s, const u16 max_length, const s64 start_time, const u16 timeout);
bool tcp_read (const s32 s, u8 **buffer, const u32 length, bool print);
bool tcp_write (const s32 s, const u8 *buffer, const u32 length);
bool http_request(const char *url, const u32 max_size, bool print);
bool http_get_result(u32 *http_status, u8 **content, u32 *length);
#ifdef __cplusplus
}
#endif
#endif

