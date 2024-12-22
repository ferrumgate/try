#include "fsocketMPort.h"

namespace Ferrum {

FSocketMPort::FSocketMPort(std::shared_ptr<FConfig> config)
    : config(config), natTable{30000}, rawSocket{"ferrum"}, forwardAddr{new FAddr{0, 0}}, lastId{0} {}

FSocketMPort::~FSocketMPort() {}

FSocketRawEbpf &FSocketMPort::getSocket() {
  return rawSocket;
}

const std::shared_ptr<FConfig> &FSocketMPort::getConfig() {
  return config;
}

void onSocketRead(FSocketBase &socket, void *context) {
  auto *mport = reinterpret_cast<FSocketMPort *>(context);
  // auto &rawSocket = mport->getSocket();
  auto *buffer = socket.buffer;
  struct ethhdr *eth = (struct ethhdr *)buffer;
  uint8_t *offset = buffer + sizeof(struct ethhdr);
  struct iphdr *ip_packet = (struct iphdr *)offset;
  struct tcphdr *tcp_packet = (struct tcphdr *)(offset + sizeof(struct iphdr));

  auto srcAddr = FAddrSharedPtr{new FAddr{ip_packet->saddr, tcp_packet->source}};
  auto dstAddr = FAddrSharedPtr{new FAddr{ip_packet->daddr, tcp_packet->dest}};

  FLog::trace("%s onRead called", socket.name.c_str());
  FLog::trace("%s socket readed buf size: %d", socket.name.c_str(), socket.bufferSize);
  FLog::trace("%s eth src: %02x%02x%02x%02x%02x%02x dst: %02x%02x%02x%02x%02x%02x", socket.name.c_str(),
              eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4],
              eth->h_source[5], eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4],
              eth->h_dest[5]);
  FLog::debug("%s ip packet id: %d from %s to %s", socket.name.c_str(), ip_packet->id,
              srcAddr->toStringWithPort().c_str(), dstAddr->toStringWithPort().c_str());
  FLog::debug("%s tcp Packet Src Port: %d Dst Port: %d", socket.name.c_str(), ntohs(tcp_packet->source),
              ntohs(tcp_packet->dest));

  // if packet is coming from client
  // change the source address to our address
  // put it into the nat table
  if (dstAddr->getV4Addr().sin_addr.s_addr == mport->rawSocket.listenInterfaceIp->getV4Addr().sin_addr.s_addr) {
    // check if the packet destination port is the same as the listen ports
    // otherwise drop packet
    if (tcp_packet->source == htons(8080)) {
      return;
    }
    if (tcp_packet->source == htons(80)) {
      return;
    }
    if (ip_packet->id && mport->lastId == ip_packet->id) {
      return;
    }
    mport->lastId = ip_packet->id;

    FLog::debug("packet is coming from the client");
    uint16_t natPort = 0;
    auto natPortFResult = mport->natTable.getNat(srcAddr);
    if (natPortFResult.isError()) {
      FLog::debug("failed to get nat port: %s", natPortFResult.message.c_str());
      natPortFResult = mport->natTable.addNat(srcAddr, dstAddr);
      if (natPortFResult.isError()) {
        FLog::error("failed to add nat port: %s", natPortFResult.message.c_str());
        return;
      }
    }
    natPort = natPortFResult.data;
    FLog::debug("assigned nat port: %d", ntohs(natPort));
    mport->rawSocket.forwardInterfaceIp->getV4Addr().sin_port = natPort;
    mport->forwardAddr->getV4Addr().sin_port = dstAddr->getV4Addr().sin_port;
    auto result =
        mport->rawSocket.write(buffer, socket.bufferSize, *mport->rawSocket.forwardInterfaceIp, *mport->forwardAddr);
    if (result.isError()) {
      FLog::error("failed to write to forward socket: %s", result.message.c_str());
    }
  } else if (dstAddr->getV4Addr().sin_addr.s_addr == mport->rawSocket.forwardInterfaceIp->getV4Addr().sin_addr.s_addr) {
    FLog::debug("packet is coming from the forward");
    auto natResult = mport->natTable.getNat(dstAddr->getV4Addr().sin_port);
    if (natResult.isError()) {
      FLog::error("failed to get nat port: %s", natResult.message.c_str());
      return;
    }
    FLog::debug("found nat port: %s", natResult.data->first->toStringWithPort().c_str());
    auto result = mport->rawSocket.write(buffer, socket.bufferSize, *natResult.data->second, *natResult.data->first);
    if (result.isError()) {
      FLog::error("failed to write to listen socket: %s", result.message.c_str());
    }
  }
}

FResult<bool> FSocketMPort::init() {
  auto result = config->loadConfig();
  if (result.isError()) {
    return result;
  }

  auto resultAddr = FAddr::from(config->getForwardDstIp(), 0);
  if (resultAddr.isError()) {
    return FResult<bool>::Error(std::string(resultAddr.message));
  }

  *forwardAddr = resultAddr.data;

  result = rawSocket.configure(config->getListenInterface(), config->getListenIp(), config->getForwardInterface(),
                               config->getForwardSrcIp());
  if (result.isError()) {
    return result;
  }
  result = rawSocket.configure(config->getEbpfPath());
  if (result.isError()) {
    return result;
  }

  rawSocket.on(this, onSocketRead, nullptr);

  result = rawSocket.init();
  if (result.isError()) {
    return result;
  }

  return FResult<bool>::Ok();
}

FResult<bool> FSocketMPort::start() {
  auto result = rawSocket.listen();
  if (result.isError()) {
    return result;
  }
  return FResult<bool>::Ok();
}

FResult<bool> FSocketMPort::stop() {
  rawSocket.close();
  return FResult<bool>::Ok();
}

}  // namespace Ferrum
