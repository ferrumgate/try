#!/bin/bash
WORKSPACE=$(pwd)/../..
clang -O2 -target bpf -c socket_ip_filter.c -o socket_ip_filter.o

cp socket_ip_filter.o "${WORKSPACE}/build/fgport/socket_ip_filter.o"
