#include <miet/lambda/lua/executor.hpp>

#include <miet/lambda/lua/http-client.hpp>
#include <miet/lambda/lua/http-context.hpp>

#include <userver/concurrent/variable.hpp>
#include <userver/engine/shared_mutex.hpp>

#include <LuaCpp/LuaCpp.hpp>

#include <unordered_set>

namespace miet::lambda::lua {
constexpr auto kHttpContextVariableName = "miet_http_context";
constexpr auto kHttpClientVariableName = "miet_http_client";

class Executor::Impl {
 public:
  Impl(ScriptsFetcherPtr fetcher, Dependencies deps)
      : fetcher_(std::move(fetcher)),
        deps_(std::move(deps)),
        luaContext_(userver::utils::MakeSharedRef<LuaCpp::LuaContext>()) {}

  void Execute(std::string_view id, ExecutionContextRef context) {
    bool compiled = false;
    {
      const auto lock = idsOfCompiledStripts_.SharedLock();
      compiled = lock->contains(id.data());
    }
    if (!compiled) [[likely]] {
      const auto scriptSource = fetcher_->Fetch(id);
      auto lock = idsOfCompiledStripts_.Lock();
      if (!lock->contains(id.data())) {
        luaContext_->CompileString(id.data(), scriptSource);
        lock->insert(id.data());
      }
    }

    const auto luaEnv = std::make_shared<LuaCpp::LuaEnvironment>();

    const auto httpContext = std::make_shared<HttpContext>(luaEnv, context);
    luaEnv->emplace(kHttpContextVariableName, httpContext);
    if (deps_.httpClient) {
      const auto httpClient = std::make_shared<HttpClient>(deps_.httpClient);
      luaEnv->emplace(kHttpClientVariableName, httpClient);
    }
    luaContext_->RunWithEnvironment(id.data(), *luaEnv);
    httpContext->PopulateResponse();
  }

 private:
  using SafeIdsSet =
      userver::concurrent::Variable<std::unordered_set<std::string>,
                                    userver::engine::SharedMutex>;

  ScriptsFetcherPtr fetcher_;
  Dependencies deps_;
  LuaContextRef luaContext_;
  SafeIdsSet idsOfCompiledStripts_;
};

Executor::Executor(ScriptsFetcherPtr fetcher, Dependencies deps)
    : impl_(std::move(fetcher), std::move(deps)) {}

Executor::~Executor() = default;

void Executor::Execute(std::string_view id, ExecutionContextRef context) {
  impl_->Execute(id, std::move(context));
}
}  // namespace miet::lambda::lua
