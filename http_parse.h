/**
* @file 		http_parse.h
* @author		Ken Chow <kenchow.cn@gmail.com>
* @date		    30 Dec 2014
* @version	    0.1
* @brief		HTTP head pick up.
* @details	    Only for HTTP head.
* @par Copyright (c): 
* 		        MIT license: http://opensource.org/licenses/MIT
* @par History:
*	            version: Ken Chow, 30 Dec 2014, Initial\n
*/

#ifndef ONVIF_PROTO_H
#define ONVIF_PROTO_H

#include "utilities.h"
#include "onvif_core.h"

/*
typedef struct http_head_s
{

}HTTP_HEAD;
*/

#ifdef __cplusplus
extern "C" {
#endif

// 外部接口
int verifyContentLen(const char *http, int *len);

#ifdef __cplusplus
}
#endif

#endif