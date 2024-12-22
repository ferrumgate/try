#include <gtest/gtest.h>

#include "futil.h"
using namespace Ferrum;
// Demonstrate some basic assertions.
TEST(TestFUtil, now) {
  auto now = FUtil::DateTime::now();
  ASSERT_TRUE(now > 0);
}