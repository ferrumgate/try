#include "fconfig.h"
namespace Ferrum {
FConfig::FConfig()
    : forwardSrcIp{""},
      forwardDstIp{""},
      forwardInterface{""},
      listenIp{""},
      listenInterface{""},
      ebpfPath{""},
      isEbpfEnabledTcp{false},
      isEbpfEnabledUdp{false},
      listenTcpPorts{},
      listenUdpPorts{} {}

FConfig::~FConfig() = default;

std::string FConfig::getEbpfPath() {
  return ebpfPath;
}

bool FConfig::getIsEbpfEnabledTcp() {
  return isEbpfEnabledTcp;
}

bool FConfig::getIsEbpfEnabledUdp() {
  return isEbpfEnabledUdp;
}

std::vector<std::pair<uint16_t, uint16_t>> FConfig::getListenTcpPorts() {
  return listenTcpPorts;
}

std::vector<std::pair<uint16_t, uint16_t>> FConfig::getListenUdpPorts() {
  return listenUdpPorts;
}

std::string FConfig::getForwardSrcIp() {
  return forwardSrcIp;
}
std::string FConfig::getForwardDstIp() {
  return forwardDstIp;
}

std::string FConfig::getForwardInterface() {
  return forwardInterface;
}

std::string FConfig::getListenIp() {
  return listenIp;
}

std::string FConfig::getListenInterface() {
  return listenInterface;
}

using PortList = std::vector<std::pair<uint16_t, uint16_t>>;

std::pair<uint16_t, uint16_t> parsePort(const std::string& port) {
  std::pair<uint16_t, uint16_t> result;
  size_t pos = port.find("-");
  if (pos == std::string::npos) {
    result.first = 0;
    result.second = 0;
    return result;
  }
  result.first = std::stoi(port.substr(0, pos));
  result.second = std::stoi(port.substr(pos + 1));
  return result;
}

PortList parsePorts(const std::string& ports) {
  std::vector<std::string> tokens;
  size_t posEnd = 0;
  std::string token;
  size_t posStart = 0;
  while ((posEnd = ports.find(";", posStart)) != std::string::npos) {
    token = ports.substr(posStart, posEnd);
    posStart = posEnd + 1;
    tokens.push_back(token);
  }
  token = ports.substr(posStart);
  if (!token.empty()) {
    tokens.push_back(token);
  }

  PortList result;
  for (const auto& port : tokens) {
    result.push_back(parsePort(port));
  }
  return result;
}

FResult<bool> FConfig::loadConfig() {
  const char* envEbpfPath = std::getenv("EBPF_PATH");
  if (envEbpfPath == nullptr) {
    envEbpfPath = "socket_ip_filter.o";
  }
  ebpfPath = envEbpfPath;

  const char* envIsEbpfEnabledTcp = std::getenv("IS_EBPF_ENABLED_TCP");
  if (envIsEbpfEnabledTcp == nullptr) {
    envIsEbpfEnabledTcp = "TRUE";
  }
  if (envIsEbpfEnabledTcp == "TRUE" || envIsEbpfEnabledTcp == "true") {
    isEbpfEnabledTcp = true;
  } else {
    isEbpfEnabledTcp = false;
  }

  const char* envIsEbpfEnabledUdp = std::getenv("IS_EBPF_ENABLED_UDP");
  if (envIsEbpfEnabledUdp == nullptr) {
    envIsEbpfEnabledUdp = "TRUE";
  }
  if (envIsEbpfEnabledUdp == "TRUE" || envIsEbpfEnabledUdp == "true") {
    isEbpfEnabledUdp = true;
  } else {
    isEbpfEnabledUdp = false;
  }

  const char* envForwardSrcIp = std::getenv("FORWARD_SRC_IP");
  if (envForwardSrcIp == nullptr) {
    envForwardSrcIp = "0.0.0.0";
  }
  forwardSrcIp = envForwardSrcIp;

  const char* envForwardDstIp = std::getenv("FORWARD_DST_IP");
  if (envForwardDstIp == nullptr) {
    envForwardDstIp = "0.0.0.0";
  }
  forwardDstIp = envForwardDstIp;

  const char* envForwardInterface = std::getenv("FORWARD_INTERFACE");
  if (envForwardInterface == nullptr) {
    envForwardInterface = "lo";
  }
  forwardInterface = envForwardInterface;

  const char* envListenIp = std::getenv("LISTEN_IP");
  if (envListenIp == nullptr) {
    envListenIp = "0.0.0.0";
  }
  listenIp = envListenIp;

  const char* envListenInterface = std::getenv("LISTEN_INTERFACE");
  if (envListenInterface == nullptr) {
    envListenInterface = "lo";
  }
  listenInterface = envListenInterface;

  const char* envListenTcpPorts = std::getenv("LISTEN_TCP_PORTS");
  if (envListenTcpPorts == nullptr) {
    envListenTcpPorts = "0-65535";
  }
  listenTcpPorts = parsePorts(envListenTcpPorts);

  const char* envListenUdpPorts = std::getenv("LISTEN_UDP_PORTS");
  if (envListenUdpPorts == nullptr) {
    envListenUdpPorts = "0-65535";
  }
  listenUdpPorts = parsePorts(envListenUdpPorts);

  return FResult<bool>::Ok();
}

std::ostream& operator<<(std::ostream& os, const FConfig& config) {
  os << "config is " << std::endl;
  os << "ebpfPath: " << config.ebpfPath << std::endl;
  os << "isEbpfEnabledTcp: " << config.isEbpfEnabledTcp << std::endl;
  os << "isEbpfEnabledUdp: " << config.isEbpfEnabledUdp << std::endl;
  os << "listenIp: " << config.listenIp << std::endl;
  os << "listenInterface: " << config.listenInterface << std::endl;
  os << "listenTcpPorts: ";
  for (auto port : config.listenTcpPorts) {
    os << port.first << "-" << port.second << "; ";
  }
  os << std::endl;
  os << "listenUdpPorts: ";
  for (auto port : config.listenUdpPorts) {
    os << port.first << "-" << port.second << "; ";
  }
  os << std::endl;

  os << "forwardSrcIp: " << config.forwardSrcIp << std::endl;
  os << "forwardDstIp: " << config.forwardDstIp << std::endl;
  os << "forwardInterface: " << config.forwardInterface << std::endl;
  return os;
}

}  // namespace Ferrum