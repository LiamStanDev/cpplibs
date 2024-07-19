#pragma once

#include "process.h"

#include <cstdint>
#include <fstream>
#include <list>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#define LOG_STREAM(logger, level)                                              \
  if (logger->getLevel() <= level)                                             \
  LogEventTraker{                                                              \
      LogEvent::ptr{new LogEvent{logger, level, __FILE__, __LINE__, 0,         \
                                 getThreadId(), getFiberId(), std::time(0)}}}  \
      .getStream()

#define LOG_DEBUG(logger) LOG_STREAM(logger, LogLevel::DEBUG)
#define LOG_INFO(logger) LOG_STREAM(logger, LogLevel::INFO)
#define LOG_WARN(logger) LOG_STREAM(logger, LogLevel::WARN)
#define LOG_ERROR(logger) LOG_STREAM(logger, LogLevel::ERROR)
#define LOG_FATAL(logger) LOG_STREAM(logger, LogLevel::FATAL)

namespace cpplibs {
namespace log {

class Logger;
class LoggerAppender;
class LoggerFormatter;

enum class LogLevel {
  UNKNOWN = 0,
  DEBUG = 1,
  INFO = 2,
  WARN = 3,
  ERROR = 4,
  FATAL = 5,
};

class LogEvent {
public:
  using ptr = std::shared_ptr<LogEvent>;
  LogEvent(std::shared_ptr<Logger> logger, LogLevel level, const char* file,
           int32_t line, uint32_t uptime, int32_t threadId, uint32_t fiberId,
           int64_t time);

  const char* getFile() const { return m_file; }
  int32_t getLine() const { return m_line; }
  uint64_t getThreadId() const { return m_threadId; }
  uint32_t getFiberId() const { return m_fiberId; }
  uint64_t getTime() const { return m_time; }
  uint32_t getUptime() const { return m_uptime; }
  const std::string getContent() const { return m_stream.str(); }

  std::stringstream& getStream() { return m_stream; }
  const std::shared_ptr<Logger>& getLogger() const { return m_logger; }
  LogLevel getLevel() const { return m_level; }

private:
  const char* m_file = nullptr; // file name
  int32_t m_line = 0;           // total line number
  uint32_t m_threadId = 0;      // thread id
  uint32_t m_fiberId = 0;       // fiber id
  uint64_t m_time = 0;          // timestamp
  uint32_t m_uptime = 0;        // running time

  std::stringstream m_stream;
  std::shared_ptr<Logger> m_logger;
  LogLevel m_level;
};

class LogEventTraker {
public:
  LogEventTraker(LogEvent::ptr event);
  ~LogEventTraker();

  std::stringstream& getStream();

private:
  LogEvent::ptr m_event;
};

class LogFormatter {
public:
  using ptr = std::shared_ptr<LogFormatter>;
  LogFormatter(const std::string& pattern);

  std::string format(std::shared_ptr<Logger> logger, LogLevel level,
                     LogEvent::ptr event);
  void init();

public:
  class FormatItem {
  public:
    using ptr = std::shared_ptr<FormatItem>;

    virtual ~FormatItem() {};
    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger,
                        LogLevel level, LogEvent::ptr event) = 0;
  };

private:
  std::string m_pattern;
  std::vector<FormatItem::ptr> m_items;
  bool m_isError = false;
};

class LogAppender {
public:
  using ptr = std::shared_ptr<LogAppender>;
  virtual ~LogAppender() {}

  virtual void log(std::shared_ptr<Logger> logger, LogLevel level,
                   LogEvent::ptr event) = 0;
  void setFormatter(LogFormatter::ptr formatter) { m_formatter = formatter; }
  LogFormatter::ptr getFormatter() const { return m_formatter; }

protected:
  LogLevel m_level;
  LogFormatter::ptr m_formatter;
};

class Logger : public std::enable_shared_from_this<Logger> {
public:
  using ptr = std::shared_ptr<Logger>;
  Logger(const std::string& name = "root");

  void log(LogLevel level, LogEvent::ptr event);
  void debug(LogEvent::ptr event);
  void info(LogEvent::ptr event);
  void warn(LogEvent::ptr event);
  void error(LogEvent::ptr event);
  void fatal(LogEvent::ptr event);

  void addAppender(LogAppender::ptr appender);
  void delAppender(LogAppender::ptr appender);
  LogLevel getLevel() const { return m_level; }
  void setLevel(LogLevel level) { m_level = level; }
  const std::string& getName() { return m_name; }
  const LogFormatter::ptr getFormatter() const { return m_formatter; }

private:
  std::string m_name; // logger name
  LogLevel m_level;
  std::list<LogAppender::ptr> m_appenders;
  LogFormatter::ptr m_formatter;
};

class StdoutLogAppender : public LogAppender {
public:
  using ptr = std::shared_ptr<StdoutLogAppender>;

  void log(Logger::ptr logger, LogLevel level, LogEvent::ptr event) override;

private:
};

class FileLogAppender : public LogAppender {
public:
  using ptr = std::shared_ptr<FileLogAppender>;

  FileLogAppender(const std::string& filename);

  void log(Logger::ptr logger, LogLevel level, LogEvent::ptr event) override;

  bool reopen();

private:
  std::string m_filename;
  std::ofstream m_filestream;
};

} // namespace log
} // namespace cpplibs
