#pragma once

#include <miet/lambda/execution/control/base/memory-limit-checker.hpp>

namespace miet::lambda::execution::control {
class MemoryLimitChecker final : public MemoryLimitCheckerBase {
 public:
  /**
   * @param limit The memory limit in bytes */
  explicit MemoryLimitChecker(std::size_t limit) noexcept;

  void Allocated(std::size_t size) override;
  void Deallocated(std::size_t size) override;

  bool IsLimitReached() const noexcept override;

 private:
  std::size_t used_ = 0;
  std::size_t limit_ = 0;
};

class MemoryLimitCheckersFactory final : public MemoryLimitCheckersFactoryBase {
 public:
  /**
   * @param limit The memory limit in bytes */
  MemoryLimitCheckerPtr CreateChecker(std::size_t limit) const override;
};
}  // namespace miet::lambda::execution::control
