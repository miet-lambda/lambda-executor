#pragma once

#include <miet/lambda/base/scripts-fetcher.hpp>

#include <gmock/gmock.h>

namespace miet::lambda {
class ScriptsFetcherMock final : public ScriptsFetcherBase {
 public:
  MOCK_METHOD(std::string, Fetch, (std::string_view));
};
}  // namespace miet::lambda
