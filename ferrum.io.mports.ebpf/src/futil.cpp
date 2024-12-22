#include "futil.h"

namespace Ferrum {
FUtil::FUtil() = default;
FUtil::~FUtil() = default;
FUtil::DateTime::DateTime() = default;
FUtil::DateTime::~DateTime() = default;

uint64_t FUtil::DateTime::now() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
      .count();
}

}  // namespace Ferrum