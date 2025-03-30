#include <miet/lambda/lua/executor.hpp>

#include <miet/lambda/lua/http-client.hpp>
#include <miet/lambda/lua/http-context.hpp>

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
    if (!idsOfCompiledStripts_.contains(id.data())) {
      const auto scriptSource = fetcher_->Fetch(id);
      luaContext_->CompileString(id.data(), scriptSource);
      idsOfCompiledStripts_.insert(id.data());
    }
    const auto httpContext =
        std::make_shared<HttpContext>(luaContext_.GetBase(), context);
    luaContext_->AddGlobalVariable(kHttpContextVariableName, httpContext);
    if (deps_.httpClient) {
      const auto httpClient = std::make_shared<HttpClient>(deps_.httpClient);
      luaContext_->AddGlobalVariable(kHttpClientVariableName, httpClient);
    }
    luaContext_->Run(id.data());
    httpContext->PopulateResponse();
  }

 private:
  ScriptsFetcherPtr fetcher_;
  Dependencies deps_;
  LuaContextRef luaContext_;
  std::unordered_set<std::string> idsOfCompiledStripts_;
};

Executor::Executor(ScriptsFetcherPtr fetcher, Dependencies deps)
    : impl_(std::move(fetcher), std::move(deps)) {}

Executor::~Executor() = default;

void Executor::Execute(std::string_view id, ExecutionContextRef context) {
  impl_->Execute(id, std::move(context));
}
}  // namespace miet::lambda::lua
