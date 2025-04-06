#include <miet/lambda/lua/executor.hpp>

#include <miet/lambda/exceptions.hpp>

#include <miet/lambda/lua/http-client.hpp>
#include <miet/lambda/lua/http-context.hpp>
#include <miet/lambda/lua/storage.hpp>

#include <userver/concurrent/variable.hpp>
#include <userver/engine/shared_mutex.hpp>

#include <LuaCpp/LuaCpp.hpp>

#include <unordered_map>

namespace miet::lambda::lua {
constexpr auto kInstructionsBeforeHoock = 100;
constexpr auto kTimeoutCheckerVariableName = "miet_execution_timeout_checker";

constexpr auto kHttpContextVariableName = "miet_http_context";
constexpr auto kHttpClientVariableName = "miet_http_client";
constexpr auto kStorageVariableName = "miet_kv_storage";

class LuaTimeoutChecker final : public LuaCpp::LuaMetaObject {
 public:
  explicit LuaTimeoutChecker(TimeoutCheckerPtr checker) noexcept
      : checker_(std::move(checker)) {}

  bool IsExpired() const noexcept { return checker_->IsExpired(); }

  static void Check(lua_State* L, [[maybe_unused]] lua_Debug* ar) {
    lua_getglobal(L, kTimeoutCheckerVariableName);
    void* checkerPtr = lua_touserdata(L, -1);
    const auto& checker = **reinterpret_cast<LuaTimeoutChecker**>(checkerPtr);
    if (checker.IsExpired()) {
      throw ExecutionTimout("Execution timeout is expired");
    }
    lua_pop(L, 1);
  }

 private:
  TimeoutCheckerPtr checker_ = nullptr;
};

class Executor::Impl {
 public:
  Impl(ScriptsFetcherPtr fetcher, TimeoutCheckersFactoryPtr checkersFactory,
       Dependencies deps)
      : fetcher_(std::move(fetcher)),
        checkersFactory_(std::move(checkersFactory)),
        deps_(std::move(deps)),
        luaContext_(userver::utils::MakeSharedRef<LuaCpp::LuaContext>()) {
    luaContext_->addHook(LuaTimeoutChecker::Check, "count",
                         kInstructionsBeforeHoock);
  }

  void Execute(std::string_view id, ExecutionContextRef context) {
    std::optional<std::int64_t> projectIdOpt;
    {
      const auto lock = compiledStripts_.SharedLock();
      const auto it = lock->find(id.data());
      if (it != lock->cend()) {
        projectIdOpt = it->second;
      }
    }
    if (!projectIdOpt) [[likely]] {
      const auto [projectId, scriptSource] = fetcher_->Fetch(id);
      auto lock = compiledStripts_.Lock();
      if (!lock->contains(id.data())) {
        luaContext_->CompileString(id.data(), scriptSource);
        lock->emplace(id.data(), projectId);
      }
      projectIdOpt = projectId;
    }

    const auto luaEnv = std::make_shared<LuaCpp::LuaEnvironment>();

    const auto checker =
        checkersFactory_->CreateChecker(context->GetOptions().timeout);
    const auto luaChecker = std::make_shared<LuaTimeoutChecker>(checker);
    luaEnv->emplace(kTimeoutCheckerVariableName, luaChecker);

    const auto httpContext = std::make_shared<HttpContext>(luaEnv, context);
    luaEnv->emplace(kHttpContextVariableName, httpContext);
    if (deps_.httpClient) {
      const auto httpClient = std::make_shared<HttpClient>(deps_.httpClient);
      luaEnv->emplace(kHttpClientVariableName, httpClient);
    }
    if (deps_.kvStorage) {
      UINVARIANT(projectIdOpt, "Project ID is nullopt");
      const auto storage =
          std::make_shared<Storage>(*projectIdOpt, deps_.kvStorage);
      luaEnv->emplace(kStorageVariableName, storage);
    }
    checker->Start();
    luaContext_->RunWithEnvironment(id.data(), *luaEnv);
    httpContext->PopulateResponse();
  }

 private:
  using ScriptsMap = userver::concurrent::Variable<
      std::unordered_map<std::string, std::int64_t>,
      userver::engine::SharedMutex>;

  ScriptsFetcherPtr fetcher_;
  TimeoutCheckersFactoryPtr checkersFactory_;
  Dependencies deps_;
  LuaContextRef luaContext_;
  ScriptsMap compiledStripts_;
};

Executor::Executor(ScriptsFetcherPtr fetcher,
                   TimeoutCheckersFactoryPtr checkersFactory, Dependencies deps)
    : impl_(std::move(fetcher), std::move(checkersFactory), std::move(deps)) {}

Executor::~Executor() = default;

void Executor::Execute(std::string_view id, ExecutionContextRef context) {
  impl_->Execute(id, std::move(context));
}
}  // namespace miet::lambda::lua
