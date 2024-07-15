#include "log.h"

#include <cctype>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>

namespace cpplibs {
namespace logger {

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
  DateTimeFormatItem(const std::string& fmt = "%Y:%m:%d %H:%M:%S")
      : m_format(fmt) {}
  void format(std::ostream& os, Logger::ptr, LogLevel,
              LogEvent::ptr event) override {
    os << event->getTime();
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

Logger::Logger(const std::string& name) : m_name(name) {}

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

LogFormatter::LogFormatter(const std::string& pattern) : m_pattern(pattern) {}

/**
 * parsing pattern e.g. %xxx, %xxx{xxx} %%
 */
void LogFormatter::init() {
  // str, format, type
  // type: 0 = text, 1 = pattern
  std::vector<std::tuple<std::string, std::string, int>> vec;
  std::string nstr; // text which is not format pattern.

  for (size_t i = 0; i < m_pattern.size(); i++) {
    // add not format pattern
    if (m_pattern[i] != '%') {
      nstr.append(1, m_pattern[i]);
      continue;
    }

    // parsing %%
    if ((i + 1) < m_pattern.size() && m_pattern[i + 1] == '%') {
      nstr.append(1, '%');
      continue;
    }

    size_t n = i + 1;
    // 0: Ok
    // 1: Not finished, without end '}'
    bool fmt_status = true;
    size_t fmt_begin = 0;

    std::string str; // format pattern
    std::string fmt; // format pattern inside {}

    // find format item
    while (n < m_pattern.size()) {
      // end of pattern
      if (fmt_status && (!std::isalpha(m_pattern[n]) && m_pattern[n] != '{' &&
                         m_pattern[n] != '}')) {
        str = m_pattern.substr(i + 1, n - (i + 1));
        break;
      }

      // the start of sub format
      if (fmt_status && m_pattern[n] == '{') {
        str = m_pattern.substr(i + 1, n - (i + 1));
        fmt_status = 1;
        fmt_begin = n;
        n++;
        continue;
      }
      // the end of sub format
      else if (!fmt_status && m_pattern[n] == '}') {
        fmt = m_pattern.substr(fmt_begin + 1, n - (fmt_begin + 1));
        fmt_status = 0;
        n++;
        break;
      }

      // last position
      if (n == m_pattern.size() - 1) {
        if (str.empty()) {
          str = m_pattern.substr(i + 1);
        }
      }
      n++;
    }

    if (fmt_status) {
      if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0)); // 0 is text
        nstr.clear();
      }
      vec.push_back(std::make_tuple(str, fmt, 1)); // 1 is %xxx
      i = n - 1;
    } else {
      std::cerr << "pattern parse error: " << m_pattern << " - "
                << m_pattern.substr(i) << std::endl;
      vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
    }
  }

  if (!nstr.empty()) {
    vec.push_back(std::make_tuple(nstr, "", 0));
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
   */
  static std::map<std::string,
                  std::function<FormatItem::ptr(const std::string& str)>>
      s_format_items = {
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

  for (auto& i : vec) {
    if (std::get<2>(i) == 0) {
      m_items.push_back(std::make_shared<StringFormatItem>(std::get<0>(i)));
    } else {
      auto it = s_format_items.find(std::get<0>(i));
      if (it == s_format_items.end()) {
        m_items.push_back(FormatItem::ptr(
            new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
      } else {
        m_items.push_back(it->second(std::get<1>(i)));
      }
    }
  }
}
} // namespace logger

} // namespace cpplibs
