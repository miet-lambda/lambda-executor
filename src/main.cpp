#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/client.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/dns_client_control.hpp>
#include <userver/server/handlers/dynamic_debug_log.hpp>
#include <userver/server/handlers/inspect_requests.hpp>
#include <userver/server/handlers/log_level.hpp>
#include <userver/server/handlers/on_log_rotate.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/server_monitor.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include <miet/lambda/handlers/execute-lambda.hpp>

int main(int argc, char* argv[]) {
  auto component_list =
      userver::components::MinimalServerComponentList()
          .Append<userver::server::handlers::Ping>()
          .Append<userver::server::handlers::TestsControl>()
          .Append<userver::server::handlers::ServerMonitor>()
          .Append<userver::server::handlers::LogLevel>()
          .Append<userver::server::handlers::OnLogRotate>()
          .Append<userver::server::handlers::InspectRequests>()
          .Append<userver::server::handlers::DnsClientControl>()
          .Append<userver::server::handlers::DynamicDebugLog>()
          .Append<userver::clients::dns::Component>()
          .Append<userver::components::HttpClient>()
          .Append<userver::components::TestsuiteSupport>();

  miet::lambda::handlers::AppendExecuteLambda(component_list);

  return userver::utils::DaemonMain(argc, argv, component_list);
}
