
can2udp: can2udp.o udp.o can.o loop.o fast.o
	gcc can2udp.o can.o udp.o loop.o fast.o -o can2udp

install: can2udp
	install -m 0755 can2udp /usr/local/bin

