#include "cpplibs.h"
#include "process.h"
#include "thread.h"

#include <iostream>
#include <memory>
#include <unistd.h>

void func1() {
  std::cout << "name: " << cpplibs::Thread::GetName()
            << "\nthis.name: " << cpplibs::Thread::GetThis().GetName()
            << "\nid: " << cpplibs::GetThreadId()
            << "\nthis.id: " << cpplibs::Thread::GetThis().getId() << "\n"
            << std::endl;

  sleep(20);
}

void func2() {
  std::cout << "name: " << cpplibs::Thread::GetName()
            << "this.name: " << cpplibs::Thread::GetThis().GetName()
            << "id: " << cpplibs::GetThreadId()
            << "this.id: " << cpplibs::Thread::GetThis().getId() << std::endl;
}

int main() {
  std::cout << "thread test begin" << std::endl;

  std::vector<std::unique_ptr<cpplibs::Thread>> threads;
  for (int i = 0; i < 5; i++) {
    std::unique_ptr<cpplibs::Thread> thread{
        new cpplibs::Thread(&func1, "name_" + std::to_string(i))};
    threads.push_back(std::move(thread));
  }

  for (const auto& thread : threads) {
    thread->join();
  }

  std::cout << "thread test end" << std::endl;
  return 0;
}
