#!/bin/bash
#$1: short_addr, $2: ipv6 addr
#usage: ./lowpan_if.sh 0x001 fe80::1:1/64

echo "Initializing lowpan for this device"
iwpan dev wpan0 set pan_id 0xbeef
iwpan dev wpan0 set short_addr $1
ip link add link wpan0 name lowpan0 type lowpan
ip link set wpan0 up
ip link set lowpan0 up

#crontab doesn't include /sbin
/sbin/ifconfig lowpan0 inet6 add $2
