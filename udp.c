
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

#include "udp.h"

int open_udp(char* address, int port)
{
	int s;
	int true = 1;
	int false = 0;
	struct sockaddr_in saddr;
	struct hostent *h; 

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
	{
		perror("socket");
		return -1;
	}
	if(setsockopt(s, SOL_SOCKET, SO_BROADCAST, &true, sizeof(true)) < 0)
	{
		return -1;
	}
	if(setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP ,&false, sizeof(false)) < 0)
	{
		return -1;
	}
	h=gethostbyname(address);
	if(h == NULL)
	{
		perror("gethostbyname");
		return -1;
	}
	memset(&saddr, '\0', sizeof(saddr));
	saddr.sin_family = AF_INET;
	memcpy((char *) &saddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
	saddr.sin_port = htons(port); 

	if (bind(s, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
	{
		perror("bind");
		return -1;
	}
	if (connect(s, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
	{
		perror("connect");
		return -1;
	}
	return s;
}
