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
#include "can.h"
#include "udp.h"
#include "loop.h"

int main(int argc, char **argv)
{
	int c;
	char* canbus;
	char* address;
	int udpport;
	int cansocket;
	int udpsocket;

	while ((c = getopt (argc, argv, "a:vb:p:")) != -1)
	{
		switch (c)
		{
			case 'v':
				verbose = 1;
			break;
			case 'a':
				address = optarg;
			break;
			case 'b':
				canbus = optarg;
			break;
			case 'p':
				udpport = atoi(optarg);
			break;
			default:
				fprintf(stderr, "usage:xxx\n");
				exit(-1);
		}
	}
	VERBOSE("can bus  = %s\n", canbus);
	VERBOSE("address  = %s\n", address);
	VERBOSE("udp port = %d\n", udpport);

	cansocket = open_can(canbus);	
	if (cansocket < 0)
	{
		perror("open can");
		exit(-1);
	}
	udpsocket = open_udp(address, udpport);	
	if (udpsocket < 0)
	{
		perror("open udp");
		exit(-1);
	}

	loop(cansocket, udpsocket);
}

