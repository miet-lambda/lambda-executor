#pragma once

#include <miet/lambda/fetchers/base/scripts-fetcher.hpp>

#include <gmock/gmock.h>

namespace miet::lambda::fetchers {
class ScriptsFetcherMock final : public ScriptsFetcherBase {
 public:
  MOCK_METHOD(ScriptInfo, Fetch, (engine::ScriptIdType));
};
}  // namespace miet::lambda::fetchers
