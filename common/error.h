#pragma once

#include <exception>
#include <iostream>
#include <string>

namespace conlib {

enum ErrorCode {
  OK,
  IllegalTimePoint,
  WaitTimeout,
  QueueFull,
  QueueClosed,
};

inline void handle_exception(const std::exception_ptr &exception_ptr, const std::string &desc) {
  try {
    if (exception_ptr) {
      std::rethrow_exception(exception_ptr);
    }
  } catch (const std::exception &e) {
    std::cout << "caught exception, desc: " << desc << ", exception: " << e.what() << std::endl;
  }
}

} // namespace conlib