/*  http -- http convenience functions

    Copyright (C) 2008 bushing

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 2.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ogcsys.h>
#include <network.h>
#include <ogc/lwp_watchdog.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <fcntl.h>

#include "http.h"
#include "manager.h"
#include "video.h"

#define TCP_CONNECT_TIMEOUT 10000
#define TCP_BLOCK_SIZE (16 * 1024)
#define TCP_BLOCK_RECV_TIMEOUT 10000
#define TCP_BLOCK_SEND_TIMEOUT 10000
#define HTTP_TIMEOUT 65535

char *http_host;
u16 http_port ;
char *http_path;
u32 http_max_size;
http_res result;
u32 http_status;
u32 content_length;
u8 *http_data;

s32 tcp_socket (void) {
	s32 s, res;

	s = net_socket (PF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		printf("net_socket failed: %d\n", s);
		return s;
	}
	res = net_fcntl (s, F_GETFL, 0);
	if (res < 0) {
		printf("F_GETFL failed: %d\n", res);
		net_close (s);
		return res;
	}
	res = net_fcntl (s, F_SETFL, res | 4);
	if (res < 0) {
		printf("F_SETFL failed: %d\n", res);
		net_close (s);
		return res;
	}
	return s;
}
s32 tcp_connect (char *host, const u16 port) {
	struct hostent *hp;
	struct sockaddr_in sa;
	s32 s, res;
	s64 t;

	hp = net_gethostbyname (host);
	if (!hp || !(hp->h_addrtype == PF_INET)) {
		printf("net_gethostbyname failed: %d\n", errno);
		return errno;
	}
	s = tcp_socket ();
	if (s < 0)
		return s;
	memset (&sa, 0, sizeof (struct sockaddr_in));
	sa.sin_family= PF_INET;
	sa.sin_len = sizeof (struct sockaddr_in);
	sa.sin_port= htons (port);
	memcpy ((char *) &sa.sin_addr, hp->h_addr_list[0], hp->h_length);
	t = gettime ();
	while (true) {
		if (ticks_to_millisecs (diff_ticks (t, gettime ())) >
				TCP_CONNECT_TIMEOUT) {
			printf("tcp_connect timeout\n");
			net_close (s);
			return -ETIMEDOUT;
		}
		res = net_connect (s, (struct sockaddr *) &sa, sizeof (struct sockaddr_in));
		if (res < 0) {
			if (res == -EISCONN)
				break;
			if (res == -EINPROGRESS || res == -EALREADY) {
				usleep (20 * 1000);
				continue;
			}
			printf("net_connect failed: %d\n", res);
			net_close (s);
			return res;
		}
		break;
	}
	return s;
}
char * tcp_readln (const s32 s, const u16 max_length, const s64 start_time, const u16 timeout) {
	char *buf;
	u16 c;
	s32 res;
	char *ret;
	buf = (char *) malloc (max_length);
	c = 0;
	ret = NULL;
	
	while (true) {
		if (ticks_to_millisecs (diff_ticks (start_time, gettime ())) > timeout)
			break;
		res = net_read (s, &buf[c], 1);
		if ((res == 0) || (res == -EAGAIN)) {
			usleep (20 * 1000);
			continue;
		}
		if (res < 0) {
			printf("tcp_readln failed: %d\n", res);
			break;
		}
		if ((c > 0) && (buf[c - 1] == '\r') && (buf[c] == '\n')) {
			if (c == 1) {
				ret = strdup ("");
				break;
			}
			ret = strndup (buf, c - 1);
			break;
		}
		c++;
		if (c == max_length)
			break;
	}
	free (buf);
	return ret;
}
bool tcp_read (const s32 s, u8 **buffer, const u32 length, bool print) {
	u8 *p;
	u32 step, left, block, received;
	s64 t;
	s32 res;
	if(print) {
	printf("\n");
	fflush(stdout);
	}
	step = 0;
	p = *buffer;
	left = length;
	received = 0;

	t = gettime ();
	while (left) {
		if(print) {
		con_clearline();
		printf("Downloaded %u / %u bytes .", received, length);
		fflush(stdout);
		}
		if (ticks_to_millisecs (diff_ticks (t, gettime ())) >
				TCP_BLOCK_RECV_TIMEOUT) {
			//logfile("tcp_read timeout\n");
			printf("tcp_read timeout\n");
			break;
		}
		block = left;
		if (block > 2048)
			block = 2048;
		//logfile("going net read ...");
		res = net_read (s, p, block);
		//logfile("res(%d) net read \n",res);
		if ((res == 0) || (res == -EAGAIN)) {
			usleep (20 * 1000);
		//	logfile("res was %d \n",res);
			continue;
		}
		if (res < 0) {
			printf("net_read failed: %d\n", res);
		//	logfile("res(%d) \n",res);
			break;
		}
		received += res;
		left -= res;
		p += res;
		if ((received / TCP_BLOCK_SIZE) > step) {
			t = gettime ();
			step++;
		}
		//logfile("step (%d) received (%d) left(%d)\n", step, received,left);
	}
	if(print) {
		if(left == 0) {
			con_clearline();
			printf("Downloaded %u / %u bytes .", received, length);
			fflush(stdout);
		}
	}
	return left == 0;
}
bool tcp_write (const s32 s, const u8 *buffer, const u32 length) {
	const u8 *p;
	u32 step, left, block, sent;
	s64 t;
	s32 res;
	step = 0;
	p = buffer;
	left = length;
	sent = 0;

	t = gettime ();
	while (left) {
		if (ticks_to_millisecs (diff_ticks (t, gettime ())) >
				TCP_BLOCK_SEND_TIMEOUT) {
			printf("tcp_write timeout\n");
			break;
		}
		block = left;
		if (block > 2048)
			block = 2048;
		res = net_write (s, p, block);
		if ((res == 0) || (res == -56)) {
			usleep (20 * 1000);
			continue;
		}
		if (res < 0) {
			printf("net_write failed: %d\n", res);
			break;
		}
		sent += res;
		left -= res;
		p += res;
		if ((sent / TCP_BLOCK_SIZE) > step) {
			t = gettime ();
			step++;
		}
	}
	return left == 0;
}
bool http_split_url (char **host, char **path, const char *url) {
	const char *p;
	char *c;
	if (strncasecmp (url, "http://", 7))
		return false;
	//logfile("after strncasecmp \n");
	p = url + 7 ;
	//logfile("p (%s) \n",p);
	c = strchr (p, '/');
	//logfile("c(%s)\n",c);
	if (c[0] == 0)
		return false;
	//logfile("after if c \n");
	*host = strndup (p, c - p);
	//logfile("host (%s) \n",*host);
	*path = strdup (c);
	//logfile("path (%s) \n",*path);
	return true;
}
bool http_request (const char *url, const u32 max_size, bool print) {
	int linecount;
	if (!http_split_url(&http_host, &http_path, url)) return false;
	http_port = 80;
	http_max_size = max_size;
	http_status = 404;
	content_length = 0;
	http_data = NULL;
	int s = tcp_connect (http_host, http_port);
	//logfile("tcp_connect(%s, %hu) = %d\n", http_host, http_port, s);
	if (s < 0) {
		result = HTTPR_ERR_CONNECT;
		return false;
	}
	char *request = (char *) malloc (1024);
	char *r = request;
	r += sprintf (r, "GET %s HTTP/1.1\r\n", http_path);
	r += sprintf (r, "Host: %s\r\n", http_host);
	r += sprintf (r, "Cache-Control: no-cache\r\n\r\n");
	//logfile("request = %s\n", request);
	bool b = tcp_write (s, (u8 *) request, strlen (request));
	//logfile("tcp_write returned %d\n", b);
	free (request);
	linecount = 0;
	for (linecount=0; linecount < 32; linecount++) {
		char *line = tcp_readln (s, 0xff, gettime(), HTTP_TIMEOUT);
	//	logfile("tcp_readln returned %p (%s)\n", line, line?line:"(null)");
		if (!line) {
			http_status = 404;
			result = HTTPR_ERR_REQUEST;
			break;
		}
		if (strlen (line) < 1) {
			free (line);
			line = NULL;
			break;
		}
		sscanf (line, "HTTP/1.1 %u", &http_status);
		sscanf (line, "Content-Length: %u", &content_length);
		free (line);
		line = NULL;
	}
	//logfile("content_length = %d, status = %d, linecount = %d\n", content_length, http_status, linecount);
	if (linecount == 32 || !content_length) http_status = 404;
	if (http_status != 200) {
		result = HTTPR_ERR_STATUS;
		net_close (s);
		return false;
	}
	if (content_length > http_max_size) {
	//	logfile("here is prob \n");
	//	logfile("clen(%d) httpmsize(%d) \n",content_length,http_max_size);
		result = HTTPR_ERR_TOOBIG;
		net_close (s);
		return false;
	}
	//logfile("going to read tcp \n");
	http_data = (u8 *) malloc (content_length);
	b = tcp_read (s, &http_data, content_length, print);
	if (!b) {
		free (http_data);
		http_data = NULL;
		result = HTTPR_ERR_RECEIVE;
		net_close (s);
		return false;
	}
	//logfile("leaving request \n");
	result = HTTPR_OK;
	net_close (s);
	return true;
}
bool http_get_result (u32 *_http_status, u8 **content, u32 *length) {
	if (http_status) *_http_status = http_status;
	//logfile("httpstatus(%d) \n",http_status);
	if (result == HTTPR_OK) {
		*content = http_data;
		*length = content_length;
	} else {
		*content = NULL;
		*length = 0;
	}
	//logfile("content(%s) length(%d) http path(%s) \n",*content,*length,http_path);
	free (http_host);
	free (http_path);
	return true;
}
