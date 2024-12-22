# ferrum.io.mports

udp tcp service that support multiple port range listen

# prepare for ebpf

sudo apt install -y clang llvm gcc libbpf-dev libxdp-dev xdp-tools bpftool linux-headers-$(uname -r)

# compile libbpf

cd libppf/src
mkdir -p build root
BUILD_STATIC_ONLY=y OBJDIR=build DESTDIR=root make install
cp -R root/* ../../lib/

# compile ebpf

bash make.sh
sudo ip link set dev $INTERFACE xdpgeneric off
sudo ip link set dev $INTERFACE xdpgeneric obj xdp_ip_filter.o sec filter

# iptables

sudo iptables -A INPUT -i lo --destination 192.168.105.105 -p tcp -j DROP

# setcap

sudo setcap cap_net_raw,cap_net_admin=eip ./fgport
sudo setcap cap_bpf=ep ./fgport

# ebpf maps

sudo mount -t bpf bpf /sys/fs/bpf/
sudo bpftool -f map show
