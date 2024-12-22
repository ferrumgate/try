#include "fsocketBase.h"

namespace Ferrum {

FSocketBase::FSocketBase(std::string name, size_t bufferCapacity)
    : name{name},
      loop(nullptr),
      initialized(false),
      buffer(new uint8_t[bufferCapacity]),
      bufferCapacity(bufferCapacity),
      bufferSize(0),
      socketReadFd(0),
      srcAddr(new FAddr(0, 0)),
      dstAddr(new FAddr(0, 0)) {
  loop = uv_default_loop();
}

FSocketBase::~FSocketBase() {
  close();
  delete[] buffer;
}

FResult<bool> FSocketBase::initSocket() {
  return FResult<bool>::Error(name + " please override this initSocket method");
}

const ::uv_os_fd_t FSocketBase::getSocketFd() const {
  return socketReadFd;
}

void handleRead(uv_poll_t *handle, int status, int events) {
  FSocketBase *socket = reinterpret_cast<FSocketBase *>(handle->data);
  if (status < 0) {
    FLog::error("%s socket poll error: %s", socket->name.c_str(), uv_strerror(status));
    return;
  }
  if (!status && events & UV_READABLE) {
    auto socketFd = socket->getSocketFd();
    uint8_t *buffer = socket->buffer;
    auto readedLen = recv(socketFd, buffer, socket->bufferCapacity, 0);
    if (readedLen < 0) {
      FLog::error("%s socket failed to read data: %s", socket->name.c_str(), strerror(errno));
      return;
    }
    if (readedLen == 0) {
      FLog::trace("%s socket no data to read", socket->name.c_str());
      return;
    }
    FLog::debug("***************************************");
    FLog::trace("%s socket readed %d bytes", socket->name.c_str(), readedLen);
    socket->bufferSize = readedLen;
    socket->onRead();
  }
  if (!status && events & UV_WRITABLE) {
    FLog::trace("%s socket is writable", socket->name.c_str());
  }
}
FResult<bool> FSocketBase::init() {
  if (initialized) {
    return FResult<bool>::Error(name + "socket is already initialized");
  }
  auto result = initSocket();
  if (result.isError()) {
    return result;
  }
  initialized = true;
  return FResult<bool>::Ok();
}

FResult<bool> FSocketBase::listen() {
  if (!initialized) {
    return FResult<bool>::Error(name + " socket is not initialized");
  }
  FLog::debug("%s socket poll init", name.c_str());
  //////////////////////////////////////////////////
  auto error = uv_poll_init_socket(uv_default_loop(), &poll, socketReadFd);
  if (error) {
    return FResult<bool>::Error(name + " failed to initialize poll: " + std::string(uv_strerror(error)));
  }
  poll.data = this;
  FLog::debug("%s socket poll start", name.c_str());
  error = uv_poll_start(&poll, UV_READABLE, handleRead);
  if (error) {
    return FResult<bool>::Error(name + "failed to poll start failed: " + std::string(uv_strerror(error)));
  }
  return FResult<bool>::Ok();
}

void FSocketBase::close() {
  if (initialized) {
    uv_poll_stop(&poll);
    uv_close((uv_handle_t *)&poll, [](uv_handle_t *handle) {
      FSocketBase *socket = reinterpret_cast<FSocketBase *>(handle->data);
      FLog::debug("%s poll closed", socket->name.c_str());
      // close socket if opened
      // TODO close socket
    });
  }
  initialized = false;
}
void FSocketBase::onRead() {
  if (onReadCallback) {
    onReadCallback(*this, onContext);
  }
}
void FSocketBase::onWrite() {
  if (onWriteCallback) {
    onWriteCallback(*this, onContext);
  }
}

void FSocketBase::on(void *context, OnReadCallback onRead, OnWriteCallback onWrite) {
  onContext = context;
  onReadCallback = onRead;
  onWriteCallback = onWrite;
}

}  // namespace Ferrum