#!/bin/sh
#sudo ./pkt_log -i eth0 -f tcp dst port 80 -o pkt_log
sudo ./pkt_log -i lo -f udp dst port 12345 -o pkt_log
sudo chown xxx-xxx-xxx-xxx *
