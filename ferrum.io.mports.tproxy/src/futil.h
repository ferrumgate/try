#ifndef __FUTIL_H__
#define __FUTIL_H__

#include <chrono>

#include "fcommon.h"

namespace Ferrum {
class FUtil {
 public:
  FUtil(const FUtil &other) = delete;
  FUtil &operator=(const FUtil &other) = delete;

 private:
  FUtil();
  ~FUtil();

 public:
  class DateTime {
   public:
    static uint64_t now();

   private:
    DateTime();
    ~DateTime();
  };
};
}  // namespace Ferrum

#endif  // __FUTIL_H__