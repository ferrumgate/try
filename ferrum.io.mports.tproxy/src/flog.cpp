#include "flog.h"
namespace Ferrum {

struct LogData {
  LogLevel level;
  LogData() : level(LogLevel::INFO) {}
};

static LogData logData{};

std::string FLog::format(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char buffer[1024];
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  return std::string(buffer);
}

#define format(fmt)                             \
  va_list args;                                 \
  va_start(args, fmt);                          \
  char buffer[1024];                            \
  vsnprintf(buffer, sizeof(buffer), fmt, args); \
  va_end(args);                                 \
  auto msg = std::string(buffer);

inline std::string timeNow() {
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);

  std::ostringstream oss;
  oss << std::put_time(&tm, "[%d-%m-%Y %H:%M:%S]");
  auto str = oss.str();
  return str;
}

void FLog::fatal(const char *fmt, ...) {
  if (logData.level >= LogLevel::FATAL) {
    format(fmt);
    std::cout << timeNow() << " [FATAL] " << msg << std::endl;
  }
}
void FLog::error(const char *fmt, ...) {
  if (logData.level >= LogLevel::ERROR) {
    format(fmt);
    std::cout << timeNow() << " [ERROR] " << msg << std::endl;
  }
}
void FLog::warn(const char *fmt, ...) {
  if (logData.level >= LogLevel::WARN) {
    format(fmt);
    std::cout << timeNow() << " [WARN] " << msg << std::endl;
  }
}
void FLog::info(const char *fmt, ...) {
  if (logData.level >= LogLevel::INFO) {
    format(fmt);
    std::cout << timeNow() << " [INFO] " << msg << std::endl;
  }
}

void FLog::debug(const char *fmt, ...) {
  if (logData.level >= LogLevel::DEBUG) {
    format(fmt);
    std::cout << timeNow() << " [DEBUG] " << msg << std::endl;
  }
}
void FLog::trace(const char *fmt, ...) {
  if (logData.level >= LogLevel::TRACE) {
    format(fmt);
    std::cout << timeNow() << " [TRACE] " << msg << std::endl;
  }
}

void FLog::setLevel(LogLevel level) {
  logData.level = level;
}

}  // namespace Ferrum