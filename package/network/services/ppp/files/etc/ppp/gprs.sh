#!/bin/sh
sleep 10
#ifconfig eth0 down
fw3 stop
#iptables -A PREROUTING -p udp -m udp --dport 53 -j ACCEPT
#iptables -A PREROUTING -p tcp -m tcp --dport 53 -j ACCEPT
iptables -t nat -F
iptables -t nat -A PREROUTING -p udp -m udp --dport 53 -j DNAT --to-destination 114.114.114.114
iptables -t nat -A PREROUTING -p tcp -m tcp --dport 53 -j DNAT --to-destination 114.114.114.114
iptables -t nat -A POSTROUTING -s 192.168.1.0/24 -o ppp0 -j MASQUERADE

dns1=" "
dns2=" "
cd /etc/ppp/peers
pppd call gprs&
echo "pppd ok"
sleep 8
echo "sleep ok"
cp -rf /etc/ppp/resolv.conf /etc/
sed -n '1p' /etc/resolv.conf > /etc/ppp/primarydns
sed -n '2p' /etc/resolv.conf > /etc/ppp/seconddns
dns1=`cut -f 2 -d ' ' /etc/ppp/primarydns`
dns2=`cut -f 2 -d ' ' /etc/ppp/seconddns`
echo $dns1
echo $dns2
