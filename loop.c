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


#include "loop.h"

#define MAX(i, j) ((i>j)?i:j)

int loop(int cansocket, int udpsocket)
{
	fd_set readfds;
	struct can_frame frame;
	char buf[14];

	FD_ZERO(&readfds);
	FD_SET(cansocket, &readfds);
	FD_SET(udpsocket, &readfds);

	for (;;)
	{
		if (select(MAX(cansocket, udpsocket)+1, &readfds, NULL, NULL, NULL) < 0)
		{
			perror("select");
			return -1;
		}
		if(FD_ISSET(cansocket, &readfds))
		{
			if (read(cansocket, &frame, sizeof(struct can_frame)) < 0)
			{
				perror("read can");
				return -1;
			}
			buf[0] = 0;
			buf[1] = (frame.can_id>>24)&0xff;
			buf[2] = (frame.can_id>>16)&0xff;
			buf[3] = (frame.can_id>>8)&0xff;
			buf[4] = frame.can_id&0xff;
			buf[5] = frame.can_dlc;
			memcpy(buf+6, frame.data, frame.can_dlc);
			if (write(udpsocket, buf, frame.can_dlc+6) < 0)
			{
				perror("write udp");
				return -1;
			}
		}
		if(FD_ISSET(udpsocket, &readfds))
		{
			if (read(udpsocket, buf, sizeof(buf)) < 0)
			{
				perror("read udp");
				return -1;
			}
			buf[0] = 0;
			frame.can_id = buf[1]<<24|buf[2]<<16|buf[3]<<8|buf[4];
			frame.can_dlc = buf[5];
			memcpy(frame.data, buf+6, frame.can_dlc);
			if (write(cansocket, &frame, sizeof(struct can_frame)) < 0)
			{
				perror("write can");
				return -1;
			}
		}
	}
}
