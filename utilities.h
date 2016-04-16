/**
* @file 		utilities.h
* @author		Ken Chow <kenchow.cn@gmail.com>
* @date		    6 Jan 2015
* @version	    0.1
* @brief		Offer convenient and useful functions for ONVIF.
* @details	    
* @par Copyright (c): 
* 		        MIT license: http://opensource.org/licenses/MIT
* @par History:
*	            version: Ken Chow, 6 Jan 2015, Initial\n
*/

#ifndef UTILITIES_H
#define UTILITIES_H

#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <dirent.h>
#include <syslog.h>
#include <time.h>

#define LINK_LOCAL_ADDR  "192.168.1.200"
#define ONVIF_WS_DISCOVERY_PORT  3702
#define ONVIF_RECV_REQUEST_PORT  80
#define ONVIF_TCP_MAXCONN 100


#ifdef __cplusplus
extern "C" {
#endif

int isValidIp4 (char *str); 
int isValidHostname (char *str); 
int createUDPServer(int port);
int createTCPServer(int port);
long periodtol(char *ptr);
long periodtime(char *ptr);
int cutString(char *pcSrc, const char *start, const char *end, char *pcDest);
int separateUri(char *uri, char *ip, char *port, char *path);
int getMAC(char *ifname, char *mac);
int setIP(char *ifname, char *addr, int netmask);
int getIP(char *ifname, char *ip);
int selectSocket(int socket, time_t sec, suseconds_t usec)

#ifdef __cplusplus
}
#endif

#endif
