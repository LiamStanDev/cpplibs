#include "cosmic.h"

#include <iostream>
#include <memory>
#include <unistd.h>

void func1() {
  std::cout << "name: " << cosmic::Thread::GetName()
            << "\nthis.name: " << cosmic::Thread::GetThis().GetName()
            << "\nid: " << cosmic::GetThreadId()
            << "\nthis.id: " << cosmic::Thread::GetThis().getId() << "\n"
            << std::endl;

  sleep(20);
}

void func2() {
  std::cout << "name: " << cosmic::Thread::GetName()
            << "this.name: " << cosmic::Thread::GetThis().GetName()
            << "id: " << cosmic::GetThreadId()
            << "this.id: " << cosmic::Thread::GetThis().getId() << std::endl;
}

int main() {
  std::cout << "thread test begin" << std::endl;

  std::vector<std::unique_ptr<cosmic::Thread>> threads;
  for (int i = 0; i < 5; i++) {
    std::unique_ptr<cosmic::Thread> thread{
        new cosmic::Thread(&func1, "name_" + std::to_string(i))};
    threads.push_back(std::move(thread));
  }

  for (const auto& thread : threads) {
    thread->join();
  }

  std::cout << "thread test end" << std::endl;
  return 0;
}
