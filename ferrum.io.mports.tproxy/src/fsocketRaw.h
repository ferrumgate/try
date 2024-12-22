#ifndef __FSOCKET_RAW_H__
#define __FSOCKET_RAW_H__

#include "fsocketBase.h"

namespace Ferrum {
class FSocketRaw : public FSocketBase {
 public:
  FSocketRaw(std::string name = "unknown", size_t bufferSize = 4096);
  virtual ~FSocketRaw() override;
  virtual FResult<bool> configure(const std::string &listenInterface, const std::string &listenIp,
                                  const std::string &forwardInterface, const std::string &forwardIp);
  virtual FResult<bool> write(uint8_t *buffer, size_t len, FAddr &srcAddr, FAddr &dstAddr);

 protected:
  virtual FResult<bool> initSocket() override;
  std::string listenInterface;
  FAddrSharedPtr listenInterfaceIp;
  unsigned char listenInterfaceHwAddr[6];
  uint8_t listenInterfaceIndex;
  std::string forwardInterface;
  FAddrSharedPtr forwardInterfaceIp;
  unsigned char forwardInterfaceHwAddr[6];
  uint8_t forwardInterfaceIndex;
  virtual FResult<bool> bindSocket();
  std::string name;
  ::uv_os_fd_t socketFdWrite;

 private:
  friend void onSocketRead(FSocketBase &socket, void *context);
};
}  // namespace Ferrum

#endif  // __FSOCKET_RAW_H__