#include <gtest/gtest.h>

#include "fnatTable.h"
using namespace Ferrum;

TEST(TestFNatTable, addNat_getNat) {
  FNatTable table{10};
  auto srcAddrSPtr = FAddrSharedPtr{new FAddr{0x01020304, 0x1234}};
  auto dstAddrSPtr = FAddrSharedPtr{new FAddr{0x05060708, 0x5678}};
  auto result1 = table.addNat(srcAddrSPtr, dstAddrSPtr);
  EXPECT_TRUE(result1.isOk());
  EXPECT_TRUE(result1.data > 0);
  auto result2 = table.getNat(srcAddrSPtr);
  EXPECT_TRUE(result2.isOk());
}

TEST(TestFNatTable, addNat_getNatPort) {
  FNatTable table{10};
  auto srcAddrSPtr = FAddrSharedPtr{new FAddr{0x01020304, 0x1234}};
  auto dstAddrSPtr = FAddrSharedPtr{new FAddr{0x05060708, 0x5678}};
  auto result1 = table.addNat(srcAddrSPtr, dstAddrSPtr);
  EXPECT_TRUE(result1.isOk());
  EXPECT_TRUE(result1.data > 0);
  auto result2 = table.getNat(result1.data);
  EXPECT_TRUE(result2.isOk());
  EXPECT_EQ(result2.data->first, srcAddrSPtr);
  EXPECT_TRUE(result2.data->first->getV4Addr().sin_addr.s_addr == 0x01020304);
  EXPECT_TRUE(result2.data->first->getV4Addr().sin_port == 0x1234);
}