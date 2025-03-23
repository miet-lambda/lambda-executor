#include <miet/lambda/handlers/execute-lambda.hpp>

#include <benchmark/benchmark.h>
#include <userver/engine/run_standalone.hpp>
#include <userver/fs/blocking/read.hpp>

#include <LuaCpp/LuaCpp.hpp>

constexpr auto kJsonSamplePath = "data/sample.json";

static void ExecuteLuaBenchmark(benchmark::State& state) {
  userver::engine::RunStandalone([&] {
    constexpr std::uint64_t kArgs[] = {11, 12, 13, 14, 15};
    std::uint64_t i = 0;

    LuaCpp::LuaContext ctx;
    ctx.CompileString("benchmark", R"(
      function factorial(n)
        if n == 0 or n == 1
          then return 1
        end
        return n * factorial(n - 1)
      end

      res = factorial(N)
      sum = 0
      for i = 0, res, 1 do
        sum = sum + i
      end
    )");
    const auto NVar = std::make_shared<LuaCpp::Engine::LuaTNumber>(0);
    ctx.AddGlobalVariable("N", NVar);

    for (auto _ : state) {
      const auto N = kArgs[i % std::size(kArgs)];
      NVar->setValue(N);
      ctx.Run("benchmark");
      i++;
    }
  });
}

static void LuaJsonBenchmark(benchmark::State& state) {
  userver::engine::RunStandalone([&] {
    const auto jsonData =
        userver::fs::blocking::ReadFileContents(kJsonSamplePath);

    const auto jsonVar = std::make_shared<LuaCpp::Engine::LuaTString>(jsonData);

    LuaCpp::LuaContext ctx;
    ctx.AddGlobalVariable("json_string", jsonVar);
    ctx.CompileString("benchmark-json", R"(
          local json = require("dkjson")

          local decoded_data, pos, err = json.decode(json_string, 1, nil)
          local encoded_data = json.encode(data, { indent = true })
    )");

    for (auto _ : state) {
      ctx.Run("benchmark-json");
    }
  });
}

BENCHMARK(ExecuteLuaBenchmark);
BENCHMARK(LuaJsonBenchmark);
