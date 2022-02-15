#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>

#include <linux/can.h>

#include "can2udp.h"
#include "fast.h"
#include "loop.h"

#define MAX(i, j) ((i>j)?i:j)

int loop(struct sockaddr_in *saddr, int cansocket, int udpsocket)
{
	fd_set readfds;
	int nf = MAX(cansocket, udpsocket)+1;
	struct can_frame frame;
	struct udp_msg msg;

	FD_ZERO(&readfds);

	for (;;)
	{
		FD_SET(cansocket, &readfds);
		FD_SET(udpsocket, &readfds);
		if (select(nf, &readfds, NULL, NULL, NULL) < 0)
		{
			ERROR("%m: select");
			return -1;
		}
		if(FD_ISSET(cansocket, &readfds))
		{
			if (read(cansocket, &frame, sizeof(struct can_frame)) < 0)
			{
				ERROR("%m: read can");
				return -1;
			}
			if (can2udp(&frame, udpsocket, saddr) < 0)
			{
				ERROR("%m: can -> udp");
				return -1;
			}
		}
		if(FD_ISSET(udpsocket, &readfds))
		{
			if (recvfrom(udpsocket, &msg, sizeof(msg), 0, NULL, NULL) < 0)
			{
				ERROR("%m: read udp");
				return -1;
			}
			if (udp2can(&msg, cansocket) < 0)
			{
				ERROR("%m: udp -> can");
				return -1;
			}
		}
	}
}
