#pragma once

#include <miet/lambda/base/executor.hpp>
#include <miet/lambda/base/http-client.hpp>
#include <miet/lambda/base/key-value-storage.hpp>
#include <miet/lambda/base/memory-allocator.hpp>
#include <miet/lambda/base/scripts-fetcher.hpp>
#include <miet/lambda/base/timeout-checker.hpp>
#include <miet/lambda/execution-context.hpp>

#include <userver/utils/fast_pimpl.hpp>

namespace miet::lambda::lua {
struct ExecutorParams final {
  ScriptsFetcherPtr scriptsFetcher = nullptr;
  TimeoutCheckersFactoryPtr timeoutCheckersFactory = nullptr;
  MemoryAllocatorsFactoryPtr memoryAllocatorsFactory = nullptr;
};

struct LibsDeps final {
  HttpClientPtr httpClient = nullptr;
  KeyValueStoragePtr kvStorage = nullptr;
};

class Executor final : public ExecutorBase {
 public:
  explicit Executor(ExecutorParams params, LibsDeps deps = {});
  ~Executor();

  void Execute(std::string_view id, ExecutionContextRef context) override;

 private:
  class Impl;
  userver::utils::FastPimpl<Impl, 448, 8> impl_;
};
}  // namespace miet::lambda::lua
