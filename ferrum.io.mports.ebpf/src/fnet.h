#ifndef __FNET_H__
#define __FNET_H__

#include "fcommon.h"

namespace Ferrum {
class FNet {
 private:
  FNet();
  ~FNet();

 public:
  static uint16_t checksum(uint16_t *addr, uint32_t byteCount);
  static uint16_t ipChecksum(struct iphdr *iphdr);
  static uint16_t tcpChecksum(struct iphdr *iphdr, struct tcphdr *tcphdrp);
  static uint16_t udpChecksum(struct iphdr *iphdr, struct udphdr *udphdrp);
};

}  // namespace Ferrum

#endif  // __NET_H__
