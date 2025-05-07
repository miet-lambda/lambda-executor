#pragma once

#include <miet/lambda/base/script-update-observer.hpp>

#include <miet/lambda/fetchers/base/scripts-fetcher.hpp>

#include <miet/lambda/engine/base/compiler.hpp>

#include <userver/cache/lru_map.hpp>
#include <userver/concurrent/variable.hpp>
#include <userver/engine/shared_mutex.hpp>

namespace miet::lambda {
class ScriptsStorage final : public ScriptUpdateObserverBase {
 public:
  ScriptsStorage(fetchers::ScriptsFetcherPtr fetcher,
                 engine::CompilerPtr compiler,
                 std::size_t maxCacheSize) noexcept;

  [[nodiscard]] engine::ScriptPtr Get(engine::ScriptIdType scriptId);

  void Refresh(engine::ScriptIdType scriptId) override;

 private:
  fetchers::ScriptsFetcherPtr fetcher_ = nullptr;
  engine::CompilerPtr compiler_ = nullptr;

  using ScriptsCache =
      userver::cache::LruMap<engine::ScriptIdType, engine::ScriptPtr>;

  userver::concurrent::Variable<ScriptsCache, userver::engine::SharedMutex>
      scripts_;
};

using ScriptsStoragePtr = std::shared_ptr<ScriptsStorage>;
}  // namespace miet::lambda
