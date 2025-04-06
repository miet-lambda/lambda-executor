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


add_library(${PROJECT_NAME}-objs OBJECT
    src/miet/lambda/base/executor.hpp
    src/miet/lambda/base/scripts-fetcher.hpp

    src/miet/lambda/http.hpp
    src/miet/lambda/http.cpp
    src/miet/lambda/http-client.hpp
    src/miet/lambda/http-client.cpp
    src/miet/lambda/key-value-storage.hpp
    src/miet/lambda/key-value-storage.cpp
    src/miet/lambda/execution-context.hpp
    src/miet/lambda/execution-context.cpp
    src/miet/lambda/components/executor.hpp
    src/miet/lambda/components/executor.cpp
    src/miet/lambda/components/instance-registrator.hpp
    src/miet/lambda/components/instance-registrator.cpp
    src/miet/lambda/fetchers/db-fetcher.hpp
    src/miet/lambda/fetchers/db-fetcher.cpp

    src/miet/lambda/lua/utils.hpp
    src/miet/lambda/lua/utils.cpp
    src/miet/lambda/lua/executor.hpp
    src/miet/lambda/lua/executor.cpp
    src/miet/lambda/lua/http-context.hpp
    src/miet/lambda/lua/http-context.cpp
    src/miet/lambda/lua/storage.hpp
    src/miet/lambda/lua/storage.cpp
    src/miet/lambda/lua/http-client.hpp
    src/miet/lambda/lua/http-client.cpp

    src/miet/lambda/handlers/execute-lambda.hpp
    src/miet/lambda/handlers/execute-lambda.cpp
)
target_include_directories(${PROJECT_NAME}-objs PUBLIC src/)
target_link_libraries(${PROJECT_NAME}-objs PUBLIC
    userver::postgresql
    lua5.4
    luacpp
)


add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}-objs)
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/lua/
                  $<TARGET_FILE_DIR:${PROJECT_NAME}>
)


add_executable(${PROJECT_NAME}-unittest
    src/ut/http-request-ut.cpp 
    src/ut/http-response-ut.cpp 
   
    src/ut/lua-executor-ut.cpp
)
target_link_libraries(${PROJECT_NAME}-unittest PRIVATE ${PROJECT_NAME}-objs userver::utest)
add_google_tests(${PROJECT_NAME}-unittest)
add_custom_command(TARGET ${PROJECT_NAME}-unittest PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/lua/
                  $<TARGET_FILE_DIR:${PROJECT_NAME}-unittest>
)


add_executable(${PROJECT_NAME}-benchmark
    src/benchmarks/execute-lua.cpp
)
target_link_libraries(${PROJECT_NAME}-benchmark PRIVATE ${PROJECT_NAME}-objs userver::ubench)
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
