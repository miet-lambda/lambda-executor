#include <miet/lambda/scripts-storage.hpp>

namespace miet::lambda {
ScriptsStorage::ScriptsStorage(fetchers::ScriptsFetcherPtr fetcher,
                               engine::CompilerPtr compiler,
                               std::size_t maxCacheSize) noexcept
    : fetcher_(std::move(fetcher)),
      compiler_(std::move(compiler)),
      scripts_(maxCacheSize) {
  useCache_ = maxCacheSize != 0;
}

engine::ScriptPtr ScriptsStorage::Get(engine::ScriptIdType scriptId) {
  if (!useCache_) {
    const auto scriptInfo = fetcher_->Fetch(scriptId);
    return compiler_->Compile(scriptInfo.sourceCode);
  }
  {
    auto lock = scripts_.Lock();
    auto* cachedScript = lock->Get(scriptId);
    if (cachedScript) {
      return *cachedScript;
    }
  }
  const auto scriptInfo = fetcher_->Fetch(scriptId);
  const auto script = compiler_->Compile(scriptInfo.sourceCode);
  script->SetScriptId(scriptId);
  script->SetProjectId(scriptInfo.projectId);
  {
    auto lock = scripts_.Lock();
    lock->Put(scriptId, script);
  }
  return script;
}

void ScriptsStorage::Refresh(engine::ScriptIdType scriptId) {
  const auto scriptInfo = fetcher_->Fetch(scriptId);
  const auto script = compiler_->Compile(scriptInfo.sourceCode);
  script->SetScriptId(scriptId);
  script->SetProjectId(scriptInfo.projectId);

  auto lock = scripts_.Lock();
  lock->Put(scriptId, script);
}
}  // namespace miet::lambda
