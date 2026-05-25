cmake_minimum_required(VERSION 3.25)

project(cxx-jsonrpc VERSION 0.0.1 LANGUAGES CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  option(CXX_JSONRPC_TESTS "Enable tests" ON)
  option(CXX_JSONRPC_EXAMPLES "Enable examples" ON)
  option(CXX_JSONRPC_CODE_COVERAGE "Enable coverage reporting" OFF)
  option(CXX_JSONRPC_ALLOW_FETCHING "Allowing fetching the vendors" ON)
else()
  option(CXX_JSONRPC_TESTS "Enable tests" OFF)
  option(CXX_JSONRPC_EXAMPLES "Enable examples" OFF)
  option(CXX_JSONRPC_CODE_COVERAGE "Enable coverage reporting" OFF)
  option(CXX_JSONRPC_ALLOW_FETCHING "Allowing fetching the vendors" OFF)
endif()

include(GNUInstallDirs)

if(NOT TARGET nlohmann_json::nlohmann_json)
  find_package(nlohmann_json QUIET)
  if(CXX_JSONRPC_ALLOW_FETCHING)
    include(FetchContent)
    FetchContent_Declare(nlohmann_json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.12.0 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(nlohmann_json)
  endif()
endif()

if(NOT TARGET simdjson::simdjson)
  find_package(simdjson QUIET)
  if(CXX_JSONRPC_ALLOW_FETCHING)
    include(FetchContent)
    FetchContent_Declare(simdjson GIT_REPOSITORY https://github.com/simdjson/simdjson.git GIT_TAG v4.6.4  GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(simdjson)
  endif()
endif()

string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT}")
string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_DEBUG}")
string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "${CMAKE_CXX_FLAGS_DEBUG_INIT}")
string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS_DEBUG_INIT "${CMAKE_CXX_FLAGS_DEBUG_INIT}")
string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAG_RELEASE_INIT "${CMAKE_CXX_FLAGS_RELEASE_INIT}")
string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")
string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS_MINSIZEREL_INIT "${CMAKE_CXX_FLAG_MINSIZEREL_INIT}")

# Warning options for the compiler
string(
        APPEND _warning_opts
        "$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wall;-Wextra;-Werror;>"
        "$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wthread-safety;-Wpedantic;>"
        "$<$<CXX_COMPILER_ID:GNU>:-pedantic;-pedantic-errors;>"
        "$<$<CXX_COMPILER_ID:MSVC>:/permissive-;/analyze>"
)

add_library(cxx-jsonrpc INTERFACE)
target_include_directories(cxx-jsonrpc INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(cxx-jsonrpc INTERFACE cxx_std_17)
target_compile_definitions(cxx-jsonrpc INTERFACE CXX_JSONRPC_EXPORT_DLL)
target_link_libraries(cxx-jsonrpc INTERFACE nlohmann_json::nlohmann_json simdjson::simdjson)
set(CXX_JSONRPC_PUBLIC_HEADERS
"${PROJECT_SOURCE_DIR}/include/cxx/jsonrpc/batch.hpp"
"${PROJECT_SOURCE_DIR}/include/cxx/jsonrpc/client.hpp"
"${PROJECT_SOURCE_DIR}/include/cxx/jsonrpc/dispatcher.hpp"
"${PROJECT_SOURCE_DIR}/include/cxx/jsonrpc/error_code.hpp"
"${PROJECT_SOURCE_DIR}/include/cxx/jsonrpc/export.hpp"
"${PROJECT_SOURCE_DIR}/include/cxx/jsonrpc/exception.hpp"
"${PROJECT_SOURCE_DIR}/include/cxx/jsonrpc/iclientconnector.hpp"
"${PROJECT_SOURCE_DIR}/include/cxx/jsonrpc/id.hpp"
"${PROJECT_SOURCE_DIR}/include/cxx/jsonrpc/server.hpp"
"${PROJECT_SOURCE_DIR}/include/cxx/jsonrpc/typemapper.hpp"
)
set_target_properties(cxx-jsonrpc PROPERTIES PUBLIC_HEADER "${CXX_JSONRPC_PUBLIC_HEADERS}")
add_library(cxx-jsonrpc::cxx-jsonrpc ALIAS cxx-jsonrpc)

install(TARGETS cxx-jsonrpc nlohmann_json simdjson
        EXPORT cxx-jsonrpcTargets
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/cxx/jsonrpc"
)
install(EXPORT cxx-jsonrpcTargets
        FILE cxx-jsonrpcTargets.cmake
        NAMESPACE cxx-jsonrpc::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/cxx-jsonrpc"
)

include(CMakePackageConfigHelpers)
configure_package_config_file("${PROJECT_SOURCE_DIR}/cmake/cxx-jsonrpcConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/cmake/cxx-jsonrpcConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cxx-jsonrpc
)
write_basic_package_version_file("${PROJECT_BINARY_DIR}/cmake/cxx-jsonrpcConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY AnyNewerVersion
)
install(FILES
        "${PROJECT_BINARY_DIR}/cmake/cxx-jsonrpcConfig.cmake"
        "${PROJECT_BINARY_DIR}/cmake/cxx-jsonrpcConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cxx-jsonrpc
)

if(CXX_JSONRPC_TESTS)
  if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(CTest)
  endif()
  enable_testing()
  add_subdirectory(test)
endif()

if(CXX_JSONRPC_EXAMPLES)
  add_subdirectory(examples)
endif()
