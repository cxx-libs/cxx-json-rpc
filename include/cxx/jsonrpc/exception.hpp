#pragma once
#include "cxx/jsonrpc/error_code.hpp"

#include <exception>
#include <string>

namespace jsonrpc
{
class exception : public std::exception
{
public:
#if !defined( _WIN32 )
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Weffc++"
#endif
  exception( int code, const std::string& message ) noexcept : m_code( code ), m_message( message ), m_err( std::to_string( code ) + ": " + message ) {}
  exception( int code, const std::string& message, const std::string& data ) noexcept : m_code( code ), m_message( message ), m_data( data ), m_err( std::to_string( code ) + ": " + message + ", data: " + data ) {}
#if !defined( _WIN32 )
  #pragma GCC diagnostic pop
#endif

  error_type type() const
  {
    if( m_code >= -32603 && m_code <= -32600 ) return static_cast<error_type>( m_code );
    if( m_code >= -32099 && m_code <= -32000 ) return server_error;
    if( m_code == -32700 ) return parse_error;
    return invalid;
  }

  int                code() const { return m_code; }
  const std::string& message() const { return m_message; }
  const std::string& data() const { return m_data; }

  const char* what() const noexcept override { return m_err.c_str(); }

private:
  int         m_code{ 0 };
  std::string m_message;
  std::string m_data;
  std::string m_err;
};

}  // namespace jsonrpc
