/**
* @file 		onvif_proto.h
* @author		Ken Chow <kenchow.cn@gmail.com>
* @date		    30 Dec 2014
* @version	    0.1
* @brief		This file implements for providing ONVIF protocol needs reply data.
* @details	    Such as: get_sys_info, set_sys_info, ... .
* @par Copyright (c): 
* 		        MIT license: http://opensource.org/licenses/MIT
* @par History:
*	            version: Ken Chow, 30 Dec 2014, Initial\n
*/

#ifndef ONVIF_PROTO_H
#define ONVIF_PROTO_H

#include "utilities.h"

#define REQ_STR_LEN		50

/*
typedef enum 
{
	// Device management - Capabilities
	GET_WSDL_URL = 1,

	// Device management - Network
	GET_HOST_NAME,
	SET_HOST_NAME,
	SET_HOST_NAME_FROM_DHCP,
	GET_DNS_SETTING,
	SET_DNS_SETTING,
	GET_NTP_SETTING,
	SET_NTP_SETTING,
	GET_DYNAMIC_DNS_SETTING,
	SET_DYNAMIC_DNS_SETTING,
	GET_NETWORK_INTERFACE_CONFIG,
	SET_NETWORK_INTERFACE_CONFIG,
	// ...

	// Device management - System
	DEVICE_INFO,
	GET_SYS_URIS,
	// ...

	ONVIF_REQ_TYPE_BUTT
}REQ_TYPE;
*/

typedef int (*REQ_PROC_F)(ONVIF_HDL*);

typedef struct PROC_MAP_s
{
	char req_str[REQ_STR_LEN];
	//REQ_TYPE req_type;
	REQ_PROC_F req_proc;
}PROC_MAP_T;

#ifdef __cplusplus
extern "C" {
#endif

// 对外接口
int getMaxProcMap();
const char *helloMsg();
const char *byeMsg();
const char *replyProbe();
int replyClient(const ONVIF_HDL *onvifHdl);

// 模块内函数
static int onvifFault(const ONVIF_HDL *onvifHdl, const char *val1, const char *val2);
static int getWsdlUrl(const ONVIF_HDL *onvifHdl);

#ifdef __cplusplus
}
#endif

#endif
