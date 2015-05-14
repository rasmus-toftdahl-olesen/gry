#!/bin/bash

echo -n "random $RANDOM"| socat - udp-datagram:127.0.0.1:8124
