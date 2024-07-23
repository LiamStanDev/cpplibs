#include "sync.h"
#include <iostream>
#include <thread>
#include <vector>

void test_mutex() {
  using namespace cpplibs;
  Mutex<int> mutex(0); // 初始化保护整数的 Mutex

  auto worker = [&mutex](int id) {
    for (int i = 0; i < 5; ++i) {
      auto guard = mutex.lock(); // 获取锁定句柄
      ++(*guard);                // 访问和修改共享资源
      std::cout << "Worker " << id << " incremented value to " << *guard
                << std::endl;
    }
  };

  std::vector<std::thread> threads;
  for (int i = 0; i < 3; ++i) {
    threads.emplace_back(worker, i);
  }

  for (auto& t : threads) {
    t.join();
  }
}

int main() {
  test_mutex();
  return 0;
}
