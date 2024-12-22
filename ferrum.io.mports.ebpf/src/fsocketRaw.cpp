#include "fsocketRaw.h"
namespace Ferrum {
FSocketRaw::FSocketRaw(std::string name, size_t bufferSize)
    : FSocketBase{name, bufferSize},
      listenInterface{""},
      listenInterfaceIp{new FAddr{0, 0}},
      listenInterfaceHwAddr{0, 0, 0, 0, 0, 0},
      listenInterfaceIndex{0},
      forwardInterface{""},
      forwardInterfaceIp{new FAddr{0, 0}},
      forwardInterfaceHwAddr{0, 0, 0, 0, 0, 0},
      forwardInterfaceIndex{0},
      name(name),
      socketFdWrite{0} {}
FSocketRaw::~FSocketRaw() = default;

FResult<bool> FSocketRaw::configure(const std::string &listenInterface, const std::string &listenIp,
                                    const std::string &forwardInterface, const std::string &forwardIp) {
  // listen interface
  this->listenInterface = listenInterface;
  FLog::debug("%s socket listen interface: %s", name.c_str(), listenInterface.c_str());
  auto result = FAddr::from(listenIp, 0);
  if (result.isError()) {
    return FResult<bool>::Error(std::string(result.message));
  }
  this->listenInterfaceIp = new FAddr{result.data};
  FLog::debug("%s socket listen interface ip: %s", name.c_str(), listenInterfaceIp->toString().c_str());
  // forward interface
  this->forwardInterface = forwardInterface;
  FLog::debug("%s socket forward interface: %s", name.c_str(), forwardInterface.c_str());
  result = FAddr::from(forwardIp, 0);
  if (result.isError()) {
    return FResult<bool>::Error(std::string(result.message));
  }
  this->forwardInterfaceIp = new FAddr{result.data};
  FLog::debug("%s socket forward interface ip: %s", name.c_str(), this->forwardInterfaceIp->toString().c_str());

  return FResult<bool>::Ok();
}

FResult<bool> FSocketRaw::bindSocket() {
  // listen interface
  std::string device = listenInterface;
  ifreq interface;
  std::memset(&interface, 0, sizeof(interface));
  std::strncpy(interface.ifr_name, device.c_str(), IFNAMSIZ);

  auto error = ioctl(socketReadFd, SIOCGIFHWADDR, &interface);
  if (error < 0) {
    return FResult<bool>::Error(name + " failed to get hw address of interface " + std::string(strerror(errno)));
  }

  std::memcpy(listenInterfaceHwAddr, interface.ifr_hwaddr.sa_data, 6);
  error = ioctl(socketReadFd, SIOCGIFINDEX, &interface);
  if (error) {
    return FResult<bool>::Error(name + " failed to get interface id " + std::string(strerror(errno)));
  }
  uint8_t ifIndex = interface.ifr_ifindex;
  FLog::debug("%s socket interface index: %d of %s, hw: %02x:%02x:%02x:%02x:%02x:%02x", name.c_str(), ifIndex,
              interface.ifr_name, listenInterfaceHwAddr[0], listenInterfaceHwAddr[1], listenInterfaceHwAddr[2],
              listenInterfaceHwAddr[3], listenInterfaceHwAddr[4], listenInterfaceHwAddr[5]);
  listenInterfaceIndex = ifIndex;

  // forward interface
  device = forwardInterface;
  std::memset(&interface, 0, sizeof(interface));
  std::strncpy(interface.ifr_name, device.c_str(), IFNAMSIZ);

  error = ioctl(socketReadFd, SIOCGIFHWADDR, &interface);
  if (error < 0) {
    return FResult<bool>::Error(name + " failed to get hw address of interface " + std::string(strerror(errno)));
  }

  std::memcpy(forwardInterfaceHwAddr, interface.ifr_hwaddr.sa_data, 6);
  error = ioctl(socketReadFd, SIOCGIFINDEX, &interface);
  if (error) {
    return FResult<bool>::Error(name + " failed to get interface id " + std::string(strerror(errno)));
  }
  ifIndex = interface.ifr_ifindex;
  FLog::debug("%s socket interface index: %d of %s, hw: %02x:%02x:%02x:%02x:%02x:%02x", name.c_str(), ifIndex,
              interface.ifr_name, forwardInterfaceHwAddr[0], forwardInterfaceHwAddr[1], forwardInterfaceHwAddr[2],
              forwardInterfaceHwAddr[3], forwardInterfaceHwAddr[4], forwardInterfaceHwAddr[5]);
  forwardInterfaceIndex = ifIndex;

  return FResult<bool>::Ok();
}

FResult<bool> FSocketRaw::initSocket() {
  socketReadFd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  // socketReadFd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
  // socketReadFd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (socketReadFd < 0) {
    return FResult<bool>::Error(name + " failed to create socket");
  }
  FLog::trace("socket created: %d", socketReadFd);
  auto socketStatus = fcntl(socketReadFd, F_GETFL);
  auto error = fcntl(socketReadFd, F_SETFL, socketStatus | O_NONBLOCK);
  if (error) {
    ::close(socketReadFd);
    return FResult<bool>::Error(name + " failed to set socket to non-blocking " + std::string(strerror(errno)));
  }
  auto result = bindSocket();
  if (result.isError()) {
    ::close(socketReadFd);
    return result;
  }

  struct sockaddr_ll sll;
  std::memset(&sll, 0, sizeof(sll));
  sll.sll_family = AF_PACKET;
  sll.sll_ifindex = listenInterfaceIndex;
  sll.sll_protocol = htons(ETH_P_ALL);

  // error = bind(socketReadFd, reinterpret_cast<struct sockaddr *>(&sll), sizeof(sll));
  if (error < 0) {
    ::close(socketReadFd);
    return FResult<bool>::Error(name + " failed to bind read socket to interface " + std::string(strerror(errno)));
  }

  // create write socket
  socketFdWrite = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
  if (socketFdWrite < 0) {
    ::close(socketReadFd);
    return FResult<bool>::Error(name + " failed to create write socket");
  }
  FLog::trace("write socket created: %d", socketFdWrite);
  const int opt_on = 1;
  auto rc = setsockopt(socketFdWrite, IPPROTO_IP, IP_HDRINCL, &opt_on, sizeof(opt_on));
  if (rc < 0) {
    ::close(socketReadFd);
    ::close(socketFdWrite);
    return FResult<bool>::Error(name + " failed to set IP_HDRINCL " + std::string(strerror(errno)));
  }

  struct ifreq ifr;
  std::memset(&ifr, 0, sizeof(ifr));
  std::strncpy(ifr.ifr_name, forwardInterface.c_str(), IFNAMSIZ);

  // rc = setsockopt(socketFdWrite, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr));
  if (rc < 0) {
    ::close(socketReadFd);
    ::close(socketFdWrite);
    return FResult<bool>::Error(name + " failed to bind write socket to lo interface " + std::string(strerror(errno)));
  }

  return FResult<bool>::Ok();
}

FResult<bool> FSocketRaw::write(uint8_t *buffer, size_t len, FAddr &srcAddr, FAddr &dstAddr) {
  uint8_t *offset = buffer + sizeof(struct ethhdr);
  struct iphdr *ip_packet = (struct iphdr *)offset;
  struct tcphdr *tcp_packet = (struct tcphdr *)(offset + sizeof(struct iphdr));
  //  change the source address to our address
  ip_packet->saddr = srcAddr.getV4Addr().sin_addr.s_addr;
  tcp_packet->source = srcAddr.getV4Addr().sin_port;
  // change destination
  ip_packet->daddr = dstAddr.getV4Addr().sin_addr.s_addr;
  tcp_packet->dest = dstAddr.getV4Addr().sin_port;
  // recalculate checksum
  ip_packet->check = FNet::ipChecksum(ip_packet);
  tcp_packet->check = FNet::tcpChecksum(ip_packet, tcp_packet);
  FLog::debug("%s socket will sent packet to %s with source %s packet size: %d", name.c_str(),
              dstAddr.toStringWithPort().c_str(), srcAddr.toStringWithPort().c_str(), len);
  auto sendedSize = sendto(socketFdWrite, buffer + sizeof(struct ethhdr), len - sizeof(struct ethhdr), 0,
                           reinterpret_cast<const struct sockaddr *>(&dstAddr.getV4Addr()), sizeof(struct sockaddr_in));
  if (sendedSize < 0) {
    FLog::error("%s socket failed to send data: %s", name.c_str(), strerror(errno));
    return FResult<bool>::Error(name + " failed to send data: " + std::string(strerror(errno)));
  }
  FLog::debug("%s socket send packet to %s", name.c_str(), dstAddr.toStringWithPort().c_str());
  return FResult<bool>::Ok();
}

}  // namespace Ferrum