#pragma once

#include <memory>
#include <string_view>

namespace miet::lambda {
class ScriptsFetcherBase {
 public:
  virtual ~ScriptsFetcherBase() = default;
  virtual std::string Fetch(std::string_view id) = 0;
};

using ScriptsFetcherPtr = std::shared_ptr<ScriptsFetcherBase>;
}  // namespace miet::lambda
