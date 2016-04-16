/**
* @file 		onvif_core.c
* @author		Ken Chow <kenchow.cn@gmail.com>
* @date		    30 Dec 2014
* @version	    0.1
* @brief		Onvif running mechanism.
* @details	    All the functions are provided for doing multi-client requesting. 
* @par Copyright (c): 
* 		        MIT license: http://opensource.org/licenses/MIT
* @par History:
*	            version: Ken Chow, 30 Dec 2014, Initial\n
*/

#include "onvif_core.h"

static char m_Running = 0; ///< 线程开关
static int m_MapNumber = 0;
static DISCOVERY_HDL m_DiscoveryHandle = {0, 1, FD_3702_BUTT, 4}; ///< 模块内使用变量，discovery相关的全局参数.
static ONVIF_HDL *m_onvifHandle = NULL; ///< 模块内使用变量，整个onvif的数据处理头地址.

/**
 * ONVIF的状态运行顺序：
 * 1. 进来一个请求之前，不存在任何关于这个请求的状态；
 * 2. 当请求被加入以后，变迁为 READY；
 * 3. 在解析XML的时候为 RUNNING，如果这是升级文件传输等连续性请求时，这个状态将被设置为 PENDING；
 * 4. 普通 RUNNING 回复以后，状态设置为 DEAD；
 * 5. 带有连续性 PENDING 被确认完成以后，状态设置为 DEAD;
 * 6. 对于 DEAD 状态的资源，需要释放回收，调用delInstance。
 */
int initONVIF()
{
	pthread_t hello, bye, probe, onvif;

	m_Running = 1;

	// discovery端口
	m_DiscoveryHandle.server_fd = createUDPServer(3702);
	if (0 > m_DiscoveryHandle.server_fd)
	{
		printf("[%s][%s][%d]Discovery port creates failed.\n", __FILE__, FUNCTIONS__, __LINE__);
		goto ERR;
	}

	// HELLO 线程 - ODTT[discovery enable and disable]
	if (0 != pthread_create(&hello, NULL, sendHello, NULL))
	{
		goto ERR;
	}

	// BYE
	if (0 != pthread_create(&bye, NULL, sendBye, NULL))
	{
		goto ERR;
	}

	// probe
	if (0 != pthread_create(&probe, NULL, startProbe, NULL))
	{
		goto ERR;
	}

	// ONVIF response
	if (0 != pthread_create(&onvif, NULL, runONVIF, NULL))
	{
		goto ERR;
	}

	if (0 >= (m_MapNumber=getMaxProcMap()))
	{
		goto ERR;
	}

	return 0;

ERR:
	releaseONVIF();
	return -1;
}

/**
 * 释放整个ONVIF状态机运行的资源
 */
int releaseONVIF()
{
	ONVIF_HDL *tmp = m_onvifHandle;

	// send bye
	m_DiscoveryHandle.fd_sync = FD_3702_BYE;
	// Delay 2s for BYE thread sending message.
	sleep(2);
	// close 3702
	if (0 > m_DiscoveryHandle.server_fd)
		close(m_DiscoveryHandle.server_fd);

	// ONVIF_HDL
	while(m_onvifHandle)
	{
		m_onvifHandle = m_onvifHandle->next;

		if (tmp->recv_buf)
			free(tmp->recv_buf);

		if (tmp->send_buf)
			free(tmp->send_buf);

		if (0 > tmp->client_fd)
			close(tmp->client_fd);

		free(tmp);
		tmp = m_onvifHandle;
	}

	m_onvifHandle = NULL;
	m_Running = 0;

	return 0;
}

/**
 * 新加一个用户请求
 */
int addInstance(const int fd, const char *buff, const int len, const struct sockaddr_in *peer)
{
	int xml_len = -1;
	ONVIF_HDL *new = NULL, *tmp = NULL;

	if ((0 > fd) || (NULL == buff) || (0 > len) || (NULL == peer))
	{
		return -1;
	}

	// validation xml's length
	if (0 != verifyContentLen(buff, &xml_len));
	{
		printf("This is not an ONVIF request.\n");
		return -1;
	}
	
	new = (ONVIF_HDL *)malloc(sizeof(ONVIF_HDL));
	if (!new)
	{
		perror("malloc:");
		return -1;
	}

	new->client_fd = fd;
	memcpy(&new->peer, peer, sizeof(struct sockaddr_in));
	new->recv_buf = strndup(buff, len);
	new->recv_len = len;
	//new->end_of_http = ;
	new->state = ONVIF_STATE_READY;
	new->max_maps = m_MapNumber;

	if (NULL != m_onvifHandle)
	{
		tmp = m_onvifHandle;
		while(tmp->next)
			tmp = tmp->next;
		tmp->next = new;
	}
	else
	{
		m_onvifHandle = new;
	}

	return 0;
}

/**
 * 当前请求统计
 */
int showInstance(ONVIF_HDL *onvifHdl)
{
	return 0;
}

/**
 * 删除释放一个已经完成的请求，DEAD状态的请求，也需要被删除
 */
static int delInstance(ONVIF_HDL *onvifHdl)
{
	ONVIF_HDL *tmp = m_onvifHandle;

	while(tmp)
	{
		if (tmp == onvifHdl)
		{
			tmp = onvifHdl->next;

			if (tmp->recv_buf)
				free(tmp->recv_buf);

			if (tmp->send_buf)
				free(tmp->send_buf);

			if (0 > tmp->client_fd)
				close(tmp->client_fd);

			free(tmp);
			tmp = NULL;

			return 0;
		}
		tmp = tmp->next;
	}

	return -1;
}

/**
 * 开始运行ONVIF，采用FIFO的方式，支持多客户端连接的处理线程
 */
static void *runONVIF()
{
	// 当前被处理的请求
	ONVIF_HDL *curOnvifHdl = m_onvifHandle;

	while(m_Running)
	{
		// 没有请求需要处理的时候，释放CPU资源
		if ((NULL == curOnvifHdl) && (NULL == m_onvifHandle))
		{
			usleep(1000);
			continue;
		}
		else if (NULL == curOnvifHdl)// 当有多个客户端操作时，FIFO的处理机制，一轮请求处理完毕以后，开始下一轮处理
		{
			curOnvifHdl = m_onvifHandle;
		}

		// 回复当前这个客户端的请求，根据ONVIF的请求类型，设定是否可以被释放
		curOnvifHdl->state = ONVIF_STATE_RUNNING;
		replyClient(curOnvifHdl);

		// 带有升级和文件传输的客户端，类似有连续性的操作时，请求需要被保持
		if (curOnvifHdl->state == ONVIF_STATE_FINISH)
		{
			//删除当前完成的请求资源，处理下一个请求
			delInstance(curOnvifHdl);
		}
	}

	pthread_exit();

	return;
}

static void *sendHello()
{
	int i = 0;
	char *msg = NULL;

	while(m_Running)
	{
		if ((FD_3702_HELLO == m_DiscoveryHandle.fd_sync) && (0 < m_DiscoveryHandle.server_fd))
		{
			if ((i++) > m_DiscoveryHandle.broadcast_nums)
			{
				i = 0;
				m_DiscoveryHandle.fd_sync = FD_3702_PROBE;
			}

			if (NULL == (msg=helloMsg()))
			{
				printf("Hello message wrong!\n");
				sleep(3);
				continue;
			}

			if (0 > send(m_DiscoveryHandle.server_fd, msg, strlen(msg), 0))
			{
				perror("HELLO send:");
				sleep(3);
				continue;
			}

			usleep(1000*500); // 500ms
		}

		usleep(1000*500); // 500ms
	}

	pthread_exit();

	return;
}

static void *sendBye()
{
	char *msg = NULL;

	while(m_Running)
	{
		if ((FD_3702_BYE == m_DiscoveryHandle.fd_sync) && (0 < m_DiscoveryHandle.server_fd))
		{
			if (NULL == (msg=byeMsg()))
			{
				printf("Bye message wrong!\n");
				sleep(3);
				continue;
			}

			if (0 > send(m_DiscoveryHandle.server_fd, msg, strlen(msg), 0))
			{
				perror("BYE send:");
				sleep(3);
				continue;
			}
		}

		usleep(1000*500); // 500ms
	}

	pthread_exit();

	return;
}

static void *startProbe()
{
	char *send_msg = NULL, recv_msg[1024] = {0};

	/* RECV中断超时1s，以防hello或者bye需要使用3702 */

	while(m_Running)
	{
		if ((m_DiscoveryHandle.is_discovery) && (FD_3702_PROBE == m_DiscoveryHandle.fd_sync) && (0 < m_DiscoveryHandle.server_fd))
		{
			if (NULL == (send_msg=replyProbe()))
			{
				printf("Bye message wrong!\n");
				sleep(3);
				continue;
			}

			// 非阻塞读，超时时，需要给其他使用3702端口的socket使用；
			if(1 == selectSocket(m_DiscoveryHandle.server_fd, 0, 1000*500)) // 500ms
			{
				recv(m_DiscoveryHandle.server_fd, recv_msg, sizeof(recv_msg), 0);
			}

			// 是否为合法的discovery请求
			// TODO

			if (0 > send(m_DiscoveryHandle.server_fd, send_msg, strlen(send_msg), 0))
			{
				perror("PROBE send:");
				sleep(3);
				continue;
			}

			usleep(1000*500); // 500ms
		}

		usleep(1000*500); // 500ms
	}

	pthread_exit();

	return;
}
