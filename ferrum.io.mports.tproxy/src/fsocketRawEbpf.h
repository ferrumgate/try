#ifndef __FSOCKET_RAW_EBPF_H__
#define __FSOCKET_RAW_EBPF_H__

#include "fsocketRaw.h"
namespace Ferrum {
class FSocketRawEbpf : public FSocketRaw {
 public:
  FSocketRawEbpf(std::string name = "unknown", size_t bufferSize = 4096);
  virtual ~FSocketRawEbpf() override;
  FResult<bool> configure(const std::string& listenInterface, const std::string& listenIp,
                          const std::string& forwardInterface, const std::string& forwardIp) override;
  FResult<bool> configure(const std::string& ebpfPath);

 protected:
  virtual FResult<bool> initSocket() override;
  virtual FResult<bool> initEbpf();
  std::string ebpfPath;

 protected:
  friend void onSocketRead(FSocketBase& socket, void* context);
};
}  // namespace Ferrum

#endif  // __FSOCKET_RAW_EBPF_H__