/*
 * Copyright (C) 2022 Timo Vesalainen <timo.vesalainen@iki.fi>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/can.h>

#include "can2udp.h"
#include "j1939.h"
#include "fast.h"

struct fast_data* sources[MAX_FAST];
__u8 fast_id = 0;

int udp2can(struct udp_msg *msg, int forward)
{
	struct can_frame frame;
	int id;
	int len;
	int seq;

	frame.can_id = msg->can_id[0]<<24|msg->can_id[1]<<16|msg->can_id[2]<<8|msg->can_id[3];
	frame.can_dlc = 8;

	if (msg->type == 0xf)
	{
		id = fast_id++<<5;
		len = msg->len;
		seq = 0;
		while (len > 0)
		{
			if (seq == 0)
			{
				frame.data[0] = id;
				frame.data[1] = len;
				memcpy(frame.data+2, &(msg->data), 6);
				if (write(forward, &frame, sizeof(struct can_frame)) < 0)
				{
					ERROR("%m: write can");
					return -1;
				}
				len -= 6;
			}
			else
			{
				frame.data[0] = id|seq;
				memcpy(frame.data+1, (char*)&(msg->data)+6+(seq-1)*7, 7);
				if (write(forward, &frame, sizeof(struct can_frame)) < 0)
				{
					ERROR("%m: write can");
					return -1;
				}
				len -= 7;
			}
			VERBOSE("udp:%x %02x%02x%02x%02x%02x%02x%02x%02x\n", frame.can_id, frame.data[0], frame.data[1], frame.data[2], frame.data[3], frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
			seq++;
		}
	}
	else
	{
		if (msg->type == 0)
		{
			memcpy(frame.data, &(msg->data), msg->len);
			if (write(forward, &frame, sizeof(struct can_frame)) < 0)
			{
				ERROR("%m: write can");
				return -1;
			}

		}
		else
		{
			VERBOSE("wrong type %d\n", msg->type);
			return -1;
		}
		VERBOSE("udp:%x %02x%02x%02x%02x%02x%02x%02x%02x\n", frame.can_id, frame.data[0], frame.data[1], frame.data[2], frame.data[3], frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
	}
	return 0;
}

int can2udp(struct can_frame *frame, int forward, struct sockaddr_in *addr)
{
	struct fast_data *fast;
	struct udp_msg msg;
	int id;
	int seq;
	int ii;

	if (is_fast(frame->can_id))
	{
		int sa = SA(frame->can_id);
		fast = sources[sa];
		if (fast == NULL)
		{
			fast = calloc(1, sizeof(struct fast_data));
			fast->id = 0xff;
			sources[sa] = fast;
		}	
		id = ID(frame->data[0]);
		seq = SEQ(frame->data[0]);
		VERBOSE("can:%x %02x%02x%02x%02x%02x%02x%02x%02x\n", frame->can_id, frame->data[0], frame->data[1], frame->data[2], frame->data[3], frame->data[4], frame->data[5], frame->data[6], frame->data[7]);
		if (id != fast->id || fast->can_id != frame->can_id)
		{
			fast->id = 0xff;
		}
		if (seq == 0)
		{
			fast->can_id = frame->can_id;
			fast->id = id;
			fast->cnt = 6;
			fast->msg.type = 0xf;
			fast->msg.can_id[0] = frame->can_id>>24;
			fast->msg.can_id[1] = frame->can_id>>16;
			fast->msg.can_id[2] = frame->can_id>>8;
			fast->msg.can_id[3] = frame->can_id;
			fast->msg.len = frame->data[1];
			memcpy(fast->msg.data, frame->data+2, 6);
		}
		else
		{
			if (fast->id != 0xff)
			{
				memcpy(fast->msg.data+6+(seq-1)*7, frame->data+1, 7);
				fast->cnt += 7;
//fprintf(stderr, "cnt %d len %d\n", fast->cnt, fast->msg.len);
				if (fast->cnt >= fast->msg.len)
				{
/*
for (ii=0;ii<fast->msg.len;ii++)
{
fprintf(stderr, "%02x", fast->msg.data[ii]);
}
fprintf(stderr, "\n");
*/
					if (sendto(forward, &(fast->msg), fast->msg.len+6, 0, (struct sockaddr*)addr, sizeof(struct sockaddr_in)) < 0)
					{
						ERROR("%m: write udp");
						return -1;
					}
					fast->id = 0xff;
				}
			}
		}
	}
	else
	{
		msg.type = 0;
		msg.can_id[0] = frame->can_id>>24;
		msg.can_id[1] = frame->can_id>>16;
		msg.can_id[2] = frame->can_id>>8;
		msg.can_id[3] = frame->can_id;
		msg.len = frame->can_dlc;
		memcpy(&(msg.data), frame->data, frame->can_dlc);
		if (sendto(forward, &msg, frame->can_dlc+MSG_HDR, 0, (struct sockaddr*)addr, sizeof(struct sockaddr_in)) < 0)
		{
			ERROR("%m: write udp");
			return -1;
		}
	}
	return 0;
}
int is_fast(int can_id)
{
	switch (PGN(can_id))
	{
		case 127489:
		case 126464:
		case 126208:
		case 129795:
		case 129794:
		case 129797:
		case 129029:
		case 129285:
		case 127237:
		case 129540:
		case 129796:
		case 129284:
		case 127497:
		case 127496:
		case 127498:
		case 129039:
		case 127503:
		case 129038:
		case 129041:
		case 129809:
		case 130320:
		case 129040:
		case 128275:
		case 130578:
		case 129810:
		case 129045:
		case 129044:
		case 126996:
		case 126998:
			return 1;
		default:
			return 0;
	}
}
