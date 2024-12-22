#include <gtest/gtest.h>

#include "flog.h"
namespace Ferrum {

TEST(TestFLog, info) {
  FLog::info("hello");
}
TEST(TestFLog, debug) {
  FLog::debug("hello");
}
TEST(TestFLog, set_level) {
  FLog::setLevel(LogLevel::DEBUG);
}

}  // namespace Ferrum