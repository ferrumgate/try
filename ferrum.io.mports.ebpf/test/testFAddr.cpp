#include "faddr.h"
#include <gtest/gtest.h>

using namespace Ferrum;

TEST(TestFAddr, v4) {
  sockaddr_in v4;
  v4.sin_family = AF_INET;
  v4.sin_port = htons(3490);
  inet_aton("192.168.100.10", reinterpret_cast<struct in_addr *>(&v4.sin_addr.s_addr));
  FAddr fv4{v4};
  EXPECT_TRUE(fv4.isV4());
  EXPECT_EQ(fv4.toString(), "192.168.100.10");
  EXPECT_EQ(fv4.toStringWithPort(), "192.168.100.10:3490");

  FAddr fv4_2{inet_addr("100.101.100.199"), 8080};
  EXPECT_TRUE(fv4_2.isV4());
  EXPECT_EQ(fv4_2.toString(), "100.101.100.199");
}

TEST(TestFAddr, v6) {
  sockaddr_in6 v6;
  v6.sin6_family = AF_INET6;
  v6.sin6_port = htons(3490);
  uv_ip6_addr("2001:db8::ff00:42:8329", 3490, (&v6));
  FAddr fv6{v6};
  EXPECT_TRUE(fv6.isV6());
  EXPECT_EQ(fv6.toString(), "2001:db8::ff00:42:8329");
  EXPECT_EQ(fv6.toStringWithPort(), "2001:db8::ff00:42:8329:[3490]");
}

TEST(TestFAddr, operatorEq) {
  sockaddr_in v4;
  v4.sin_family = AF_INET;
  v4.sin_port = htons(3490);
  inet_aton("192.168.100.10", reinterpret_cast<struct in_addr *>(&v4.sin_addr.s_addr));
  FAddr fv4{v4};
  FAddr fv4_2{v4};
  EXPECT_TRUE(fv4 == fv4_2);
}

TEST(TestFAddr, from) {
  auto result = FAddr::from("192.168.100.10", 8081);
  EXPECT_TRUE(result.isOk());
  FAddr fv4 = result.data;
  EXPECT_TRUE(fv4.isV4());
  EXPECT_EQ(fv4.toString(), "192.168.100.10");
  EXPECT_EQ(fv4.toStringWithPort(), "192.168.100.10:8081");
}