#include "conlib/concurrent/executor.h"

using namespace conlib;

int main() {
  auto fun = []() { std::cout << "Hello world!" << std::endl; };

  auto &&executor = Executor(4);

  executor.submit(fun);

  usleep(1000000);
}