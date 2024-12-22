#ifndef __FSOCKET_BASE_H__
#define __FSOCKET_BASE_H__

#include <unordered_map>

#include "faddr.h"
#include "fcommon.h"
#include "flog.h"
#include "fnet.h"

namespace Ferrum {

class FSocketBase {
 public:
  FSocketBase(std::string name = "unknown", size_t bufferCapacity = std::numeric_limits<uint16_t>::max());
  virtual ~FSocketBase();
  virtual FResult<bool> init();
  virtual FResult<bool> listen();
  virtual void close();

 public:  // function pointers for callbacks
  typedef void (*OnReadCallback)(FSocketBase &socket, void *context);
  typedef void (*OnWriteCallback)(FSocketBase &socket, void *context);
  virtual void on(void *context, OnReadCallback onRead, OnWriteCallback onWrite);

 protected:
  virtual FResult<bool> initSocket();
  virtual const ::uv_os_fd_t getSocketFd() const;
  virtual void onRead();
  virtual void onWrite();

 protected:
  ::uv_poll_t poll;
  ::uv_loop_t *loop;
  ::uv_os_fd_t socketReadFd;
  bool initialized;
  std::string name;

 protected:
  uint8_t *buffer;
  size_t bufferSize;
  size_t bufferCapacity;
  FAddrSharedPtr srcAddr;
  FAddrSharedPtr dstAddr;
  void *onContext;
  OnReadCallback onReadCallback;
  OnWriteCallback onWriteCallback;

 private:
  friend void handleRead(uv_poll_t *handle, int status, int events);
  friend void onSocketRead(FSocketBase &socket, void *context);
};

}  // namespace Ferrum

#endif  // __FSOCKET_BASE_H__