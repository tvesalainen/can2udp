
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "can2udp.h"
#include "udp.h"

int open_udp(struct sockaddr_in *saddr, char* address, int port)
{
	int s;
	int true = 1;
	int false = 0;
	struct sockaddr_in raddr;
	struct ip_mreqn group;

	VERBOSE("socket\n");
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
	{
		ERROR("%m: socket");
		return -1;
	}
	VERBOSE("setsockopt SO_REUSEADDR\n");
	if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(true)) < 0)
	{
		ERROR("%m: SO_REUSEADDR");
		return -1;
	}
/*
	VERBOSE("setsockopt SO_BROADCAST\n");
	if(setsockopt(s, SOL_SOCKET, SO_BROADCAST, &true, sizeof(true)) < 0)
	{
		ERROR("%m: SO_BROADCAST");
		return -1;
	}
*/
	group.imr_multiaddr.s_addr = inet_addr(address);
	group.imr_address.s_addr = htonl(INADDR_ANY);
	group.imr_ifindex = 0;
	VERBOSE("setsockopt IP_ADD_MEMBERSHIP\n");
	if(setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP ,&group, sizeof(group)) < 0)
	{
		ERROR("%m: IP_ADD_MEMBERSHIP");
		return -1;
	}
	VERBOSE("setsockopt IP_MULTICAST_LOOP\n");
	if(setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP ,&false, sizeof(false)) < 0)
	{
		ERROR("%m: IP_MULTICAST_LOOP");
		return -1;
	}

	memset(&raddr, 0, sizeof(raddr));
	raddr.sin_family = AF_INET;
	raddr.sin_addr.s_addr = htonl(INADDR_ANY);
	raddr.sin_port = htons(port);
	if (bind(s, (struct sockaddr *)&raddr, sizeof(struct sockaddr)) < 0)
	{
		ERROR("%m: bind");
		return -1;
	}
	return s;
}
int get_address(struct sockaddr_in *saddr, char* address, int port)
{
	struct hostent *h; 

	h=gethostbyname(address);
	if(h == NULL)
	{
		ERROR("%m: gethostbyname");
		return -1;
	}
	VERBOSE("gethostbyname %s\n", address);
	memset(saddr, '\0', sizeof(struct sockaddr));
	saddr->sin_family = AF_INET;
	saddr->sin_port = htons(port); 
	memcpy((char *) &(saddr->sin_addr.s_addr), h->h_addr_list[0], h->h_length);
	return 1;
}
