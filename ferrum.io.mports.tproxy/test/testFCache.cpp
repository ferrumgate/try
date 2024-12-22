#include <gtest/gtest.h>

#include "fcache.h"

using namespace Ferrum;
TEST(TestFCachePage, getExpireTime) {
  FCachePage<int, int> cachePage{1000};
  EXPECT_EQ(cachePage.getExpireTime(), 1000);
}
TEST(TestFCachePage, add_remove_get_isExists) {
  // check if add, remove, get and isExists are working correctly
  FCachePage<int, int> cachePage{1000};
  cachePage.add(1, 100);
  int search = 1;
  EXPECT_TRUE(cachePage.isExists(search).isOk());
  EXPECT_EQ(cachePage.getSize(), 1);
  EXPECT_TRUE(cachePage.remove(search).isOk());
  EXPECT_FALSE(cachePage.isExists(search).isOk());
  EXPECT_EQ(cachePage.getSize(), 0);
}

TEST(TestFCachePage, isExists) {
  // check if isExists is working correctly, adding a key and checking if it
  FCache<int, int> cache{10};
  const int search = 1;
  auto result = cache.isExists(search);
  EXPECT_TRUE(result.isError());
  int add = search;
  auto result2 = cache.add(add, 100);
  EXPECT_TRUE(result2.isOk());
  auto result3 = cache.isExists(search);
  EXPECT_TRUE(result3.isOk());
}

TEST(TestFCachePage, get_add) {
  // check if get is working correctly, adding a key and checking if it
  // exists
  FCache<int, int> cache{10};
  int search = 1;
  auto result1 = cache.get(search);
  EXPECT_TRUE(result1.isError());
  auto result2 = cache.add(search, 100);
  EXPECT_TRUE(result2.isOk());
  auto result3 = cache.isExists(search);
  EXPECT_TRUE(result3.isOk());
}

TEST(TestFCachePage, add_Internal) {
  // check now and future cache implementation
  FCache<int, int> cache{10};

  auto *nowPtr = cache.getCacheNow();
  EXPECT_TRUE(nowPtr != nullptr);
  auto *futurePtr = cache.getCacheFuture();
  EXPECT_TRUE(futurePtr != nullptr);

  int search = 1;
  auto result2 = cache.add(search, 100);
  nowPtr = cache.getCacheNow();
  futurePtr = cache.getCacheFuture();
  EXPECT_TRUE(nowPtr != nullptr);
  EXPECT_TRUE(futurePtr != nullptr);
  auto nowFResult = nowPtr->get(search);
  EXPECT_TRUE(nowFResult.isOk());
  auto futureFResult = futurePtr->get(search);
  EXPECT_TRUE(futureFResult.isOk());
}