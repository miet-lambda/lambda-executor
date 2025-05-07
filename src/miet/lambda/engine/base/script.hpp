#pragma once

#include <memory>

namespace miet::lambda::engine {
enum class ScriptLanguage : std::uint8_t { Stub, Lua, JS };

using ScriptIdType = std::size_t;
using ProjectIdType = std::size_t;

class ScriptBase {
 public:
  virtual ~ScriptBase() = default;

  ScriptBase() noexcept = default;
  explicit ScriptBase(ScriptLanguage language) noexcept : language_(language) {}

  void SetLanguage(ScriptLanguage language) noexcept { language_ = language; }
  void SetProjectId(ProjectIdType projectId) noexcept {
    projectId_ = projectId;
  }
  void SetScriptId(ScriptIdType scriptId) noexcept { scriptId_ = scriptId; }

  ScriptLanguage GetLanguage() const noexcept { return language_; }
  ProjectIdType GetProjectId() const noexcept { return projectId_; }
  ScriptIdType GetScriptId() const noexcept { return scriptId_; }

 private:
  ScriptLanguage language_;
  ProjectIdType projectId_;
  ScriptIdType scriptId_;
};

using ScriptPtr = std::shared_ptr<ScriptBase>;
}  // namespace miet::lambda::engine
