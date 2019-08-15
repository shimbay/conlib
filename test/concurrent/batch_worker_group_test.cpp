#include <gtest/gtest.h>

#include "conlib/concurrent/batch_worker_group.h"

using namespace conlib;

class TestBatchWorkerGroup final : public BatchWorkerGroup<int, std::string> {
public:
  using BatchWorkerGroup::BatchWorkerGroup;

  ~TestBatchWorkerGroup() override = default;

protected:
  void work(VectorTuple<int, std::string> &data) override {
    std::cout << "Working..." << std::endl;
    auto &iv = visit<0>(data);
    auto &sv = visit<1>(data);
    for (size_t i = 0; i < iv.size(); ++i) {
      std::cout << iv[i] << std::endl;
      std::cout << sv[i] << std::endl;
    }
  }
};

class BatchWorkerGroupTest : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    test_group = std::make_shared<TestBatchWorkerGroup>(2, 2, std::chrono::milliseconds(10));
  }

  static void TearDownTestSuite() {}

  static std::shared_ptr<TestBatchWorkerGroup> test_group;
};

std::shared_ptr<TestBatchWorkerGroup> BatchWorkerGroupTest::test_group{};

TEST_F(BatchWorkerGroupTest, Run) {
  test_group->submit(0, "Test 0");
  test_group->submit(1, "Test 1");
  usleep(1000000);
}
