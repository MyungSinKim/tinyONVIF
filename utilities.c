/**
* @file 		utilities.c
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

#include "utilities.h"

/* @brief Check if IP is valid */
int isValidIp4 (char *str) 
{
	int segs = 0;   /* Segment count. */     
	int chcnt = 0;  /* Character count within segment. */     
	int accum = 0;  /* Accumulator for segment. */      
	/* Catch NULL pointer. */      
	if (str == NULL) return 0;      
	/* Process every character in string. */      
	while (*str != '\0') 
	{         
		/* Segment changeover. */          
		if (*str == '.') 
		{             
			/* Must have some digits in segment. */              
			if (chcnt == 0) return 0;              
			/* Limit number of segments. */              
			if (++segs == 4) return 0;              
			/* Reset segment values and restart loop. */              
			chcnt = accum = 0;             
			str++;             
			continue;         
		}  

		/* Check numeric. */          
		if ((*str < '0') || (*str > '9')) return 0;
		/* Accumulate and check segment. */          
		if ((accum = accum * 10 + *str - '0') > 255) return 0;
		/* Advance other segment specific stuff and continue loop. */          
		chcnt++;         
		str++;     
	}      
	/* Check enough segments and enough characters in last segment. */      
	if (segs != 3) return 0;      
	if (chcnt == 0) return 0;      
	/* Address okay. */      
	return 1; 
} 

/* @brief Check if a hostname is valid */
int isValidHostname (char *str) 
{
	/* Catch NULL pointer. */      
	if (str == NULL) 
	{
		return 0;      
	}
	/* Process every character in string. */      
	while (*str != '\0') 
	{         
		if ((*str >= 'a' && *str <= 'z') || (*str >= 'A' && *str <= 'Z') || (*str >= '0' && *str <= '9') || (*str == '.') || (*str == '-') )
		{
			str++;
		}
		else
		{
			return 0;
		}
	}
	return 1; 
}

static int setNonblock(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL);
	if (flags == -1)
		return -1;

	flags |= O_NONBLOCK;
	flags = fcntl(fd, F_SETFL, flags);
	return flags;
}

static int bindServer(int server_fd, char *server_url, int server_port)
{
	struct sockaddr_in server_sockaddr;
    
	memset(&server_sockaddr, 0, sizeof server_sockaddr);
    
	server_sockaddr.sin_family = AF_INET;
    
	if (server_url != NULL) 
	{
		inet_aton(server_url, &server_sockaddr.sin_addr);
    } 
    else 
    {
        server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    
    server_sockaddr.sin_port = htons(server_port);

    return bind(server_fd, (struct sockaddr *) &server_sockaddr, sizeof(server_sockaddr));
}

int createUDPServer(int port)
{
    int server_udp;

    if (1 > port)
    {
    	return -1;
    }

    server_udp = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (-1 == server_udp) 
    {
        perror("socket:");
        return -1;
    }

    /* server socket is nonblocking */
    if (-1 == set_nonblock_fd(server_udp)) 
    {
        perror("set_nonblock_fd:");
        close(server_udp);
        return -1;
    }

    /* close server socket on exec so cgi's can't write to it */
    if (-1 == fcntl(server_udp, F_SETFD, 1)) 
    {
        perror("fcntl:");
        close(server_udp);
        return -1;
    }

    /* internet family-specific code encapsulated in bindServer()  */
    if (-1 == bindServer(server_udp, NULL, port)) 
    {
        perror("bind:");
        close(server_udp);
        return -1;
    }

    return server_udp;
}

/*
 * For Receive Clients Request
 */
int createTCPServer(int port)
{
    int server_s;
	int sock_opt = 1;
	char localIP[IP_LENGTH] = {0};

    server_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == server_s) 
    {
		perror("socket:");
		return -1;
    }

    /* server socket is nonblocking */
    if (-1 == set_nonblock_fd(server_s))
    {
    	close(server_s);
    	perror("set_nonblock_fd:");
		return -1;
    }

    /* close server socket on exec so cgi's can't write to it */
    if (-1 == fcntl(server_s, F_SETFD, 1)) 
    {
    	close(server_s);
    	perror("fcntl:");
		return -1;
    }

    /* reuse socket addr */
    if (-1 == setsockopt(server_s, SOL_SOCKET, SO_REUSEADDR, (void *)&sock_opt, sizeof(sock_opt)))
    {
    	close(server_s);
		perror("setsockopt:");
		return -1;
    }

    /* internet family-specific code encapsulated in bind_server()  */

    if (-1 == getIP(ETH_NAME, localIP))
    {
    	close(server_s);
    	printf("[%s][%d] ERROR: get IP addr. \n", __FILE__, __LINE__);
    	return -1;
    }
    
    if (-1 == bindServer(server_s, NULL, port)) 
    {
    	close(server_s);
    	perror("bind:");
		return -1;
    }

    /* listen: large number just in case your kernel is nicely tweaked */
    if (listen(server_s, ONVIF_TCP_MAXCONN) == -1) 
    {
    	close(server_s);
		perror("listen:");
		return -1;
    }

    return server_s;
}

int connectTCP(int sock, char *ip, int port)
{
	struct sockaddr_in addr;

	if (ip == NULL || 0 > port)
	{
		return -1;
	}
	
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	inet_pton(AF_INET, ip, &addr.sin_addr);

	return connect(sock, (struct sockaddr*)&addr, sizeof(addr));
}

int s2hex(char *str, char *hex)
{
	int i;
	char tmp[3] = {0};

	if (NULL == str || NULL == hex)
	{
		return -1;
	}

	while(*str)
	{
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "%x", *str);
		strcat(hex, tmp);
		str++;
	}

	/* To upper */
	for (i=0; i<strlen(hex); i++)
	{
		hex[i] = toupper(hex[i]);
	}

	return 0;
}

long periodTime(char *ptr)
{
	int i = 0;
	int nums = 0, secs = 0, mins = 0, hours = 0;
	long time = -1;
	char str_time[10] = {0};
	char *tmp = ptr;

	/* If the string isn't starting from PT, Don't deal with it. */
	if (NULL == strstr(tmp, "PT"))
	{
		return -1;
	}

	while(*ptr)
	{
		/* Only like numbers. */
		if (*ptr >= '0' && *ptr <= '9')
		{
			str_time[i] = *ptr;
			nums = 1;
			i++;
		}
		else if ((*ptr == 'S' || *ptr == 's') && nums) /* S means second */
			secs = 1;
		else if ((*ptr == 'M' || *ptr == 'm') && nums) /* S means Minute */
			mins = 1;
		else if ((*ptr == 'H' || *ptr == 'h') && nums) /* H means Hour */
			hours = 1;
		ptr++;
	}

	if (secs)
		time = atoi(str_time);
	else if(mins)
		time = (60*atoi(str_time));
	else if (hours)
		time = (3600*atoi(str_time));

	return time;
}

int cutString(char *pcSrc, const char *start, const char *end, char *pcDest)
{
	char *posStart = NULL, *posEnd = NULL;
	int len;

	if (0 == *pcSrc || NULL == pcDest)
	{
		return -1;
	}

	if (NULL == start)
	{
		posStart = pcSrc;
	}
	else
	{
		posStart = strstr(pcSrc, start);
		if (NULL == posStart)
		{
			return -1;
		}
		posStart++;
	}

	if (NULL == end)
	{
		posEnd = posStart+strlen(posStart);
	}
	else
	{
		posEnd = strstr(pcSrc, end);
		if (NULL == posEnd)
		{
			return -1;
		}
	}
	
	len = posEnd - posStart;
	strncpy(pcDest, posStart, len);
	return len;
}

/*
 * Separating the uri, for example: http://10.0.1.1:8080/path1/path2
 */
int separateUri(char *uri, char *ip, char *port, char *path)
{
	if (NULL == uri || NULL == ip || NULL == port || NULL == path)
	{
		return -1;
	}

	/* take ip */
	if (0 > cutString(uri+strlen("http://"), NULL, ":", ip))
	{
		return -1;
	}

	/* take port */
	if (0 > cutString(uri+strlen("http://")+strlen(ip)+strlen(":"), NULL, "/", port))
	{
		return -1;
	}

	/* take path */
	if (0 > cutString(uri+strlen("http://")+strlen(ip)+strlen(":")+strlen(port)+strlen("/"), NULL, NULL, path))
	{
		return -1;
	}

	return 0;
}


int getMAC(char *ifname, char *mac)
{
	struct ifreq ifr;
	int skfd;

	//printf("[%s][%d][%s]\n", __FILE__, __LINE__, __FUNCTION__);

	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
		printf("socket error\n");
		return -1;
	}

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0) 
	{
		printf("get_SysHWAddr: ioctl SIOCGIFHWADDR\n");
		close(skfd);
		return -1;
	}
	close(skfd);

	memcpy(mac, ifr.ifr_ifru.ifru_hwaddr.sa_data, IFHWADDRLEN);
	return 0;
}
/**
 * @brief	set ip of an interface
 * @param	"char *ifname" : interface name
 * @param	"in_addr_t addr" : ip address
 * @retval	0 : success ; -1 : fail
 */
int setIP(char *ifname, char *addr, int netmask)
{
	printf("[%s][%d][%s] Start ...\n", __FILE__, __LINE__, __FUNCTION__);

#if 0
	int i = 0, pos = 0;
	struct ifreq ifr;
	int skfd;
	struct in_addr in;
	struct sockaddr_in *host = NULL;
	unsigned int mask_hex = 0;

	/* Modify it later */
/*
	ControlSystemData(SFIELD_SET_IP, (void *)&ipaddr.s_addr, sizeof(ipaddr.s_addr));
*/

	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
		perror("socket error");
		return -1;
	}

	bzero(&ifr, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, ifname);
	host = (struct sockaddr_in*)&ifr.ifr_addr; 
	host->sin_family = AF_INET;

	if (1 != inet_pton(AF_INET, addr, &(host->sin_addr)))
	{
		close(skfd);
		return -1;
	}
	
	if (ioctl(skfd, SIOCSIFADDR, &ifr) < 0) 
	{
		printf("Line:[%d] ErrNo:[%d]\n", __LINE__, errno);
		close(skfd);
		return -1;
	}

	bzero(&ifr, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, ifname);
	host = (struct sockaddr_in*)&ifr.ifr_addr; 
	host->sin_family = AF_INET;
	
	for (i=31; i>(31-netmask); i--)
	{
		mask_hex |= (1<<i);
	}
	
	in.s_addr = htonl(mask_hex);
	DBG("LINE:[%d] mask = %s \n", __LINE__, inet_ntoa(in));
	
	if (1 != inet_pton(AF_INET, inet_ntoa(in), &(host->sin_addr)))
	{
		close(skfd);
		return -1;
	}
	
	if (ioctl(skfd, SIOCSIFNETMASK, &ifr) < 0)
	{
		close(skfd);
		return -1;
	}
	
	close(skfd);
#endif
    printf("[%s][%d][%s] End ...\n", __FILE__, __LINE__, __FUNCTION__);

	return 0;
}

/**
 * @brief	get address of an interface
 * @param	"char *ifname" : interface name
 * @retval	net address
 */
int getIP(char *ifname, char *ip)
{
	int skfd;
	struct ifreq ifr;
	struct sockaddr_in *saddr;

	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
		printf("socket error");
		return -1;
	}

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0) 
	{
		printf("net_get_ifaddr: ioctl SIOCGIFADDR \n");
		close(skfd);
		return -1;
	}
	close(skfd);
	
	saddr = (struct sockaddr_in *)&ifr.ifr_addr;
	stpcpy(ip, inet_ntoa(saddr->sin_addr));
	
	return 0;
}

/* return: status -> read/write/timeout -> 1: read, 0: write/timeout */
int selectSocket(int socket, time_t sec, suseconds_t usec)
{
	fd_set rfds;
	struct timeval tv;
	int retval;

	tv.tv_sec = sec;
	tv.tv_usec = usec;

	while(1)
	{
		FD_ZERO(&rfds);
		FD_SET(socket, &rfds);
		
		retval = select(socket+1, &rfds, NULL, NULL, &tv);
		if (retval == -1)
		{
			perror("select()");
			return -1;
		}
		else if (retval == 0)
		{
			/* time out */
			return 0;
		}
		else
		{
			/* Is read, or can send*/
			if(FD_ISSET(socket, &rfds))
				return 1;
			else 
				return 0;	
		}
	}

	return 0;
}

