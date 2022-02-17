# Can2Udp

Can2Udp connects can bus to ip network. 
Can messages are forwarded to ip network by using multicast udp messages. 
Incoming multicast messages are written to can bus.

N2K fast messages are sent as one udp datagram.

Setup can have several physical can busses and several virtual can busses.
It is also possible to implement client to access can from other that can bus.


# Installation

```
git clone https://github.com/tvesalainen/can2udp.git
cd can2udp
make
```

To install in /usr/local/bin run
```
sudo make install
```

Build debian package with command
```
debuild -us -uc
```

install with command
```
sudo dpkg -i ../can2udp_0.1_XXXXX.deb
```

Note! Debian installing starts service can2udp with default settings. 
Stop it with ``service can2udp stop`` and edit /etc/default/can2udp. 
After that run ``service can2udp start``.

Note! Receiving NMEA 2000 fast packet from udp generates max 32 writes to can bus.
With default can setting ``txqueuelen 10`` it will crash.
Try:
```
ip link set canX txqueuelen 1000
```

# Command line

```
can2udp
usage:
        -v              verbose
        -a <address>    multicast address
        -p <port>       multicast port
        -b <canbus>     can bus
        -s              log only to syslog
```

# Test with virtual can
## Real can in host A
```
can2udp -b can0 -a 224.0.0.3 -p 11111
```
## Virtual can in host B
```
can2udp -b vcan0 -a 224.0.0.3 -p 11111
```
