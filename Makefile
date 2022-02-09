can2udp: can2udp.o can.o udp.o loop.o
	gcc can2udp.o can.o udp.o loop.o -o can2udp

install: can2udp
	install -m 0755 can2udp /usr/local/bin

