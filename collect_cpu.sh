#!/bin/sh

echo -n "CPU `cat /proc/loadavg |cut -f1 -d' '`"| socat - udp-datagram:127.0.0.1:8124
