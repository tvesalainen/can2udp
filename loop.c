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
#include "loop.h"

#define MAX(i, j) ((i>j)?i:j)

int loop(struct sockaddr_in *saddr, int cansocket, int udpsocket)
{
	fd_set readfds;
	int nf = MAX(cansocket, udpsocket)+1;
	struct can_frame frame;
	char buf[14];

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
			fastprefix(frame.data[0], frame.data[1]);
			VERBOSE("can: id %x len %d %x %x %x %x %x %x %x %x\n", frame.can_id, frame.can_dlc, frame.data[0], frame.data[1], frame.data[2], frame.data[3], frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
			buf[0] = 0;
			buf[1] = frame.can_id>>24;
			buf[2] = frame.can_id>>16;
			buf[3] = frame.can_id>>8;
			buf[4] = frame.can_id;
			buf[5] = frame.can_dlc;
			memcpy(buf+6, frame.data, frame.can_dlc);
			if (sendto(udpsocket, buf, frame.can_dlc+6, 0, (struct sockaddr*)saddr, sizeof(struct sockaddr_in)) < 0)
			{
				ERROR("%m: write udp");
				return -1;
			}
		}
		if(FD_ISSET(udpsocket, &readfds))
		{
			if (recvfrom(udpsocket, buf, sizeof(buf), 0, NULL, NULL) < 0)
			{
				ERROR("%m: read udp");
				return -1;
			}
			if (buf[0] == 0)
			{
				frame.can_id = buf[1]<<24|buf[2]<<16|buf[3]<<8|buf[4];
				frame.can_dlc = buf[5];
				VERBOSE("udp: id %x len %d %x %x %x %x %x %x %x %x\n", frame.can_id, frame.can_dlc, frame.data[0], frame.data[1], frame.data[2], frame.data[3], frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
				memcpy(frame.data, buf+6, frame.can_dlc);
				if (write(cansocket, &frame, sizeof(struct can_frame)) < 0)
				{
					ERROR("%m: write can");
					return -1;
				}
			}
			else
			{
				VERBOSE("wrong version %d\n", buf[0]);
			}
		}
	}
}
