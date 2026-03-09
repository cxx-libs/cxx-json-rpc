cmake_minimum_required(VERSION 3.14)

project(json-rpc-cxx VERSION 0.0.1 LANGUAGES CXX)

option(COMPILE_TESTS "Enable tests" ON)
option(COMPILE_EXAMPLES "Enable examples" ON)
option(CODE_COVERAGE "Enable coverage reporting" OFF)
option(PREFER_SYSTEM_NLOHMANN_JSON "find system nlohmann_json" OFF)

include(GNUInstallDirs)

if(NOT TARGET nlohmann_json::nlohmann_json)
  if(PREFER_SYSTEM_NLOHMANN_JSON)
    find_package(nlohmann_json REQUIRED)
  else()
    include(FetchContent)
    FetchContent_Declare(nlohmann_json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.12.0)
    FetchContent_MakeAvailable(nlohmann_json)
  endif()
endif()

add_library(json-rpc-cxx INTERFACE)
target_include_directories(json-rpc-cxx INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(json-rpc-cxx INTERFACE cxx_std_17)
target_link_libraries(json-rpc-cxx INTERFACE nlohmann_json::nlohmann_json)
set_target_properties(json-rpc-cxx PROPERTIES PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/include/jsonrpccxx/batchclient.hpp;${PROJECT_SOURCE_DIR}/include/jsonrpccxx/client.hpp;${PROJECT_SOURCE_DIR}/include/jsonrpccxx/common.hpp;${PROJECT_SOURCE_DIR}/include/jsonrpccxx/dispatcher.hpp;${PROJECT_SOURCE_DIR}/include/jsonrpccxx/iclientconnector.hpp;${PROJECT_SOURCE_DIR}/include/jsonrpccxx/server.hpp;${PROJECT_SOURCE_DIR}/include/jsonrpccxx/typemapper.hpp")
add_library(json-rpc-cxx::json-rpc-cxx ALIAS json-rpc-cxx)

install(TARGETS json-rpc-cxx nlohmann_json
        EXPORT jsonrpccxxTargets
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/jsonrpccxx"
)
install(EXPORT jsonrpccxxTargets
        FILE jsonrpccxxTargets.cmake
        NAMESPACE json-rpc-cxx::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/jsonrpccxx"
)


include(CMakePackageConfigHelpers)
configure_package_config_file("${PROJECT_SOURCE_DIR}/cmake/jsonrpccxxConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/cmake/jsonrpccxxConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jsonrpccxx
)
write_basic_package_version_file("${PROJECT_BINARY_DIR}/cmake/jsonrpccxxConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY AnyNewerVersion
)
install(FILES
        "${PROJECT_BINARY_DIR}/cmake/jsonrpccxxConfig.cmake"
        "${PROJECT_BINARY_DIR}/cmake/jsonrpccxxConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jsonrpccxx
)

# Warning options for the compiler
string(
        APPEND _warning_opts
        "$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wall;-Wextra;-Weffc++;-Werror;>"
        "$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wthread-safety;-Wpedantic;>"
        "$<$<CXX_COMPILER_ID:GNU>:-pedantic;-pedantic-errors;>"
)


if (COMPILE_TESTS)
    add_library(coverage_config INTERFACE)
    if (CODE_COVERAGE)
        message("Enabled coverage flags")
        target_compile_options(coverage_config INTERFACE -O0 -g --coverage)
        target_link_libraries(coverage_config INTERFACE --coverage)
    endif ()
    add_executable(jsonrpccpp-test test/main.cpp test/client.cpp test/typemapper.cpp test/dispatcher.cpp test/server.cpp test/batchclient.cpp test/testclientconnector.hpp examples/warehouse/warehouseapp.cpp test/warehouseapp.cpp test/common.cpp)
    target_compile_options(jsonrpccpp-test PUBLIC "${_warning_opts}")
    target_include_directories(jsonrpccpp-test PRIVATE vendor examples)
    target_link_libraries(jsonrpccpp-test coverage_config json-rpc-cxx::json-rpc-cxx)
    enable_testing()
    add_test(NAME test COMMAND jsonrpccpp-test)
endif ()

if (COMPILE_EXAMPLES)
    find_package(Threads)
    add_executable(example-warehouse examples/warehouse/main.cpp examples/warehouse/warehouseapp.cpp examples/warehouse/types.h examples/inmemoryconnector.hpp)
    target_compile_options(example-warehouse PUBLIC "${_warning_opts}")
    target_link_libraries(example-warehouse json-rpc-cxx::json-rpc-cxx Threads::Threads)
    target_include_directories(example-warehouse SYSTEM PRIVATE vendor)
    target_include_directories(example-warehouse PRIVATE examples)
    add_test(NAME example COMMAND example-warehouse)
endif ()
