#ifndef __FCOMMON_H__
#define __FCOMMON_H__

#include <errno.h>
#include <fcntl.h>
#include <linux/if_packet.h>
#include <linux/udp.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <uv.h>
#include <bpf/bpf.h>        // For bpf_prog_load
#include <linux/if_link.h>  // For XDP-related constants
#include <bpf/libbpf.h>     // Add this line to include the header for bpf_prog_load
#include <linux/bpf.h>

#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <utility>

namespace Ferrum {

enum class RCode : uint8_t {
  OK = 0,
  ERROR = 1,
};

template <typename T>
struct FResult {
  RCode code;
  std::string message;
  T data;
  bool isOk() {
    return code == RCode::OK;
  }
  bool isError() {
    return code == RCode::ERROR;
  }

  static struct FResult<T> Ok() {
    return {RCode::OK, std::string{""}, T{}};
  }

  static struct FResult<T>
  Ok(T data) {
    return {RCode::OK, std::string(""), data};
  }

  static struct FResult<T>
  Ok(T &&data) {
    return {RCode::OK, std::string(""), data};
  }

  static struct FResult<T>
  Error(const std::string &&message) {
    return {RCode::ERROR, message, T{}};
  }
};

template <typename T>
class FSharedPtr : public std::shared_ptr<T> {
 public:
  FSharedPtr() : std::shared_ptr<T>() {}
  FSharedPtr(T *ptr) : std::shared_ptr<T>(ptr) {}
  FSharedPtr(const std::shared_ptr<T> &ptr) : std::shared_ptr<T>(ptr) {}
  FSharedPtr(const FSharedPtr<T> &ptr) : std::shared_ptr<T>(ptr) {}
  bool operator==(const FSharedPtr<T> &other) const {
    return *(this->get()) == *(other.get());
  }
  bool operator<(const FSharedPtr<T> &other) const {
    return *(this->get()) < *(other.get());
  }
};

}  // namespace Ferrum
#endif  // __LIB_H__