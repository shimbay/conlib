#pragma once

#include "worker_group.h"

namespace conlib {

class Executor : public WorkerGroup<std::function<void()>> {
public:
  explicit Executor(size_t worker_num) : WorkerGroup(worker_num) {}

  ~Executor() override = default;

private:
  void work(std::function<void()> &t) override { t(); }
};

} // namespace conlib