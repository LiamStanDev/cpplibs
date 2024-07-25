#pragma once

#include "process.h"

#include <cstdint>
#include <ctime>
#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#define LOG_DEBUG(logger)                                                      \
  cosmic::log_tracker(logger, cosmic::LogLevel::DEBUG).getStream()
#define LOG_INFO(logger)                                                       \
  cosmic::log_tracker(logger, cosmic::LogLevel::INFO).getStream()
#define LOG_WARN(logger)                                                       \
  cosmic::log_tracker(logger, cosmic::LogLevel::WARN).getStream()
#define LOG_ERROR(logger)                                                      \
  cosmic::log_tracker(logger, cosmic::LogLevel::ERROR).getStream()
#define LOG_FATAL(logger)                                                      \
  cosmic::log_tracker(logger, cosmic::LogLevel::FATAL).getStream()

#define ROOT_LOGGER() *cosmic::LoggerManager::GetInstance()->getRoot()

namespace cosmic {
class Logger;
class LogAppender;

enum class LogLevel {
  UNKNOWN = 0,
  DEBUG = 1,
  INFO = 2,
  WARN = 3,
  ERROR = 4,
  FATAL = 5,
};

/**
 * @brief Include the log detail information.
 */
class LogEvent {
public:
  using ptr = std::shared_ptr<LogEvent>;
  LogEvent(LogLevel level, const char* file, int32_t line, uint32_t uptime,
           int32_t threadId, uint32_t fiberId, int64_t time);

  LogLevel getLevel() const { return m_level; }
  const char* getFile() const { return m_file; }
  int32_t getLine() const { return m_line; }
  uint64_t getThreadId() const { return m_threadId; }
  uint32_t getFiberId() const { return m_fiberId; }
  uint64_t getTime() const { return m_time; }
  uint32_t getUptime() const { return m_uptime; }
  const std::string getContent() const { return m_stream.str(); }

  std::stringstream& getStream() { return m_stream; }

private:
  LogLevel m_level;
  const char* m_file = nullptr; // file name
  int32_t m_line = 0;           // total line number
  uint32_t m_threadId = 0;      // thread id
  uint32_t m_fiberId = 0;       // fiber id
  uint64_t m_time = 0;          // timestamp
  uint32_t m_uptime = 0;        // running time
  std::stringstream m_stream;   // content
};

/**
 * @brief Track the lifetime of LogEvent. In order to commit log when LogEvent
 * drop.
 */
class LogEventTracker {
public:
  LogEventTracker(LogEvent::ptr event, const Logger& logger);
  ~LogEventTracker();
  std::stringstream& getStream() { return m_event->getStream(); }

private:
  std::shared_ptr<LogEvent> m_event;
  const Logger& m_logger;
};

inline LogEventTracker log_tracker(const Logger& logger, LogLevel level) {
  return LogEventTracker{
      std::shared_ptr<LogEvent>{
          new LogEvent{level, __FILE__, __LINE__, 0, GetProcessId(),
                       GetFiberId(), std::time(0)},
      },
      logger};
}

/**
 * @brief Manage log output (appender) and commit log.
 */
class Logger {
public:
  Logger(const std::string& name = "root");

  void log(LogEvent::ptr event) const;

  void addAppender(std::shared_ptr<LogAppender> appender);
  void delAppender(std::shared_ptr<LogAppender> appender);
  const std::string& getName() const { return m_name; }

private:
  std::string m_name; // logger name
  std::list<std::shared_ptr<LogAppender>> m_appenders;
};

/**
 * @brief Format the log pattern.
 */
class LogFormatter {
public:
  LogFormatter(const std::string& pattern =
                   "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T(%c)%T<%f:%l>%T%m%n");

  std::string format(const Logger& logger, std::shared_ptr<LogEvent> event);

  const std::string& getPattern() { return m_pattern; }
  void setPattern(const std::string& pattern);

private:
  void init();

public:
  class FormatItem {
  public:
    virtual void format(std::ostream& os, const Logger& logger,
                        LogEvent::ptr event) = 0;
    virtual ~FormatItem() = default;
  };

private:
  std::string m_pattern;
  std::vector<std::unique_ptr<FormatItem>> m_items;
};

/**
 * @brief The base class for the sink of log output.
 */
class LogAppender {
public:
  explicit LogAppender(LogLevel level);
  LogAppender(std::unique_ptr<LogFormatter> formatter, LogLevel level);
  virtual ~LogAppender() = default;
  virtual void log(const Logger& logger, std::shared_ptr<LogEvent> event) = 0;

  void setFormatter(std::unique_ptr<LogFormatter> formatter) {
    m_formatter = std::move(formatter);
  }
  const LogFormatter* getFormatter() const { return m_formatter.get(); }
  void setFormatter(const std::string& pattern);
  LogLevel getLogLevel() const { return m_level; }
  void setLogLevel(LogLevel level) { m_level = level; }

protected:
  std::unique_ptr<LogFormatter> m_formatter;
  LogLevel m_level;
};

/**
 * @brief standard log output implementation.
 */
class StdoutLogAppender : public LogAppender {
public:
  StdoutLogAppender(std::unique_ptr<LogFormatter> formatter, LogLevel level);
  StdoutLogAppender(LogLevel level);
  StdoutLogAppender();
  virtual void log(const Logger& logger,
                   std::shared_ptr<LogEvent> event) override;
};

/**
 * @brief file log output implementation.
 */
class FileLogAppender : public LogAppender {
public:
  FileLogAppender(const std::string& filename,
                  std::unique_ptr<LogFormatter> formatter, LogLevel level);

  FileLogAppender(const std::string& filename, LogLevel level);
  FileLogAppender(const std::string& filename);
  ~FileLogAppender();

  virtual void log(const Logger& logger,
                   std::shared_ptr<LogEvent> event) override;

  bool reopen();

private:
  std::string m_filename;
  std::ofstream m_filestream;
};

class LoggerManager {
public:
  static std::shared_ptr<LoggerManager> GetInstance();
  std::shared_ptr<Logger> getLogger(const std::string& name);
  std::shared_ptr<Logger> getRoot() { return m_root; }

private:
  LoggerManager();

private:
  std::map<std::string, std::shared_ptr<Logger>> m_loggers;
  std::shared_ptr<Logger> m_root;

  static std::shared_ptr<LoggerManager> s_instance;
};
} // namespace cosmic
