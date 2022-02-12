
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/can.h>

#include "can2udp.h"
#include "j1939.h"
#include "fast.h"

struct fast_data* sources[MAX_FAST];

int handle_fast(struct can_frame *frame, int forward, struct sockaddr_in *addr)
{
	struct fast_data *fast;
	char buf[230];
	int id;
	int seq;

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
		if (id != fast->id)
		{
			if (seq == 0)
			{
				fast->pgn = PGN(frame->can_id);
				fast->id = id;
				fast->msg.type = 0xf;
				fast->msg.can_id[0] = frame->can_id>>24;
				fast->msg.can_id[1] = frame->can_id>>16;
				fast->msg.can_id[2] = frame->can_id>>8;
				fast->msg.can_id[3] = frame->can_id;
				fast->msg.len = frame->data[1];
				memcpy(fast->msg.data, frame->data+2, 6);
				fast->cnt = 6;
			}
		}
		else
		{
			memcpy(fast->msg.data+6+(seq-1)*7, frame->data+1, 7);
			fast->cnt += 7;
			
		}
		if (fast->cnt >= fast->msg.len)
		{
			if (sendto(forward, &(fast->msg), fast->msg.len+6, 0, (struct sockaddr*)addr, sizeof(struct sockaddr_in)) < 0)
			{
				ERROR("%m: write udp");
				return -1;
			}
		}
	}
	else
	{
		buf[0] = 0;
		buf[1] = frame->can_id>>24;
		buf[2] = frame->can_id>>16;
		buf[3] = frame->can_id>>8;
		buf[4] = frame->can_id;
		buf[5] = frame->can_dlc;
		memcpy(buf+6, frame->data, frame->can_dlc);
		if (sendto(forward, buf, frame->can_dlc+6, 0, (struct sockaddr*)addr, sizeof(struct sockaddr_in)) < 0)
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
