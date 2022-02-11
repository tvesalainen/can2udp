
#ifndef _UDP_H
#define _UDP_H

int open_udp(struct sockaddr_in *saddr, char* address, int port);
int get_address(struct sockaddr_in *saddr, char* address, int port);

#endif /* _UDP_H */
