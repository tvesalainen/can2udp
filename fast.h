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

#ifndef _FAST_H
#define _FAST_H

#include <sys/socket.h>
#include <linux/can.h>

#define MAX_FAST 223
#define MSG_HDR 6

#define ID(b)	((b)&0xe0)
#define SEQ(b)	((b)&0x1f)

struct udp_msg
{
	__u8 type;
	__u8 can_id[4];
	__u8 len;
	__u8 data[MAX_FAST];
};
struct fast_data
{
	__u32 can_id;
	__u8 id;
	__u8 cnt;
	struct udp_msg msg;
};
int is_fast(int can_id);
int can2udp(struct can_frame *frame, int forward, struct sockaddr_in *addr);
int udp2can(struct udp_msg *msg, int forward);

#endif /* _FAST_H */
