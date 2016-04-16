/**
* @file 		xml_parse.h
* @author		Ken Chow <kenchow.cn@gmail.com>
* @date		    30 Dec 2014
* @version	    0.1
* @brief		XML format to ONVIF data structure.
* @details	    XML parser use external library - libxml2<http://xmlsoft.org>.
* @par Copyright (c): 
* 		        MIT license: http://opensource.org/licenses/MIT
* @par History:
*	            version: Ken Chow, 30 Dec 2014, Initial\n
*/

#ifndef XML_PARSE_H
#define XML_PARSE_H

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "utilities.h"
#include "onvif_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// 对外接口
int getRootNode(char *xml, char *node);

#ifdef __cplusplus
}
#endif

#endif