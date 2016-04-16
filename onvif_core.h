/**
* @file 		onvif_core.h
* @author		Ken Chow <kenchow.cn@gmail.com>
* @date		    30 Dec 2014
* @version	    0.1
* @brief		Onvif data structure defination.
* @details	    This head file included onvif data handle, processing state.
* @par Copyright (c): 
* 		        MIT license: http://opensource.org/licenses/MIT
* @par History:
*	            version: Ken Chow, 30 Dec 2014, Initial\n
*/
#ifndef ONVIF_CORE_H
#define ONVIF_CORE_H

#include "utilities.h"
#include "http_parse.h"
#include "xml_parse.h"
#include "onvif_proto.h"

typedef enum 
{
	ONVIF_STATE_READY = 66,
	ONVIF_STATE_RUNNING,
	ONVIF_STATE_PENDING,
	ONVIF_STATE_FINISH,
	ONVIF_STATE_BUTT
}ONVIF_STATE;

typedef enum 
{
	FD_3702_HELLO = 11,
	FD_3702_BYE,
	FD_3702_PROBE,
	FD_3702_BUTT
}FD_3702_STATE;

typedef struct DISCOVERY_HDL_s
{
	int server_fd; // 3702's port file description.
	char is_discovery; // Is discovery enable(value:1) or disable(value:0)?
	// PROBE, HELLO and BYE are using the same port at the different situation.
	char fd_sync; //see above:: FD_3702_STATE
	char broadcast_nums; // Sending Hello's or Bye's number of times.
}DISCOVERY_HDL;

typedef int (*PARSE_HTTP)(void*);
typedef int (*PARSE_XML)(void*);

typedef struct ONVIF_HDL_s
{
	// client fd.
	int client_fd;

	// Remote information.
	struct sockaddr_in peer;

	// Each request from client, it will be covered in the next receiving.
	char *recv_buf;
	int recv_len;

	// ONVIF packet consist of HTTP head and XML, this is a marker for separating.
	int end_of_http;

	// It will be covered in the next sending.
	char *send_buf;
	int send_len;

	// Onvif client's requesting data type, such as: get_sys_time, get_ip_addr, set_dns, set_sys_name 
	//int req_onvif_type; // see onvif_proto.h:: ONVIF_REQ_TYPE

	// Monitor the ONVIF running state, make sure all things have a good result. 
	char state; // see above:: ONVIF_STATE
	
	// The current maximum map of response process
	int max_maps;

	struct ONVIF_HDL_s *next;
}ONVIF_HDL;

#ifdef __cplusplus
extern "C" {
#endif

// 对外接口
int addInstance(int fd, char *buff, int len);
int showInstance(ONVIF_H *onvifHdl);
int initONVIF();
int releaseONVIF();

// 模块内函数
static int delInstance(ONVIF_HDL *onvifHdl);
static void *runONVIF();
static void *sendHello();
static void *sendBye();
static void *startProbe();

#ifdef __cplusplus
}
#endif

#endif

