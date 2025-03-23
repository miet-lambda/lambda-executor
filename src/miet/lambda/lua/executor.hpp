#pragma once

#include <miet/lambda/base/scripts-fetcher.hpp>
#include <miet/lambda/execution-context.hpp>

#include <userver/utils/fast_pimpl.hpp>

namespace miet::lambda::lua {
class Executor final {
 public:
  Executor(ScriptsFetcherPtr fetcher);
  ~Executor();

  void Execute(std::string_view id, ExecutionContextRef context);

 private:
  class Impl;
  userver::utils::FastPimpl<Impl, 320, 8> impl_;
};
}  // namespace miet::lambda::lua
