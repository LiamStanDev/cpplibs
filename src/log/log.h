#pragma once

#include <cstdint>
#include <fstream>
#include <list>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace cpplibs {
namespace logger {

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
  LogEvent();

  const char* getFile() const { return m_file; }
  int32_t getLine() const { return m_line; }
  uint32_t getThreadId() const { return m_threadId; }
  uint32_t getFiberId() const { return m_fiberId; }
  uint64_t getTime() const { return m_time; }
  uint32_t getUptime() const { return m_uptime; }
  const std::string& getContent() const { return m_content; }

private:
  const char* m_file = nullptr; // file name
  int32_t m_line = 0;           // total line number
  uint32_t m_threadId = 0;      // thread id
  uint32_t m_fiberId = 0;       // fiber id
  uint64_t m_time = 0;          // timestamp
  uint32_t m_uptime = 0;        // running time
  std::string m_content;
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

private:
  std::string m_name; // logger name
  LogLevel m_level;
  std::list<LogAppender::ptr> m_appenders;
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

} // namespace logger
} // namespace cpplibs
