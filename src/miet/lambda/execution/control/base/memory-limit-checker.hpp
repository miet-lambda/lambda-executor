#pragma once

#include <memory>

namespace miet::lambda::execution::control {
class MemoryLimitCheckerBase {
 public:
  virtual ~MemoryLimitCheckerBase() = default;

  virtual void Allocated(std::size_t size) = 0;
  virtual void Deallocated(std::size_t size) = 0;

  virtual bool IsLimitReached() const noexcept = 0;
};

using MemoryLimitCheckerPtr = std::shared_ptr<MemoryLimitCheckerBase>;

class MemoryLimitCheckersFactoryBase {
 public:
  virtual ~MemoryLimitCheckersFactoryBase() = default;
  /**
   * @param limit The memory limit in bytes */
  virtual MemoryLimitCheckerPtr CreateChecker(std::size_t limit) const = 0;
};

using MemoryLimitCheckersFactoryPtr =
    std::shared_ptr<MemoryLimitCheckersFactoryBase>;
}  // namespace miet::lambda::execution::control
