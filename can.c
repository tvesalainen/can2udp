#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>

#include "can2udp.h"
#include "can.h"

int open_can(char * bus)
{
	int s;
	struct sockaddr_can addr;
	struct ifreq ifr;

	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (s < 0)
	{
		ERROR("%m: socket can");
		return -1;
	}
	strcpy(ifr.ifr_name, bus );
	if (ioctl(s, SIOCGIFINDEX, &ifr) < 0)
	{
		ERROR("%m: ioctl can");
		return -1;
	}

	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		ERROR("%m: bind can");
		return -1;
	}

	return s;
}
