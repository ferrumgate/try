#ifndef __FCACHE_H__
#define __FCACHE_H__

#include <chrono>
#include <map>
#include <memory>

#include "faddr.h"
#include "fcommon.h"
#include "futil.h"

namespace Ferrum {

/// @brief time base caching with pages
/// by this way, we can easily delete a timedout page in O(1) time
/// @tparam TKey
/// @tparam TValue
template <typename TKey, typename TValue>
class FCachePage {
 public:
  FCachePage(uint64_t expireTime) : _expireTime(expireTime), _cache() {}

  virtual ~FCachePage() {}

  uint64_t getExpireTime() const {
    return _expireTime;
  }

  FResult<bool> add(const TKey key, const TValue value) {
    _cache[key] = value;
    return FResult<bool>::Ok();
  }

  FResult<bool> remove(const TKey &key) {
    auto it = _cache.find(key);
    if (it != _cache.end()) {
      _cache.erase(it);
      return FResult<bool>::Ok();
    }
    return FResult<bool>::Error("key not found");
  }

  FResult<bool> isExists(const TKey &key) {
    auto it = _cache.find(key);
    if (it != _cache.end()) {
      return FResult<bool>::Ok();
    }
    return FResult<bool>::Error("key not found");
  }

  FResult<TValue> get(const TKey &key) {
    auto it = _cache.find(key);
    if (it != _cache.end()) {
      auto k = it->second;
      return FResult<TValue>::Ok(it->second);
    }
    return FResult<TValue>::Error("key not found");
  }

  size_t getSize() {
    return _cache.size();
  }

 protected:
  uint64_t _expireTime;
  std::map<TKey, TValue> _cache;
};

template <typename TKey, typename TValue>
class FCache {
 public:
  using CachePage = FCachePage<TKey, TValue>;
  using CacheNow = std::unique_ptr<FCachePage<TKey, TValue>>;
  using CacheFuture = std::unique_ptr<FCachePage<TKey, TValue>>;

 public:
  FCache(uint64_t timeoutMS = 5 * 60 * 1000) : _timeoutMS(timeoutMS), _nowPage(nullptr), _futurePage(nullptr) {
    init();
  }
  virtual ~FCache() {
    clear();
  }

  virtual void clear() {
    if (_nowPage) {
      _nowPage.reset();
    }
    if (_futurePage) {
      _futurePage.reset();
    }
  }
  virtual void init() {
    auto nowMS = FUtil::DateTime::now();
    auto nowStart = (nowMS / _timeoutMS) * _timeoutMS;
    if (!_nowPage) {
      _nowPage = std::make_unique<FCachePage<TKey, TValue>>(nowStart + _timeoutMS);
    }
    if (!_futurePage) {
      _futurePage = std::make_unique<FCachePage<TKey, TValue>>(nowStart + _timeoutMS + _timeoutMS);
    }
  }
  virtual FResult<bool> clearTimedOut() {
    auto nowMS = FUtil::DateTime::now();
    auto nowStart = (nowMS / _timeoutMS) * _timeoutMS;
    if (_nowPage && _nowPage->getExpireTime() < nowStart) {
      _nowPage.reset();
      _nowPage = std::move(_futurePage);
      _futurePage = nullptr;
    }
    init();
    return FResult<bool>::Ok();
  }

  virtual FResult<bool> isExists(const TKey &key) const {
    auto result = _nowPage->isExists(key);
    return result;
  }

  virtual FResult<TValue> get(const TKey &key) const {
    return _nowPage->get(key);
  }

  virtual FResult<bool> add(TKey key, TValue value) {
    _nowPage->add(key, value);
    _futurePage->add(key, value);
    return FResult<bool>::Ok();
  }

 protected:
  uint64_t _timeoutMS;
  CacheNow _nowPage;
  CacheFuture _futurePage;
  // static std::unique_ptr<FCache<TKey, TValue>> _instance;

 public:
  CachePage *getCacheNow() {
    return _nowPage.get();
  }
  CachePage *getCacheFuture() {
    return _futurePage.get();
  }
};
}  // namespace Ferrum

#endif  // __FCACHE_H__