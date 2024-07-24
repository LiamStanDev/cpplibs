#include "cpplibs.h"
#include <ctime>
#include <iostream>
#include <memory>

int main() {
  using namespace cpplibs;

  std::unique_ptr<Logger> logger{new Logger{}};
  std::shared_ptr<StdoutLogAppender> stdoutLogAppender{
      new StdoutLogAppender{LogLevel::INFO}};
  logger->addAppender(stdoutLogAppender);

  std::shared_ptr<FileLogAppender> fileLogAppender{
      new FileLogAppender{"./log.txt", LogLevel::DEBUG}};
  // fileLogAppender->setFormatter("%d%m%d%n");
  logger->addAppender(fileLogAppender);

  std::cout << "Hello Logger" << std::endl;
  LOG_INFO(*logger) << "test macro info";
  LOG_ERROR(*logger) << "test macro error";

  auto logger2 = LoggerManager::GetInstance()->getLogger("xxx");
  logger2->addAppender(stdoutLogAppender);
  logger2->addAppender(fileLogAppender);
  LOG_INFO(*logger2) << "test manager";
  LOG_ERROR(*logger2) << "test manager";
  return 0;
}
