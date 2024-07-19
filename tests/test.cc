#include "log.h"

#include <ctime>
#include <memory>

int main() {
  using namespace cpplibs::log;
  using namespace cpplibs::common;

  Logger::ptr logger{new Logger{}};
  StdoutLogAppender::ptr stdoutLogAppender{new StdoutLogAppender};
  logger->addAppender(stdoutLogAppender);

  FileLogAppender::ptr fileLogAppender{new FileLogAppender{"./log.txt"}};
  LogFormatter::ptr formatter{new LogFormatter("%d%T%m%d")};
  fileLogAppender->setFormatter(formatter);
  logger->addAppender(fileLogAppender);

  LOG_INFO(logger) << "test macro info";
  LOG_ERROR(logger) << "test macro error";

  return 0;
}
