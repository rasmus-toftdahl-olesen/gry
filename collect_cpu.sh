#!/bin/sh

HOST="127.0.0.1"

DATA="`hostname -s`.CPU `cat /proc/loadavg |cut -f1 -d' '`"
echo -n $DATA | socat - udp-datagram:$HOST:8124

DATA="`hostname -s`.day `date +%j`"
echo -n $DATA | socat - udp-datagram:$HOST:8124

DATA="`hostname -s`.hour `date +%H`"
echo -n $DATA | socat - udp-datagram:$HOST:8124

DATA="`hostname -s`.minute `date +%M`"
echo -n $DATA | socat - udp-datagram:$HOST:8124

DATA="`hostname -s`.uptime `cat /proc/uptime |cut -f1 -d' '`"
echo -n $DATA | socat - udp-datagram:$HOST:8124
