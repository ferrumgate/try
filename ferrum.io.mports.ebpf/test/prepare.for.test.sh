#!/bin/bash
#sudo ip link add name fservice0 up type dummy
FORWARD_IP=100.100.100.100
echo "adding ip address $FORWARD_IP/32 to lo interface"
sudo ip addr add $FORWARD_IP/32 dev lo || true

LISTEN_IP=192.168.105.105
#sudo ip link add name fservice1 up type dummy
echo "adding ip address $LISTEN_IP/32 to lo interface"
sudo ip addr add $LISTEN_IP/32 dev lo || true

SEND_IP=192.168.105.200
#sudo ip link add name fservice1 up type dummy
echo "adding ip address $SEND_IP/32 to lo interface"
sudo ip addr add $SEND_IP/32 dev lo || true

docker stop nginx-mports || true
docker run -it --rm -d -p $FORWARD_IP:8080:80 --name nginx-mports nginx

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

sudo iptables -A INPUT --source 192.168.88.51 --destination 192.168.105.105 -p tcp -j DROP
sudo iptables -A INPUT --source 100.100.100.100 --destination 192.168.105.200 -p tcp -j DROP

#readme
# start server with
# nc -lv 100.100.100.100 8080
# start client with on the other host
# nc 192.168.105.105 8080
