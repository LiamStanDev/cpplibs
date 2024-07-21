#include "log.h"
#include <ctime>
#include <iostream>
#include <memory>

int main() {
  using namespace cpplibs;

  std::unique_ptr<Logger> logger{new Logger{}};
  std::unique_ptr<StdoutLogAppender> stdoutLogAppender{
      new StdoutLogAppender{LogLevel::INFO}};
  logger->addAppender(std::move(stdoutLogAppender));

  std::unique_ptr<FileLogAppender> fileLogAppender{
      new FileLogAppender{"./log.txt", LogLevel::DEBUG}};
  fileLogAppender->setFormatter("%d%m%d%n");
  logger->addAppender(std::move(fileLogAppender));

  std::cout << "Hello Logger" << std::endl;
  LOG_INFO(*logger) << "test macro info";
  LOG_ERROR(*logger) << "test macro error";

  return 0;
}
