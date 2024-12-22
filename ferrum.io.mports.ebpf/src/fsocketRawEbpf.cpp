#include "fsocketRawEbpf.h"
namespace Ferrum {
FSocketRawEbpf::FSocketRawEbpf(std::string name, size_t bufferSize) : FSocketRaw{name, bufferSize}, ebpfPath{} {}
FSocketRawEbpf::~FSocketRawEbpf() = default;
FResult<bool> FSocketRawEbpf::configure(const std::string& listenInterface, const std::string& listenIp,
                                        const std::string& forwardInterface, const std::string& forwardIp) {
  return FSocketRaw::configure(listenInterface, listenIp, forwardInterface, forwardIp);
}

FResult<bool> FSocketRawEbpf::initSocket() {
  auto result = FSocketRaw::initSocket();
  if (result.isError()) {
    return result;
  }

  return initEbpf();
}
FResult<bool> FSocketRawEbpf::configure(const std::string& ebpfPath) {
  this->ebpfPath = ebpfPath;
  return FResult<bool>::Ok();
}

FResult<bool> FSocketRawEbpf::initEbpf() {
  struct bpf_object* obj;
  int prog_fd, sock;
  struct bpf_program* prog;
  auto ebpfModulePath = ebpfPath.length() ? ebpfPath : "socket_ip_filter.o";
  struct bpf_prog_load_attr prog_load_attr = {
      .file = ebpfModulePath.c_str(),
      .prog_type = BPF_PROG_TYPE_SOCKET_FILTER,
  };
  // Check if the file exists
  if (access(prog_load_attr.file, F_OK) == -1) {
    return FResult<bool>::Error(name + " BPF object file not found: " + std::string(prog_load_attr.file));
  }

  if (bpf_prog_load_xattr(&prog_load_attr, &obj, &prog_fd)) {
    return FResult<bool>::Error(name + " failed to load attr BPF program: " + std::string(strerror(errno)));
  }

  prog = bpf_object__find_program_by_title(obj, "socket");
  if (!prog) {
    return FResult<bool>::Error(name + " failed to find BPF program: " + std::string(strerror(errno)));
  }

  if (setsockopt(socketReadFd, SOL_SOCKET, SO_ATTACH_BPF, &prog_fd, sizeof(prog_fd)) < 0) {
    return FResult<bool>::Error(name + " failed to attach BPF program: " + std::string(strerror(errno)));
  }

  FLog::debug("%s socket XDP program attached from %s", name.c_str(), ebpfModulePath.c_str());
  auto pos = ebpfModulePath.rfind("/");
  std::string mapPath{"/sys/fs/bpf/"};
  if (pos == std::string::npos && ebpfModulePath.length() > 2) {
    mapPath += ebpfModulePath.substr(0, ebpfModulePath.length() - 2);
  } else {
    mapPath += ebpfModulePath.substr(pos + 1, ebpfModulePath.length() - pos - 3);
  }
  mapPath += "_config";
  FLog::debug("%s socket BPF map path %s", name.c_str(), mapPath.c_str());
  int bpfMap = bpf_obj_get(mapPath.c_str());
  if (bpfMap < 0) {
    return FResult<bool>::Error(name + " failed to get BPF map: " + std::string(strerror(errno)));
  }
  uint32_t key = 0;
  uint32_t value = listenInterfaceIp->getV4Addr().sin_addr.s_addr;
  FLog::debug("%s socket BPF map updated listen ip %s %u", name.c_str(), listenInterfaceIp->toString().c_str(), value);
  int result = bpf_map_update_elem(bpfMap, &key, &value, 0);
  if (result < 0) {
    return FResult<bool>::Error(name + " failed to update BPF map: " + std::string(strerror(errno)));
  }

  key = 1;
  value = forwardInterfaceIp->getV4Addr().sin_addr.s_addr;
  FLog::debug("%s socket BPF map updated forward ip %s %u", name.c_str(), forwardInterfaceIp->toString().c_str(),
              value);
  result = bpf_map_update_elem(bpfMap, &key, &value, 0);
  if (result < 0) {
    return FResult<bool>::Error(name + " failed to update BPF map: " + std::string(strerror(errno)));
  }
  return FResult<bool>::Ok();
}

}  // namespace Ferrum