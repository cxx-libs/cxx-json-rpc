#pragma once
#include "cxx/jsonrpc/error_code.hpp"
#include "cxx/jsonrpc/export.hpp"

#include <exception>
#include <string>

namespace jsonrpc
{

class CXX_JSONRPC_API exception : public std::exception
{
public:
#if !defined( _WIN32 )
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Weffc++"
#endif
  CXX_JSONRPC_API exception( int code, const std::string& message ) noexcept : m_code( code ), m_message( message ), m_err( std::to_string( code ) + ": " + message ) {}
  CXX_JSONRPC_API exception( int code, const std::string& message, const std::string& data ) noexcept : m_code( code ), m_message( message ), m_data( data ), m_err( std::to_string( code ) + ": " + message + ", data: " + data ) {}
#if !defined( _WIN32 )
  #pragma GCC diagnostic pop
#endif

  CXX_JSONRPC_API error_type type() const
  {
    if( m_code >= -32603 && m_code <= -32600 ) return static_cast<error_type>( m_code );
    if( m_code >= -32099 && m_code <= -32000 ) return server_error;
    if( m_code == -32700 ) return parse_error;
    return invalid;
  }

  CXX_JSONRPC_API int   code() const { return m_code; }
  CXX_JSONRPC_API const std::string& message() const { return m_message; }
  CXX_JSONRPC_API const std::string& data() const { return m_data; }

  CXX_JSONRPC_API const char* what() const noexcept override { return m_err.c_str(); }

private:
  int         m_code{ 0 };
  std::string m_message;
  std::string m_data;
  std::string m_err;
};

}  // namespace jsonrpc
