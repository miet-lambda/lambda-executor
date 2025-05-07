cmake_minimum_required(VERSION 3.12...3.31)

set(CMAKE_CXX_STANDARD 20)

project(lambda-executor
        LANGUAGES CXX
        DESCRIPTION "A service for running lambda-scripts on nodes"
        VERSION 0.0.1
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DownloadUserver)

find_package(userver COMPONENTS core postgresql QUIET)
if(NOT userver_FOUND)
  download_userver(TRY_DIR third_party/userver)
endif()

userver_setup_environment()

set(PATH_TO_SRC_ROOT src/miet/lambda)

add_library(${PROJECT_NAME}-context OBJECT
    ${PATH_TO_SRC_ROOT}/http/message.cpp
    ${PATH_TO_SRC_ROOT}/http/request.cpp
    ${PATH_TO_SRC_ROOT}/http/response.cpp

    ${PATH_TO_SRC_ROOT}/execution-context.cpp
)
target_include_directories(${PROJECT_NAME}-context PUBLIC src/)
target_link_libraries(${PROJECT_NAME}-context PUBLIC
    userver::core
)

add_library(${PROJECT_NAME}-core OBJECT
    ${PATH_TO_SRC_ROOT}/components/instance-registrator.cpp
    ${PATH_TO_SRC_ROOT}/components/script-engine.cpp
    ${PATH_TO_SRC_ROOT}/components/scripts-fetcher-provider.cpp
    ${PATH_TO_SRC_ROOT}/components/scripts-storage-provider.cpp

    ${PATH_TO_SRC_ROOT}/execution/control/memory-limit-checker.cpp
    ${PATH_TO_SRC_ROOT}/execution/control/timeout-checker.cpp

    ${PATH_TO_SRC_ROOT}/fetchers/db-scripts-fetcher.cpp

    ${PATH_TO_SRC_ROOT}/handlers/execute-lambda.cpp

    ${PATH_TO_SRC_ROOT}/http/client.cpp
    ${PATH_TO_SRC_ROOT}/http/message.cpp
    ${PATH_TO_SRC_ROOT}/http/request.cpp
    ${PATH_TO_SRC_ROOT}/http/response.cpp

    ${PATH_TO_SRC_ROOT}/project/storage.cpp

    ${PATH_TO_SRC_ROOT}/execution-context.cpp
    ${PATH_TO_SRC_ROOT}/scripts-storage.cpp
)
target_include_directories(${PROJECT_NAME}-core PUBLIC src/)
target_link_libraries(${PROJECT_NAME}-core PUBLIC
    userver::postgresql
    ${PROJECT_NAME}-context
)


# add_library(${PROJECT_NAME}-engine SHARED
#     ${PATH_TO_SRC_ROOT}/engine/stub/engine.cpp
# )
# target_include_directories(${PROJECT_NAME}-engine PUBLIC src/)
# target_link_libraries(${PROJECT_NAME}-engine PUBLIC
#     userver::core
#     ${PROJECT_NAME}-context
# )


add_library(${PROJECT_NAME}-engine SHARED
    ${PATH_TO_SRC_ROOT}/engine/lua/utils.cpp
    ${PATH_TO_SRC_ROOT}/engine/lua/engine.cpp
    ${PATH_TO_SRC_ROOT}/engine/lua/script.cpp
    ${PATH_TO_SRC_ROOT}/engine/lua/compiler.cpp
    ${PATH_TO_SRC_ROOT}/engine/lua/executor.cpp

    ${PATH_TO_SRC_ROOT}/engine/lua/modules/http/context.cpp
    ${PATH_TO_SRC_ROOT}/engine/lua/modules/http/client.cpp

    ${PATH_TO_SRC_ROOT}/engine/lua/modules/kv/storage.cpp
)
target_include_directories(${PROJECT_NAME}-engine PUBLIC src/)
target_link_libraries(${PROJECT_NAME}-engine PUBLIC
    userver::core
    ${PROJECT_NAME}-context
    lua5.4
    luacpp
)


add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}-core ${PROJECT_NAME}-engine)
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/lua/
                  $<TARGET_FILE_DIR:${PROJECT_NAME}>
)


add_executable(${PROJECT_NAME}-context-unittest
    src/ut/http-request-ut.cpp 
    src/ut/http-response-ut.cpp
)
target_link_libraries(${PROJECT_NAME}-context-unittest PRIVATE ${PROJECT_NAME}-context userver::utest)
add_google_tests(${PROJECT_NAME}-context-unittest)


add_executable(${PROJECT_NAME}-engine-unittest
    src/ut/lua-executor-ut.cpp
)
target_link_libraries(${PROJECT_NAME}-engine-unittest PRIVATE ${PROJECT_NAME}-engine userver::utest)
add_google_tests(${PROJECT_NAME}-engine-unittest)
add_custom_command(TARGET ${PROJECT_NAME}-engine-unittest PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/lua/
                  $<TARGET_FILE_DIR:${PROJECT_NAME}-engine-unittest>
)


add_executable(${PROJECT_NAME}-benchmark
    src/benchmarks/execute-lua.cpp
)
target_link_libraries(${PROJECT_NAME}-benchmark PRIVATE ${PROJECT_NAME}-engine userver::ubench)
add_google_benchmark_tests(${PROJECT_NAME}-benchmark)

add_custom_command(TARGET ${PROJECT_NAME}-benchmark PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/src/benchmarks/
                  $<TARGET_FILE_DIR:${PROJECT_NAME}-benchmark>
)


userver_testsuite_add_simple()


include(GNUInstallDirs)

if(DEFINED ENV{PREFIX})
  message(STATUS "Set install prefix: $ENV{PREFIX}")
  file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
  set(CMAKE_INSTALL_PREFIX ${PREFIX_PATH})
endif()

file(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})
install(FILES ${CONFIGS_FILES} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME} COMPONENT ${PROJECT_NAME})
