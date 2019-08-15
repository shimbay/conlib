#pragma once

#include <iostream>
#include <mutex>
#include <sstream>
#include <sys/syscall.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

namespace conlib {

inline size_t _thread_id() {
#ifdef _WIN32
  return static_cast<size_t>(::GetCurrentThreadId());
#elif __linux__
#if defined(__ANDROID__) && defined(__ANDROID_API__) && (__ANDROID_API__ < 21)
#define SYS_gettid __NR_gettid
#endif
  return static_cast<size_t>(syscall(SYS_gettid));
#elif __FreeBSD__
  long tid;
  thr_self(&tid);
  return static_cast<size_t>(tid);
#elif __APPLE__
  uint64_t tid;
  pthread_threadid_np(nullptr, &tid);
  return static_cast<size_t>(tid);
#else // Default to standard C++11 (other Unix)
  return static_cast<size_t>(std::hash<std::thread::id>()(std::this_thread::get_id()));
#endif
}

inline std::string &get_tid() {
  static thread_local std::string tid;
  static thread_local std::once_flag flag;

  std::call_once(flag, []() {
    auto id = _thread_id();
    std::stringstream ss;
    ss << id;
    tid = ss.str();
  });

  return tid;
}

} // namespace conlib
