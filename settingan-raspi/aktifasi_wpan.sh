#!/bin/bash
#$1: pan_id, $2: short_addr

echo "Initializing wpan for this device"
iwpan dev wpan0 set pan_id $1
iwpan dev wpan0 set short_addr $2
ip link add link wpan0 name lowpan0 type lowpan
ip link set wpan0 up
ip link set lowpan0 up
