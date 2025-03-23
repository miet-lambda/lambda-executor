#include <miet/lambda/lua/executor.hpp>

#include <miet/lambda/lua/http-context.hpp>

#include <LuaCpp/LuaCpp.hpp>

#include <unordered_set>

namespace miet::lambda::lua {
constexpr auto kHttpContextVariableName = "miet_http_context";

class Executor::Impl {
 public:
  explicit Impl(ScriptsFetcherPtr fetcher)
      : fetcher_(std::move(fetcher)),
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
    luaContext_->Run(id.data());
    httpContext->PopulateResponse();
  }

 private:
  ScriptsFetcherPtr fetcher_;
  LuaContextRef luaContext_;
  std::unordered_set<std::string> idsOfCompiledStripts_;
};

Executor::Executor(ScriptsFetcherPtr fetcher) : impl_(std::move(fetcher)) {}

Executor::~Executor() = default;

void Executor::Execute(std::string_view id, ExecutionContextRef context) {
  impl_->Execute(id, std::move(context));
}
}  // namespace miet::lambda::lua
