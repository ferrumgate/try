#include "fconfig.h"
#include <gtest/gtest.h>

using namespace Ferrum;

TEST(FConfigTest, loadConfigWithDefaultValues) {
  FConfig fconfig{};
  fconfig.loadConfig();
  EXPECT_STREQ(fconfig.getEbpfPath().c_str(), "socket_ip_filter.o");
  EXPECT_TRUE(fconfig.getIsEbpfEnabledTcp());
  EXPECT_TRUE(fconfig.getIsEbpfEnabledUdp());
  EXPECT_STREQ(fconfig.getForwardSrcIp().c_str(), "0.0.0.0");
  EXPECT_STREQ(fconfig.getForwardDstIp().c_str(), "0.0.0.0");
  EXPECT_STREQ(fconfig.getForwardInterface().c_str(), "lo");
  EXPECT_STREQ(fconfig.getListenIp().c_str(), "0.0.0.0");
  EXPECT_STREQ(fconfig.getListenInterface().c_str(), "lo");

  auto tcpPortList = fconfig.getListenTcpPorts();
  EXPECT_EQ(tcpPortList.size(), 1);
  EXPECT_EQ(tcpPortList[0].first, 0);
  EXPECT_EQ(tcpPortList[0].second, UINT16_MAX);

  auto udpPortList = fconfig.getListenUdpPorts();
  EXPECT_EQ(udpPortList.size(), 1);
  EXPECT_EQ(udpPortList[0].first, 0);
  EXPECT_EQ(udpPortList[0].second, UINT16_MAX);

  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}

// load config from environment variables
TEST(FConfigTest, loadConfigWithEnvironmentValues) {
  setenv("EBPF_PATH", "socket_ip_filter_2.o", 1);
  setenv("IS_EBPF_ENABLED_TCP", "FALSE", 1);
  setenv("IS_EBPF_ENABLED_UDP", "FALSE", 1);
  setenv("FORWARD_DST_IP", "1.2.3.4", 1);
  setenv("FORWARD_SRC_IP", "1.2.3.6", 1);
  setenv("FORWARD_INTERFACE", "eth0", 1);
  setenv("LISTEN_IP", "1.2.3.5", 1);
  setenv("LISTEN_INTERFACE", "eth1", 1);
  setenv("LISTEN_TCP_PORTS", "80-81;8080-8081", 1);
  setenv("LISTEN_UDP_PORTS", "82-83;8082-8085", 1);
  FConfig fconfig{};
  fconfig.loadConfig();
  EXPECT_STREQ(fconfig.getEbpfPath().c_str(), "socket_ip_filter_2.o");
  EXPECT_FALSE(fconfig.getIsEbpfEnabledTcp());
  EXPECT_FALSE(fconfig.getIsEbpfEnabledUdp());
  EXPECT_STREQ(fconfig.getForwardSrcIp().c_str(), "1.2.3.6");
  EXPECT_STREQ(fconfig.getForwardDstIp().c_str(), "1.2.3.4");
  EXPECT_STREQ(fconfig.getForwardInterface().c_str(), "eth0");
  EXPECT_STREQ(fconfig.getListenIp().c_str(), "1.2.3.5");
  EXPECT_STREQ(fconfig.getListenInterface().c_str(), "eth1");
  auto tcpPortList = fconfig.getListenTcpPorts();
  EXPECT_EQ(tcpPortList.size(), 2);
  EXPECT_EQ(tcpPortList[0].first, 80);
  EXPECT_EQ(tcpPortList[0].second, 81);
  EXPECT_EQ(tcpPortList[1].first, 8080);
  EXPECT_EQ(tcpPortList[1].second, 8081);

  auto udpPortList = fconfig.getListenUdpPorts();
  EXPECT_EQ(udpPortList.size(), 2);
  EXPECT_EQ(udpPortList[0].first, 82);
  EXPECT_EQ(udpPortList[0].second, 83);
  EXPECT_EQ(udpPortList[1].first, 8082);
  EXPECT_EQ(udpPortList[1].second, 8085);

  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}