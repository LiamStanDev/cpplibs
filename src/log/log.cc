#include "log.h"

#include <cctype>
#include <ctime>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>

namespace cpplibs {
namespace log {

const char* stringifyLogLevel(LogLevel level) {
  switch (level) {
#define XX(name)                                                               \
  case LogLevel::name:                                                         \
    return #name;                                                              \
    break;

    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
  default:
    return "UNKNOWN";
#undef XX
  }
  return "UNKNOWN";
}

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel level,
                   const char* file, int32_t line, uint32_t uptime,
                   int32_t threadId, uint32_t fiberId, int64_t time)
    : m_file(file), m_line(line), m_threadId(threadId), m_fiberId(fiberId),
      m_time(time), m_uptime(uptime), m_logger(logger), m_level(level) {}

LogEventTraker::LogEventTraker(LogEvent::ptr event) { m_event = event; }

LogEventTraker::~LogEventTraker() {
  m_event->getLogger()->log(m_event->getLevel(), m_event);
}

std::stringstream& LogEventTraker::getStream() { return m_event->getStream(); }

std::string LogFormatter::format(Logger::ptr logger, LogLevel level,
                                 LogEvent::ptr event) {
  std::ostringstream ss;
  for (auto& item : m_items) {
    item->format(ss, logger, level, event);
  }
  return ss.str();
}

class MessageFormatItem : public LogFormatter::FormatItem {
public:
  MessageFormatItem(const std::string& fmt = "") {}
  void format(std::ostream& os, Logger::ptr, LogLevel,
              LogEvent::ptr event) override {
    os << event->getContent();
  }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
  LevelFormatItem(const std::string& fmt = "") {}
  void format(std::ostream& os, Logger::ptr, LogLevel level,
              LogEvent::ptr) override {
    os << stringifyLogLevel(level);
  }
};

class UptimeFormatItem : public LogFormatter::FormatItem {
public:
  UptimeFormatItem(const std::string& fmt = "") {}
  void format(std::ostream& os, Logger::ptr, LogLevel,
              LogEvent::ptr event) override {
    os << event->getUptime();
  }
};

class LoggerNameFormatItem : public LogFormatter::FormatItem {
public:
  LoggerNameFormatItem(const std::string& fmt = "") {}
  void format(std::ostream& os, Logger::ptr logger, LogLevel,
              LogEvent::ptr) override {
    os << logger->getName();
  }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
  ThreadIdFormatItem(const std::string& fmt = "") {}
  void format(std::ostream& os, Logger::ptr, LogLevel,
              LogEvent::ptr event) override {
    os << event->getThreadId();
  }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
  FiberIdFormatItem(const std::string& fmt = "") {}
  void format(std::ostream& os, Logger::ptr, LogLevel,
              LogEvent::ptr event) override {
    os << event->getFiberId();
  }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
  DateTimeFormatItem(const std::string& fmt = "%Y-%m-%d %H:%M:%S")
      : m_format(fmt) {}
  void format(std::ostream& os, Logger::ptr, LogLevel,
              LogEvent::ptr event) override {
    struct tm tm;
    time_t time = event->getTime();
    localtime_r(&time, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), m_format.c_str(), &tm);
    os << buf;
  }

private:
  std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatItem {
public:
  FilenameFormatItem(const std::string& fmt = "") {}
  void format(std::ostream& os, Logger::ptr, LogLevel,
              LogEvent::ptr event) override {
    os << event->getFile();
  }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
  LineFormatItem(const std::string& fmt = "") {}
  void format(std::ostream& os, Logger::ptr, LogLevel,
              LogEvent::ptr event) override {
    os << event->getLine();
  }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
  NewLineFormatItem(const std::string& fmt = "") {}
  void format(std::ostream& os, Logger::ptr, LogLevel, LogEvent::ptr) override {
    os << '\n';
  }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
  StringFormatItem(const std::string& str) : m_string(str) {}
  void format(std::ostream& os, Logger::ptr, LogLevel, LogEvent::ptr) override {
    os << m_string;
  }

private:
  std::string m_string;
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
  TabFormatItem(const std::string& str) {}
  void format(std::ostream& os, Logger::ptr, LogLevel, LogEvent::ptr) override {
    os << '\t';
  }
};

/*
 * %m: message
 * %p: level, e.g. DEBUG, INFO ...
 * %r: uptime
 * %c: class name (loger name)
 * %t: thread id
 * %n: new line
 * %d: time format e.g. %d{yyy MMM dd HH:mm:ss,SSS}
 * %f: file name
 * %l: line number
 * %T: tab
 * %F: fiberId
 */
Logger::Logger(const std::string& name)
    : m_name(name), m_level(LogLevel::DEBUG) {
  m_formatter.reset(new LogFormatter{
      "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T(%c)%T<%f:%l>%T%m%n"});
}

void Logger::log(LogLevel level, LogEvent::ptr event) {
  if (level >= m_level) {
    auto self = shared_from_this();
    for (auto& appender : m_appenders) {
      appender->log(self, level, event);
    }
  }
}
void Logger::debug(LogEvent::ptr event) { log(LogLevel::DEBUG, event); }
void Logger::info(LogEvent::ptr event) { log(LogLevel::INFO, event); }
void Logger::warn(LogEvent::ptr event) { log(LogLevel::WARN, event); }
void Logger::error(LogEvent::ptr event) { log(LogLevel::ERROR, event); }
void Logger::fatal(LogEvent::ptr event) { log(LogLevel::FATAL, event); }

void Logger::addAppender(LogAppender::ptr appender) {
  if (!appender->getFormatter()) {
    appender->setFormatter(m_formatter);
  }
  m_appenders.push_back(appender);
}
void Logger::delAppender(LogAppender::ptr appender) {
  for (auto it = m_appenders.begin(); it != m_appenders.end(); it++) {
    if (*it == appender) {
      m_appenders.erase(it);
      break;
    }
  }
}

FileLogAppender::FileLogAppender(const std::string& filename)
    : m_filename(filename) {}

void StdoutLogAppender::log(Logger::ptr logger, LogLevel level,
                            LogEvent::ptr event) {
  if (level > m_level) {
    std::cout << m_formatter->format(logger, level, event);
  }
}

bool FileLogAppender::reopen() {
  if (m_filestream) {
    m_filestream.close();
  }
  m_filestream.open(m_filename);
  return !!m_filestream;
}

void FileLogAppender::log(Logger::ptr logger, LogLevel level,
                          LogEvent::ptr event) {
  if (level >= m_level) {
    m_filestream << m_formatter->format(logger, level, event);
  }
}

LogFormatter::LogFormatter(const std::string& pattern) : m_pattern(pattern) {
  init();
}

/**
 * parsing pattern e.g. %xxx, %xxx{xxx} %%
 */
void LogFormatter::init() {
  // str, format, type
  // type: 0 = text, 1 = pattern
  std::vector<std::tuple<std::string, std::string, int>> vec;

  size_t p0 = 0;
  std::string nstr; // text which is not format pattern.
  while (p0 < m_pattern.size()) {
    if (m_pattern[p0] != '%') {
      nstr.append(1, m_pattern[p0]);
      p0++;
      continue;
    }

    size_t p1 = p0 + 1;
    bool isValid = true;
    std::string str;
    std::string fmt;
    size_t fmt_begin = 0;
    while (p1 < m_pattern.size()) {
      char cur = m_pattern[p1];
      if (isValid && !std::isalpha(cur) && cur != '{' && cur != '}') {
        str = m_pattern.substr(p0 + 1, p1 - (p0 + 1));
        break;
      }

      if (isValid) {
        if (cur == '{') {
          str = m_pattern.substr(p0 + 1, p1 - (p0 + 1));
          isValid = false;
          fmt_begin = p1;
          ++p1;
          continue;
        }
      }

      else if (!isValid) {
        if (cur == '}') {
          fmt = m_pattern.substr(fmt_begin + 1, p1 - (fmt_begin + 1));
          isValid = true;
          ++p1;
          break;
        }
      }
      ++p1;
      if (p1 == m_pattern.size()) {
        str = m_pattern.substr(p0 + 1);
      }
    }

    if (isValid) {
      if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
        nstr.clear();
      }
      vec.push_back(std::make_tuple(str, fmt, 1));
      p0 = p1;
    } else {
      std::cout << "pattern parse error: " << m_pattern << " - "
                << m_pattern.substr(p0) << std::endl;
      vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
    }

    if (!nstr.empty()) {
      vec.push_back(std::make_tuple(nstr, "", 0));
    }
  }

  /*
   * %m: message
   * %p: level, e.g. DEBUG, INFO ...
   * %r: uptime
   * %c: class name (loger name)
   * %t: thread id
   * %n: new line
   * %d: time format e.g. %d{yyy MMM dd HH:mm:ss,SSS}
   * %f: file name
   * %l: line number
   * %T: tab
   * %F: fiberId
   */
  static std::map<std::string,
                  std::function<FormatItem::ptr(const std::string& str)>>
      s_format_items_map = {
#define XX(str, C)                                                             \
  {#str, [](const std::string& fmt) { return std::make_shared<C>(fmt); }}

        XX(m, MessageFormatItem),
        XX(p, LevelFormatItem),
        XX(r, UptimeFormatItem),
        XX(c, LoggerNameFormatItem),
        XX(t, ThreadIdFormatItem),
        XX(n, NewLineFormatItem),
        XX(d, DateTimeFormatItem),
        XX(f, FilenameFormatItem),
        XX(l, LineFormatItem),
        XX(T, TabFormatItem),
        XX(F, FiberIdFormatItem)
#undef XX
      };

  for (const auto& item : vec) {
    if (std::get<2>(item) == 0) {
      m_items.push_back(std::make_shared<StringFormatItem>(std::get<0>(item)));
    } else {
      auto it = s_format_items_map.find(std::get<0>(item));
      if (it == s_format_items_map.end()) {
        m_items.push_back(std::make_shared<StringFormatItem>(
            "<<error_format %" + std::get<0>(item) + ">>"));
      } else {
        m_items.push_back(it->second(std::get<1>(item)));
      }
    }
  }
}

} // namespace log

} // namespace cpplibs
