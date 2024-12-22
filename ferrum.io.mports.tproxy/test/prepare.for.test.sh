#!/bin/bash
#sudo ip link add name fservice0 up type dummy
FORWARD_IP=100.100.100.100
echo "adding ip address $FORWARD_IP/32 to lo interface"
#sudo ip addr add $FORWARD_IP/32 dev lo || true

LISTEN_IP=192.168.105.105
#sudo ip link add name fservice1 up type dummy
echo "adding ip address $LISTEN_IP/32 to lo interface"
sudo ip addr add $LISTEN_IP/32 dev lo || true

SEND_IP=192.168.105.200
#sudo ip link add name fservice1 up type dummy
echo "adding ip address $SEND_IP/32 to lo interface"
sudo ip addr add $SEND_IP/32 dev lo || true

#docker stop nginx-mports || true
#docker run -it --rm -d -p $FORWARD_IP:8080:80 --name nginx-mports nginx

read -r -p "Do you want to continue to configure iptables ? (yes/no): " choice
case "$choice" in
yes | Yes | y | Y) echo "Continuing..." ;;
no | No | n | N)
    echo "Exiting..."
    exit 1
    ;;
*)
    echo "Invalid response"
    exit 1
    ;;
esac

#sudo iptables -A INPUT -i lo --destination 192.168.105.105 -p tcp -j DROP
#sudo iptables -A INPUT -i lo --destination 192.168.105.200 -p tcp -j DROP

sudo iptables -t nat -I PREROUTING -p tcp --destination 192.168.105.105 -m multiport --dport 1:65535 -j REDIRECT --to-ports 9000
sudo iptables -t nat -I OUTPUT -o lo -p tcp --destination 192.168.105.105 -m multiport --dport 1:65535 -j REDIRECT --to-ports 9000
sudo iptables -t nat -I PREROUTING -p udp --destination 192.168.105.105 -m multiport --dport 1:65535 -j REDIRECT --to-ports 9000
sudo iptables -t nat -I OUTPUT -o lo -p udp --destination 192.168.105.105 -m multiport --dport 1:65535 -j REDIRECT --to-ports 9000

### TPROXY
sudo ip route add local 192.168.105.105/32 dev lo src 127.0.0.1
sudo iptables -t mangle -I PREROUTING -d 192.168.105.105/32 -p tcp -j TPROXY --on-port=9000 --on-ip=127.0.0.1
sudo iptables -t mangle -I PREROUTING -d 192.168.105.105/32 -p udp -j TPROXY --tproxy-mark 0x1/0x1 --on-port=9000 --on-ip=127.0.0.1
sudo iptables -t mangle -A PREROUTING -d 192.168.105.105/32 -p tcp --dport 1:65000 -j TPROXY --on-port 9000 --on-ip 127.0.0.1 --tproxy-mark 0x1/0x1
sudo ip rule add fwmark 0x1/0x1 table 100
sudo ip route add local 0.0.0.0/0 dev lo table 100

sudo iptables -t mangle -N DIVERT
sudo iptables -t mangle -A PREROUTING -p tcp -m socket -j DIVERT
sudo iptables -t mangle -A DIVERT -j MARK --set-mark 0x1
sudo iptables -t mangle -A DIVERT -j ACCEPT
