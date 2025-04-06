#pragma once

#include <memory>
#include <string>

namespace miet::lambda {
struct ScriptInfo final {
  std::int64_t projectId = 0;
  std::string sourceCode;
};

class ScriptsFetcherBase {
 public:
  virtual ~ScriptsFetcherBase() = default;
  virtual ScriptInfo Fetch(std::string_view id) = 0;
};

using ScriptsFetcherPtr = std::shared_ptr<ScriptsFetcherBase>;
}  // namespace miet::lambda
