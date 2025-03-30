#pragma once

#include <miet/lambda/base/executor.hpp>
#include <miet/lambda/base/http-client.hpp>
#include <miet/lambda/base/scripts-fetcher.hpp>
#include <miet/lambda/execution-context.hpp>

#include <userver/utils/fast_pimpl.hpp>

namespace miet::lambda::lua {
struct Dependencies final {
  HttpClientPtr httpClient = nullptr;
};

class Executor final : public ExecutorBase {
 public:
  Executor(ScriptsFetcherPtr fetcher, Dependencies deps = {});
  ~Executor();

  void Execute(std::string_view id, ExecutionContextRef context) override;

 private:
  class Impl;
  userver::utils::FastPimpl<Impl, 320, 8> impl_;
};
}  // namespace miet::lambda::lua
