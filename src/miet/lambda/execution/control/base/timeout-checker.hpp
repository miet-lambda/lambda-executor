#pragma once

#include <chrono>
#include <memory>

namespace miet::lambda::execution::control {
class TimeoutCheckerBase {
 public:
  using TimeoutType = std::chrono::milliseconds;
  using DeadlineType = std::chrono::steady_clock::time_point;

  virtual ~TimeoutCheckerBase() = default;

  virtual void Start() noexcept = 0;

  virtual bool IsExpired() const noexcept = 0;
};

using TimeoutCheckerPtr = std::shared_ptr<TimeoutCheckerBase>;

class TimeoutCheckersFactoryBase {
 public:
  virtual ~TimeoutCheckersFactoryBase() = default;

  virtual TimeoutCheckerPtr CreateChecker(
      TimeoutCheckerBase::TimeoutType timeout) const noexcept = 0;
};

using TimeoutCheckersFactoryPtr = std::shared_ptr<TimeoutCheckersFactoryBase>;
}  // namespace miet::lambda::execution::control
