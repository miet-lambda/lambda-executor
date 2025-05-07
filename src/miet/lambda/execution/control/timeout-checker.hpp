#pragma once

#include <miet/lambda/execution/control/base/timeout-checker.hpp>

namespace miet::lambda::execution::control {
class TimeoutChecker final : public TimeoutCheckerBase {
 public:
  explicit TimeoutChecker(TimeoutType timeout) noexcept;

  void Start() noexcept override;

  bool IsExpired() const noexcept override;

 private:
  TimeoutType timeout_;
  DeadlineType deadline_;
};

class TimeoutCheckersFactory final : public TimeoutCheckersFactoryBase {
 public:
  TimeoutCheckerPtr CreateChecker(
      TimeoutCheckerBase::TimeoutType timeout) const noexcept override;
};
}  // namespace miet::lambda::execution::control
