#include <miet/lambda/timeout-checker.hpp>

namespace miet::lambda {
TimeoutChecker::TimeoutChecker(TimeoutType timeout) noexcept
    : timeout_(timeout) {}

void TimeoutChecker::Start() noexcept {
  deadline_ = std::chrono::steady_clock::now() + timeout_;
}

bool TimeoutChecker::IsExpired() const noexcept {
  const auto now = std::chrono::steady_clock::now();
  return deadline_ < now;
}

TimeoutCheckerPtr TimeoutCheckersFactory::CreateChecker(
    TimeoutCheckerBase::TimeoutType timeout) const noexcept {
  return std::make_shared<TimeoutChecker>(timeout);
}
}  // namespace miet::lambda
