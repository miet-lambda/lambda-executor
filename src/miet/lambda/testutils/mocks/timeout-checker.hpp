#pragma once

#include <miet/lambda/base/timeout-checker.hpp>

#include <gmock/gmock.h>

namespace miet::lambda {
class TimeoutCheckerMock final : public TimeoutCheckerBase {
 public:
  MOCK_METHOD(void, Start, (), (noexcept));
  MOCK_METHOD(bool, IsExpired, (), (const, noexcept));
};

class TimeoutCheckersFactoryMock final : public TimeoutCheckersFactoryBase {
 public:
  MOCK_METHOD(TimeoutCheckerPtr, CreateChecker,
              (TimeoutCheckerBase::TimeoutType), (const, noexcept));
};
}  // namespace miet::lambda
