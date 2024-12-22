#include "flog.h"
#include "fsocketMPort.h"
using namespace Ferrum;

int main(int, char **) {
  FLog::info("starting application");
#ifdef FDEBUG
  FLog::info("setting log level to TRACE");
  FLog::setLevel(LogLevel::TRACE);
#endif

  std::shared_ptr<FConfig> config{new FConfig{}};
  auto result = config->loadConfig();
  if (result.isError()) {
    FLog::error("failed to load config: %s", result.message.c_str());
    exit(1);
  }

  std::cout << *config << std::endl;

  FSocketMPort socket{config};

  result = socket.init();
  if (result.isError()) {
    FLog::error("socket init error: %s", result.message.c_str());
    exit(1);
  }
  result = socket.start();
  if (result.isError()) {
    FLog::error("socket start error: %s", result.message.c_str());
    exit(1);
  }
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
