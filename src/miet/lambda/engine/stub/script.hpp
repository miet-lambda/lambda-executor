#pragma once

#include <miet/lambda/engine/base/script.hpp>

namespace miet::lambda::engine::stub {
class Script final : public ScriptBase {
 public:
  Script() : ScriptBase(ScriptLanguage::Stub) {}

  void SetSourceCode(std::string_view code) { code_ = code; }
  const std::string& GetSourceCode() const noexcept { return code_; }

 private:
  std::string code_;
};
}  // namespace miet::lambda::engine::stub
