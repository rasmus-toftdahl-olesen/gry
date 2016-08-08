#!/bin/sh

DATA="`hostname -s`.CPU `cat /proc/loadavg |cut -f1 -d' '`"
HOST="127.0.0.1"

echo -n $DATA | socat - udp-datagram:$HOST:8124
