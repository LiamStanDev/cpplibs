#include "cosmic/log.h"

#include <functional>
#include <ios>
#include <iostream>
#include <map>
#include <memory>

namespace cosmic {

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

class MessageFormatItem : public LogFormatter::FormatItem {
public:
  MessageFormatItem(const std::string& fmt = "") {}
  void format(std::ostream& os, const Logger& logger,
              LogEvent::ptr event) override {
    os << event->getContent();
  }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
  LevelFormatItem(const std::string& fmt = "") {}
  void format(std::ostream& os, const Logger&, LogEvent::ptr event) override {
    os << stringifyLogLevel(event->getLevel());
  }
};

class UptimeFormatItem : public LogFormatter::FormatItem {
public:
  UptimeFormatItem(const std::string& fmt = "") {}
  void format(std::ostream& os, const Logger&, LogEvent::ptr event) override {
    os << event->getUptime();
  }
};

class LoggerNameFormatItem : public LogFormatter::FormatItem {
public:
  LoggerNameFormatItem(const std::string& fmt = "") {}
  void format(std::ostream& os, const Logger& logger, LogEvent::ptr) override {
    os << logger.getName();
  }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
  ThreadIdFormatItem(const std::string& fmt = "") {}
  void format(std::ostream& os, const Logger&, LogEvent::ptr event) override {
    os << event->getThreadId();
  }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
  FiberIdFormatItem(const std::string& fmt = "") {}
  void format(std::ostream& os, const Logger&, LogEvent::ptr event) override {
    os << event->getFiberId();
  }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
  DateTimeFormatItem(const std::string& fmt = "%Y-%m-%d %H:%M:%S")
      : m_format(fmt) {}
  void format(std::ostream& os, const Logger&, LogEvent::ptr event) override {
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
  void format(std::ostream& os, const Logger&, LogEvent::ptr event) override {
    os << event->getFile();
  }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
  LineFormatItem(const std::string& fmt = "") {}
  void format(std::ostream& os, const Logger&, LogEvent::ptr event) override {
    os << event->getLine();
  }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
  NewLineFormatItem(const std::string& fmt = "") {}
  void format(std::ostream& os, const Logger&, LogEvent::ptr) override {
    os << '\n';
  }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
  StringFormatItem(const std::string& str) : m_string(str) {}
  void format(std::ostream& os, const Logger&, LogEvent::ptr) override {
    os << m_string;
  }

private:
  std::string m_string;
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
  TabFormatItem(const std::string& str) {}
  void format(std::ostream& os, const Logger&, LogEvent::ptr) override {
    os << '\t';
  }
};

LogEvent::LogEvent(LogLevel level, const char* file, int32_t line,
                   uint32_t uptime, int32_t threadId, uint32_t fiberId,
                   int64_t time)
    : m_level(level), m_file(file), m_line(line), m_threadId(threadId),
      m_fiberId(fiberId), m_time(time), m_uptime(uptime) {}

LogEventTracker::LogEventTracker(LogEvent::ptr event, const Logger& logger)
    : m_event(event), m_logger(logger) {}

LogEventTracker::~LogEventTracker() { m_logger.log(m_event); }

LogFormatter::LogFormatter(const std::string& pattern) : m_pattern(pattern) {
  init();
}

std::string LogFormatter::format(const Logger& logger,
                                 std::shared_ptr<LogEvent> event) {
  std::ostringstream ss;

  for (const auto& item : m_items) {
    item->format(ss, logger, event);
  }
  std::string content = ss.str();
  return content;
}

void LogFormatter::setPattern(const std::string& pattern) {
  m_pattern = pattern;
  init();
}

void LogFormatter::init() {
  m_items.clear();
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
  static std::map<std::string, std::function<std::unique_ptr<FormatItem>(
                                   const std::string& str)>>
      s_format_items_map = {
#define XX(str, C)                                                             \
  {                                                                            \
    #str, [](const std::string& fmt) { return std::make_unique<C>(fmt); }      \
  }

          XX(m, MessageFormatItem),  XX(p, LevelFormatItem),
          XX(r, UptimeFormatItem),   XX(c, LoggerNameFormatItem),
          XX(t, ThreadIdFormatItem), XX(n, NewLineFormatItem),
          XX(d, DateTimeFormatItem), XX(f, FilenameFormatItem),
          XX(l, LineFormatItem),     XX(T, TabFormatItem),
          XX(F, FiberIdFormatItem)
#undef XX
      };

  for (const auto& item : vec) {
    if (std::get<2>(item) == 0) {
      m_items.push_back(std::make_unique<StringFormatItem>(std::get<0>(item)));
    } else {
      auto it = s_format_items_map.find(std::get<0>(item));
      if (it == s_format_items_map.end()) {
        m_items.push_back(std::make_unique<StringFormatItem>(
            "<<error_format %" + std::get<0>(item) + ">>"));
      } else {
        m_items.push_back(it->second(std::get<1>(item)));
      }
    }
  }
}

LogAppender::LogAppender(LogLevel level) : m_level(level) {
  m_formatter = std::make_unique<LogFormatter>();
}

LogAppender::LogAppender(std::unique_ptr<LogFormatter> formatter,
                         LogLevel level)
    : m_formatter(std::move(formatter)), m_level(level) {}

void LogAppender::setFormatter(const std::string& pattern) {
  m_formatter->setPattern(pattern);
}

StdoutLogAppender::StdoutLogAppender(std::unique_ptr<LogFormatter> formatter,
                                     LogLevel level)
    : LogAppender(std::move(formatter), level) {}

StdoutLogAppender::StdoutLogAppender(LogLevel level) : LogAppender(level) {}

StdoutLogAppender::StdoutLogAppender() : LogAppender(LogLevel::DEBUG) {}

void StdoutLogAppender::log(const Logger& logger, LogEvent::ptr event) {
  if (event->getLevel() >= m_level) {
    std::cout << m_formatter->format(logger, event);
  }
}

FileLogAppender::FileLogAppender(const std::string& filename,
                                 std::unique_ptr<LogFormatter> formatter,
                                 LogLevel level)
    : LogAppender(std::move(formatter), level), m_filename(filename) {
  reopen();
}

FileLogAppender::FileLogAppender(const std::string& filename, LogLevel level)
    : LogAppender(level), m_filename(filename) {
  reopen();
}

FileLogAppender::FileLogAppender(const std::string& filename)
    : LogAppender(LogLevel::DEBUG), m_filename(filename) {
  reopen();
}

FileLogAppender::~FileLogAppender() {
  if (m_filestream) {
    m_filestream.close();
  }
}

void FileLogAppender::log(const Logger& logger, LogEvent::ptr event) {
  if (!m_filestream.is_open()) {
    reopen();
  }
  if (event->getLevel() >= m_level) {
    m_filestream << m_formatter->format(logger, event);
    m_filestream.flush();
  }
}

bool FileLogAppender::reopen() {
  if (m_filestream) {
    m_filestream.close();
  }
  m_filestream.open(m_filename, std::ios_base::out | std::ios_base::app);
  return !!m_filestream;
}

Logger::Logger(const std::string& name) : m_name(name) {}

void Logger::log(LogEvent::ptr event) const {
  for (const auto& appender : m_appenders) {
    appender->log(*this, event);
  }
}

void Logger::addAppender(std::shared_ptr<LogAppender> appender) {
  m_appenders.push_back(appender);
}

void Logger::delAppender(std::shared_ptr<LogAppender> appender) {
  for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
    if (*it == appender) {
      m_appenders.erase(it);
      break;
    }
  }
}

std::shared_ptr<LoggerManager> LoggerManager::s_instance = nullptr;

std::shared_ptr<LoggerManager> LoggerManager::GetInstance() {
  if (s_instance == nullptr) {
    s_instance = std::shared_ptr<LoggerManager>{new LoggerManager{}};
  }
  return s_instance;
}

LoggerManager::LoggerManager() {
  m_root.reset(new Logger{});
  m_root->addAppender(std::shared_ptr<LogAppender>{new StdoutLogAppender{}});
  m_loggers[m_root->getName()] = m_root;
}

std::shared_ptr<Logger> LoggerManager::getLogger(const std::string& name) {
  auto it = m_loggers.find(name);
  if (it != m_loggers.end()) {
    return it->second;
  }

  // add new logger
  std::shared_ptr<Logger> logger{new Logger{name}};
  m_loggers[name] = logger;
  return logger;
}

} // namespace cosmic
