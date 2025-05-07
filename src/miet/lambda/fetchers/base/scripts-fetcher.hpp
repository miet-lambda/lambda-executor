#pragma once

#include <miet/lambda/engine/base/script.hpp>

namespace miet::lambda::fetchers {
struct ScriptInfo final {
  engine::ScriptIdType id;
  engine::ProjectIdType projectId;
  std::string sourceCode;
};

class ScriptsFetcherBase {
 public:
  virtual ~ScriptsFetcherBase() = default;
  [[nodiscard]] virtual ScriptInfo Fetch(engine::ScriptIdType id) = 0;
};

using ScriptsFetcherPtr = std::shared_ptr<ScriptsFetcherBase>;
}  // namespace miet::lambda::fetchers
