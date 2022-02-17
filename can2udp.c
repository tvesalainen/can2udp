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

int verbose = 0;

int main(int argc, char **argv)
{
	int c;
	char* canbus = NULL;
	char* address = NULL;
	struct sockaddr_in saddr;
	int udpport = -1;
	int cansocket;
	int udpsocket;
	int use_stderr = 1;

	while ((c = getopt (argc, argv, "a:vb:p:s")) != -1)
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
			case 's':
				use_stderr = 0;
			break;
			default:
				fprintf(stderr, "usage:xxx\n");
				exit(-1);
		}
	}
	if (use_stderr)
	{
		openlog(NULL, LOG_PERROR, LOG_USER);
	}
	else
	{
		openlog(NULL, 0, LOG_USER);
	}
	if (address == NULL || canbus == NULL || udpport == -1)
	{
		fprintf(stderr, "usage: \n"
				"\t-v\t\tverbose\n"
				"\t-a <address>\tmulticast address\n"
				"\t-p <port>\tmulticast port\n"
				"\t-b <canbus>\tcan bus\n"
				"\t-s \t\tlog only to syslog\n"
			);
		exit(-1);
	}
	VERBOSE("can bus  = %s\n", canbus);
	VERBOSE("address  = %s\n", address);
	VERBOSE("udp port = %d\n", udpport);

	cansocket = open_can(canbus);	
	if (cansocket < 0)
	{
		ERROR("%m: open can");
		exit(-1);
	}
	if (get_address(&saddr, address, udpport) < 0)
	{
		ERROR("%m: get_address");
		exit(-1);
	}
	udpsocket = open_udp(&saddr, address, udpport);	
	if (udpsocket < 0)
	{
		ERROR("%m: open udp");
		exit(-1);
	}

	loop(&saddr, cansocket, udpsocket);
}

