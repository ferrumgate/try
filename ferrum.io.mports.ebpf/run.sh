#!/bin/bash
if [ "$EUID" -ne 0 ]; then
    echo "please run with sudo"
    exit 1
fi
if ! cd build/fgport; then
    echo "Failed to change directory to build/fgport"
    exit 1
fi

mount -t bpf bpf /sys/fs/bpf/ || true

export EBPF_PATH="$(pwd)/socket_ip_filter.o"
export IS_EBPF_ENABLED_TCP=TRUE
export IS_EBPF_ENABLED_UDP=TRUE
export FORWARD_DST_IP=100.100.100.100
export FORWARD_SRC_IP=192.168.105.200
export FORWARD_INTERFACE="lo"
export LISTEN_IP=192.168.105.105
export LISTEN_INTERFACE="lo"
export LISTEN_TCP_PORTS="80-80;8080-8080"
export LISTEN_UDP_PORTS="80-80;8080-8080"

ulimit -Hc unlimited

#echo "$(pwd)/core.%e.%p.%t" | sudo tee /proc/sys/kernel/core_pattern
echo "core pattern is set to"
echo "$(pwd)/core.%e" | tee /proc/sys/kernel/core_pattern

./fgport
echo "checking core file in $(pwd)"
ls "$(pwd)/core.*" || true
