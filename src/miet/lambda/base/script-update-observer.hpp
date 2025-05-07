#pragma once

#include <miet/lambda/engine/base/script.hpp>

namespace miet::lambda {
class ScriptUpdateObserverBase {
 public:
  virtual void Refresh(engine::ScriptIdType scriptId) = 0;
};
}  // namespace miet::lambda
