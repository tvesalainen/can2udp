
#ifndef _FAST_H
#define _FAST_H

#include <sys/socket.h>
#include <linux/can.h>

#define MAX_FAST 223

#define ID(b)	((b)&0xe0)
#define SEQ(b)	((b)&0x1f)

struct fast_msg
{
	__u8 type;
	__u8 can_id[4];
	__u8 len;
	__u8 data[MAX_FAST];
};
struct fast_data
{
	__u32 pgn;
	__u8 id;
	__u8 cnt;
	struct fast_msg msg;
};
int is_fast(int can_id);
int handle_fast(struct can_frame *frame, int forward, struct sockaddr_in *addr);

#endif /* _FAST_H */
