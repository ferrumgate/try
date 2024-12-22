#ifndef __FLOG_H__
#define __FLOG_H__

#include <cstdarg>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>

namespace Ferrum {

/// @brief LogLevel enum
enum class LogLevel { FATAL, ERROR, WARN, INFO, DEBUG, TRACE };

/// @brief Log class
/// @details This class is used to log messages to the console.
class FLog {
 public:
  static std::string format(const char *fmt, ...);
  static void setLevel(LogLevel level);
  static void fatal(const char *fmt, ...);
  static void error(const char *fmt, ...);
  static void warn(const char *fmt, ...);
  static void info(const char *fmt, ...);
  static void debug(const char *fmt, ...);
  static void trace(const char *fmt, ...);
};

}  // namespace Ferrum

#endif  // __LOG_H__