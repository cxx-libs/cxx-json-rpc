#pragma once

#if defined( _WIN32 ) || defined( _WIN64 )
  // On Windows, use __declspec(dllexport/dllimport)
  #ifdef CXX_JSONRPC_BUILD_DLL
    #define CXX_JSONRPC_API __declspec( dllexport )
  #else
    #define CXX_JSONRPC_API __declspec( dllimport )
  #endif
#else
  #if defined( __GNUC__ ) || defined( __clang__ )
    #define CXX_JSONRPC_API __attribute__( ( visibility( "default" ) ) )
  #else
    #define CXX_JSONRPC_API
  #endif
#endif

#if defined( __GNUC__ ) || defined( __clang__ )
  #define CXX_JSONRPC_INTERNAL __attribute__( ( visibility( "hidden" ) ) )
#else
  #define CXX_JSONRPC_INTERNAL
#endif
