/**
* @file 		onvif_proto.h
* @author		Ken Chow <kenchow.cn@gmail.com>
* @date		    30 Dec 2014
* @version	    0.1
* @brief		This file implements for providing ONVIF protocol needs reply interface.
* @details	    Such as: get_sys_info, set_sys_info, ... .
* @par Copyright (c): 
* 		        MIT license: http://opensource.org/licenses/MIT
* @par History:
*	            version: Ken Chow, 30 Dec 2014, Initial\n
*/

#include "onvif_proto.h"

static PROC_MAP_T m_procMap[] = 
{
	{"GetWsdlUrlRequest", getWsdlUrl}
};

int getMaxProcMap()
{
	return (sizeof(m_procMap)/sizeof(PROC_MAP_T));
}

const char *helloMsg()
{
	return "";
}

const char *byeMsg()
{
	return "";
}

const char *replyProbe()
{
	return "";
}

/**
 * 回复客户端函数实现
 * ... 
 */
int replyClient(const ONVIF_HDL *onvifHdl)
{
	int i = 0;
	char req_root[REQ_STR_LEN] = {0};

	if (NULL == onvifHdl)
		return -1;
	
	if (0 != getRootNode(onvifHdl->recv_buf+onvifHdl->end_of_http, req_root))
	{
		// It has been verfied before add instance, I assume XML format was right.
		//onvifHdl->state = ONVIF_STATE_FINISH;
		printf("Wrong request type\n");
		return -1;
	}
	
	for (i=0; i<onvifHdl->max_maps; i++)
	{
		// 根据获取的rootNode来决定消息的类型，并执行对应的处理函数
		if (!strncmp(m_procMap[i].req_str, req_root, strlen(m_procMap[i].req_str)))
			break;
	}
	
	if (i == onvifHdl->max_maps)
	{
		// reply fault
		onvifFault(onvifHdl, req_root, "NotSupport");
		return -1;
	}
	
	// 调用在m_procMap里面填充的处理
	m_procMap[i].req_proc(onvifHdl);

	// replying client's time out is 2s, in case which block ONVIF Running thread.
	if (0 == selectSocket(onvifHdl->fd, 2, 0)) // 2s
		return send(onvifHdl->fd, onvifHdl->send_buf, onvifHdl->send_len, 0);

	return -1;
}

static int onvifFault(const ONVIF_HDL *onvifHdl, const char *val1, const char *val2)
{
	// 与预期不符合的请求，可以被释放
	onvifHdl->state = ONVIF_STATE_FINISH;

	//send();
	return 0;
}

static int getWsdlUrl(const ONVIF_HDL *onvifHdl)
{
	return 0;
}

