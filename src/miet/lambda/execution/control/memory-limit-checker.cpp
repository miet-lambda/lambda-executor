#include <miet/lambda/execution/control/memory-limit-checker.hpp>

#include <userver/logging/log.hpp>

namespace miet::lambda::execution::control {
MemoryLimitChecker::MemoryLimitChecker(std::size_t limit) noexcept
    : limit_(limit) {}

void MemoryLimitChecker::Allocated(std::size_t size) { used_ += size; }

void MemoryLimitChecker::Deallocated(std::size_t size) {
  if (used_ < size) {
    LOG_WARNING() << "Deallocated is more than allocated";
    used_ = 0;
    return;
  }
  used_ -= size;
}

bool MemoryLimitChecker::IsLimitReached() const noexcept {
  return used_ > limit_;
}

MemoryLimitCheckerPtr MemoryLimitCheckersFactory::CreateChecker(
    std::size_t limit) const {
  return std::make_shared<MemoryLimitChecker>(limit);
}
}  // namespace miet::lambda::execution::control
