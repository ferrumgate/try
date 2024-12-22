#ifndef __FCONFIG_H__
#define __FCONFIG_H__

#include "fcommon.h"
namespace Ferrum {

class FConfig {
 public:
  FConfig();
  virtual ~FConfig();
  virtual std::string getEbpfPath();
  virtual bool getIsEbpfEnabledTcp();
  virtual bool getIsEbpfEnabledUdp();
  virtual std::vector<std::pair<uint16_t, uint16_t>> getListenTcpPorts();
  virtual std::vector<std::pair<uint16_t, uint16_t>> getListenUdpPorts();
  virtual std::string getForwardDstIp();
  virtual std::string getForwardSrcIp();
  virtual std::string getForwardInterface();

  virtual std::string getListenIp();
  virtual std::string getListenInterface();
  virtual FResult<bool> loadConfig();

 protected:
  std::string forwardSrcIp;
  std::string forwardDstIp;
  std::string forwardInterface;
  std::string listenIp;
  std::string listenInterface;
  std::string ebpfPath;
  bool isEbpfEnabledTcp;
  bool isEbpfEnabledUdp;
  std::vector<std::pair<uint16_t, uint16_t>> listenTcpPorts;
  std::vector<std::pair<uint16_t, uint16_t>> listenUdpPorts;

 protected:
  friend std::ostream& operator<<(std::ostream& os, const FConfig& config);
};

}  // namespace Ferrum

#endif  // __FCONFIG_H__