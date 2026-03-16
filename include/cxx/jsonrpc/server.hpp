#pragma once

#include "cxx/jsonrpc/dispatcher.hpp"
#include "cxx/jsonrpc/export.hpp"

#include <initializer_list>
#include <string>

namespace jsonrpc
{

class JsonRpcServer
{
public:
#if !defined( _WIN32 )
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Weffc++"
#endif
  CXX_JSONRPC_API explicit JsonRpcServer() = default;
#if !defined( _WIN32 )
  #pragma GCC diagnostic pop
#endif
  CXX_JSONRPC_API ~JsonRpcServer() noexcept = default;

  CXX_JSONRPC_API bool Add( const std::string& name, Method callback, const std::initializer_list<std::string>& mapping = {} )
  {
    if( name.rfind( "rpc.", 0 ) == 0 ) return false;
    if( mapping.size() == 0 ) return m_dispatcher.Add( name, std::move( callback ), std::nullopt );
    else
      return m_dispatcher.Add( name, std::move( callback ), mapping );
  }

  CXX_JSONRPC_API bool Add( const std::string& name, Notification callback, const std::initializer_list<std::string>& mapping = {} )
  {
    if( name.rfind( "rpc.", 0 ) == 0 ) return false;
    if( mapping.size() == 0 ) return m_dispatcher.Add( name, std::move( callback ), std::nullopt );
    else
      return m_dispatcher.Add( name, std::move( callback ), mapping );
  }

  CXX_JSONRPC_API nlohmann::json getMethodList() const { return m_dispatcher.listMethods(); }

  CXX_JSONRPC_API nlohmann::json getNotificationList() const { return m_dispatcher.listNotifications(); }

  CXX_JSONRPC_API nlohmann::json getProcedures() { return m_dispatcher.listProcedures(); }

  CXX_JSONRPC_API std::string HandleRequest( const std::string& requestString )
  {
    try
    {
      nlohmann::json request = nlohmann::json::parse( requestString );

      if( request.is_array() )
      {
        nlohmann::json result = nlohmann::json::array();
        for( nlohmann::json& r: request )
        {
          nlohmann::json res = this->HandleSingleRequest( r );
          if( !res.is_null() ) result.push_back( std::move( res ) );
        }
        return result.dump();
      }
      else if( request.is_object() )
      {
        const nlohmann::json res = HandleSingleRequest( request );
        if( !res.is_null() ) return res.dump();
        else
          return {};
      }
      else
        return nlohmann::json{ { "id", nullptr }, { "error", { { "code", invalid_request }, { "message", "invalid request: expected array or object" } } }, { "jsonrpc", "2.0" } }.dump();
    }
    catch( const nlohmann::json::parse_error& e )
    {
      return nlohmann::json{ { "id", nullptr }, { "error", { { "code", parse_error }, { "message", std::string( "parse error: " ) + e.what() } } }, { "jsonrpc", "2.0" } }.dump();
    }
  }

protected:
  Dispatcher m_dispatcher;

private:
  nlohmann::json HandleSingleRequest( nlohmann::json& request )
  {
    nlohmann::json id = nullptr;
    if( request.contains( "id" ) && ( request["id"].is_number() || request["id"].is_string() || request["id"].is_null() ) ) { id = request["id"]; }
    try
    {
      return ProcessSingleRequest( request );
    }
    catch( const exception& e )
    {
      nlohmann::json error = { { "code", e.code() }, { "message", e.message() } };
      if( !e.data().empty() ) error["data"] = nlohmann::json::parse( e.data() );
      return nlohmann::json{ { "id", id }, { "error", error }, { "jsonrpc", "2.0" } };
    }
    catch( const std::exception& e )
    {
      return nlohmann::json{ { "id", id }, { "error", { { "code", internal_error }, { "message", std::string( "internal server error: " ) + e.what() } } }, { "jsonrpc", "2.0" } };
    }
    catch( ... )
    {
      return nlohmann::json{ { "id", id }, { "error", { { "code", internal_error }, { "message", std::string( "internal server error" ) } } }, { "jsonrpc", "2.0" } };
    }
  }

  nlohmann::json ProcessSingleRequest( nlohmann::json& request )
  {
    if( !request.contains( "jsonrpc" ) || !request["jsonrpc"].is_string() || request["jsonrpc"] != "2.0" ) throw exception( invalid_request, R"(invalid request: missing jsonrpc field set to "2.0")" );
    if( !request.contains( "method" ) || !request["method"].is_string() ) throw exception( invalid_request, "invalid request: method field must be a string" );
    if( request.contains( "id" ) && !( request.contains( "id" ) && ( request["id"].is_number() || request["id"].is_string() || request["id"].is_null() ) ) ) throw exception( invalid_request, "invalid request: id field must be a number, string or null" );
    if( request.contains( "params" ) && !( request["params"].is_array() || request["params"].is_object() || request["params"].is_null() ) ) throw exception( invalid_request, "invalid request: params field must be an array, object or null" );
    if( !request.contains( "params" ) || request["params"].is_null() ) request["params"] = nlohmann::json::array();
    if( !request.contains( "id" ) )
    {
      try
      {
        m_dispatcher.InvokeNotification( request["method"], request["params"] );
        return nlohmann::json();
      }
      catch( const std::exception& )
      {
        return nlohmann::json();
      }
    }
    else
      return { { "jsonrpc", "2.0" }, { "id", request["id"] }, { "result", m_dispatcher.InvokeMethod( request["method"], request["params"] ) } };
  }
};

}  // namespace jsonrpc
