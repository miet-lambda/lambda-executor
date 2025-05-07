#pragma once

#include <miet/lambda/execution/control/base/memory-limit-checker.hpp>

#include <gmock/gmock.h>

namespace miet::lambda::execution::control {
class MemoryLimitCheckerMock final : public MemoryLimitCheckerBase {
 public:
  MOCK_METHOD(void, Allocated, (std::size_t), ());
  MOCK_METHOD(void, Deallocated, (std::size_t), ());
  MOCK_METHOD(bool, IsLimitReached, (), (const, noexcept));
};

class MemoryLimitCheckersFactoryMock final
    : public MemoryLimitCheckersFactoryBase {
 public:
  MOCK_METHOD(MemoryLimitCheckerPtr, CreateChecker, (std::size_t limit),
              (const));
};
}  // namespace miet::lambda::execution::control
