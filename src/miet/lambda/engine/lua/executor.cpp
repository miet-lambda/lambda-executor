#include <miet/lambda/engine/lua/executor.hpp>

#include <miet/lambda/exceptions.hpp>

#include <miet/lambda/engine/lua/modules/http/client.hpp>
#include <miet/lambda/engine/lua/modules/http/context.hpp>
#include <miet/lambda/engine/lua/modules/kv/storage.hpp>
#include <miet/lambda/engine/lua/script.hpp>

#include <LuaCpp/LuaContext.hpp>
#include <LuaCpp/LuaVersion.hpp>

namespace miet::lambda::engine::lua {
constexpr auto kInstructionsBeforeHoock = 100;
constexpr auto kTimeoutCheckerVariableName = "miet_execution_timeout_checker";

constexpr auto kHttpClientVariableName = "miet_http_client";
constexpr auto kStorageVariableName = "miet_kv_storage";

static void Check(lua_State* L, [[maybe_unused]] lua_Debug* ar) {
  lua_pushstring(L, kTimeoutCheckerVariableName);
  lua_gettable(L, LUA_REGISTRYINDEX);
  void* checkerPtr = lua_touserdata(L, -1);
  const auto& checker =
      **reinterpret_cast<execution::control::TimeoutCheckerBase**>(checkerPtr);
  if (checker.IsExpired()) {
    throw ExecutionTimeout("Execution timeout is expired");
  }
  lua_pop(L, 1);
}

static void* LuaAllocator(void* ud, void* ptr, size_t osize, size_t nsize) {
  auto* checker =
      reinterpret_cast<execution::control::MemoryLimitCheckerBase*>(ud);
  UINVARIANT(checker, "Memory limit checker is NULL");
  if (ptr == nullptr) {
    osize = 0;
  }
  if (nsize == 0) {
    free(ptr);
    checker->Deallocated(osize);
    return nullptr;
  }
  const auto allocated = nsize > osize ? nsize - osize : osize - nsize;
  checker->Allocated(allocated);
  if (checker->IsLimitReached()) [[unlikely]] {
    return nullptr;
  }
  return realloc(ptr, nsize);
}

static void RemoveUnsafeObjects(LuaCpp::LuaEnvironment& env) {
  static constexpr auto unsafeObjects = {
      "os",         "io",     "package", "load",    "loadfile",
      "loadstring", "dofile", "getfenv", "setfenv",
#ifdef NDEBUG
      "print",      "error",  "debug"
#endif
  };
  for (const auto& objectName : unsafeObjects) {
    env.emplace(objectName, std::make_shared<LuaCpp::Engine::LuaTNil>());
  }
}

static std::unique_ptr<LuaCpp::Engine::LuaState> CreateLuaState(
    LuaCpp::Engine::StateParams params) {
  auto L = std::make_unique<LuaCpp::Engine::LuaState>(std::move(params));
  luaL_openlibs(*L);
  lua_pushstring(*L, LuaCpp::Version);
  lua_setglobal(*L, "_luacppversion");
  return L;
}

static void AttachTimeoutChecker(
    lua_State* L, execution::control::TimeoutCheckerPtr timeoutChecker) {
  lua_pushstring(L, kTimeoutCheckerVariableName);
  auto** data = (execution::control::TimeoutCheckerBase**)lua_newuserdata(
      L, sizeof(execution::control::TimeoutCheckerBase*));
  *data = timeoutChecker.get();
  lua_settable(L, LUA_REGISTRYINDEX);
  lua_sethook(L, Check, LUA_MASKCOUNT, kInstructionsBeforeHoock);
}

class ExecutionGuard final {
 public:
  ExecutionGuard(LuaCpp::Engine::LuaState& luaState,
                 LuaCpp::LuaEnvironment& luaEnv,
                 ExecutionContextRef execContext)
      : luaState_(luaState), luaEnv_(luaEnv) {
    context = std::make_unique<modules::http::Context>(luaEnv, execContext);
    for (const auto& [name, variable] : luaEnv_) {
      variable->PushGlobal(luaState_, name);
    }
  }

  ~ExecutionGuard() {
    if (std::uncaught_exceptions()) {
      return;
    }
    for (const auto& [name, variable] : luaEnv_) {
      variable->PopGlobal(luaState_);
    }
  }

 private:
  LuaCpp::Engine::LuaState& luaState_;
  LuaCpp::LuaEnvironment& luaEnv_;
  std::unique_ptr<modules::http::Context> context;
};

Executor::Executor(ExecutionControlParams execCtlParams,
                   ModulesDeps deps) noexcept
    : execCtlParams_(std::move(execCtlParams)), modulesDeps_(std::move(deps)) {}

void Executor::Execute(ScriptPtr script, ExecutionContextRef context) {
  UINVARIANT(script, "Script is NULL");
  UINVARIANT(script->GetLanguage() == ScriptLanguage::Lua,
             "Incorrect script language");

  const auto luaScript = std::static_pointer_cast<Script>(script);

  const auto memoryChecker =
      execCtlParams_.memLimitCheckersFactory->CreateChecker(
          context->GetOptions().memoryLimit);

  const auto luaState = CreateLuaState(
      {.allocator = LuaAllocator, .userData = memoryChecker.get()});
  luaScript->GetCodeSnippet().UploadCode(*luaState);

  const auto luaEnv = std::make_shared<LuaCpp::LuaEnvironment>();
  if (modulesDeps_.httpClient) {
    const auto httpClient =
        std::make_shared<modules::http::Client>(modulesDeps_.httpClient);
    luaEnv->emplace(kHttpClientVariableName, httpClient);
  }
  if (modulesDeps_.projectStorage) {
    const auto storage = std::make_shared<modules::kv::Storage>(
        script->GetProjectId(), modulesDeps_.projectStorage);
    luaEnv->emplace(kStorageVariableName, storage);
  }

  const auto timeoutChecker =
      execCtlParams_.timeoutCheckersFactory->CreateChecker(
          context->GetOptions().timeout);

  AttachTimeoutChecker(*luaState, timeoutChecker);

  RemoveUnsafeObjects(*luaEnv);

  ExecutionGuard guard(*luaState, *luaEnv, context);

  timeoutChecker->Start();

  const auto res = lua_pcall(*luaState, 0, LUA_MULTRET, 0);
  if (res != LUA_OK) {
    const auto errorMessage = lua_tostring(*luaState, 1);
    if (res == LUA_ERRMEM) {
      throw OutOfMemoryLimit(errorMessage);
    }
    throw ExecutionError(errorMessage);
  }
}
}  // namespace miet::lambda::engine::lua
