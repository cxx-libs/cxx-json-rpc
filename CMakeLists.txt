cmake_minimum_required(VERSION 3.14)

project(json-rpc-cxx VERSION 0.0.1 LANGUAGES CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  option(JSONRPCCXX_TESTS "Enable tests" ON)
  option(JSONRPCCXX_EXAMPLES "Enable examples" ON)
  option(JSONRPCCXX_CODE_COVERAGE "Enable coverage reporting" OFF)
  option(JSONRPCCXX_ALLOW_FETCHING "Allowing fetching the vendors" ON)
else()
  option(JSONRPCCXX_TESTS "Enable tests" OFF)
  option(JSONRPCCXX_EXAMPLES "Enable examples" OFF)
  option(JSONRPCCXX_CODE_COVERAGE "Enable coverage reporting" OFF)
  option(JSONRPCCXX_ALLOW_FETCHING "Allowing fetching the vendors" OFF)
endif()

include(GNUInstallDirs)

if(NOT TARGET nlohmann_json::nlohmann_json)
  find_package(nlohmann_json QUIET)
  if(JSONRPCCXX_ALLOW_FETCHING)
    include(FetchContent)
    FetchContent_Declare(nlohmann_json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.12.0)
    FetchContent_MakeAvailable(nlohmann_json)
  endif()
endif()

add_library(json-rpc-cxx INTERFACE)
target_include_directories(json-rpc-cxx INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(json-rpc-cxx INTERFACE cxx_std_17)
target_link_libraries(json-rpc-cxx INTERFACE nlohmann_json::nlohmann_json)
set_target_properties(json-rpc-cxx PROPERTIES PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/include/jsonrpccxx/batchclient.hpp;${PROJECT_SOURCE_DIR}/include/jsonrpccxx/client.hpp;${PROJECT_SOURCE_DIR}/include/jsonrpccxx/exception.hpp;${PROJECT_SOURCE_DIR}/include/jsonrpccxx/dispatcher.hpp;${PROJECT_SOURCE_DIR}/include/jsonrpccxx/iclientconnector.hpp;${PROJECT_SOURCE_DIR}/include/jsonrpccxx/server.hpp;${PROJECT_SOURCE_DIR}/include/jsonrpccxx/typemapper.hpp")
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


if(JSONRPCCXX_TESTS)
  if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(CTest)
  endif()
  enable_testing()
  add_subdirectory(test)
endif()

if(JSONRPCCXX_EXAMPLES)
  add_subdirectory(examples)
endif()
