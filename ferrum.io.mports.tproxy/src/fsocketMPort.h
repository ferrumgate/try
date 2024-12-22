#ifndef __FSOCKET_TCP_H__
#define __FSOCKET_TCP_H__

#include "fcache.h"
#include "fnatTable.h"
#include "fsocketRawEbpf.h"
#include "fconfig.h"

namespace Ferrum {

class FSocketMPort {
 public:
  FSocketMPort(std::shared_ptr<FConfig> config);
  virtual ~FSocketMPort();
  virtual FResult<bool> init();
  virtual FResult<bool> start();
  virtual FResult<bool> stop();
  virtual FSocketRawEbpf &getSocket();
  virtual const std::shared_ptr<FConfig> &getConfig();

 protected:
  std::shared_ptr<FConfig> config;
  FSocketRawEbpf rawSocket;
  FAddrSharedPtr forwardAddr;
  FNatTable natTable;
  uint32_t lastId;

 protected:
  friend void onSocketRead(FSocketBase &socket, void *context);
};

}  // namespace Ferrum
#endif  // __FSOCKET_TCP_H__