#pragma once

#include "cxx/jsonrpc/exception.hpp"
#include "cxx/jsonrpc/export.hpp"
#include "cxx/jsonrpc/id.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <variant>

namespace jsonrpc
{

static inline exception fromJson( const nlohmann::json& value )
{
  if( value.contains( "code" ) && value["code"].is_number_integer() && value.contains( "message" ) && value["message"].is_string() )
  {
    if( value.contains( "data" ) ) return exception( value["code"], value["message"], value["data"].get<nlohmann::json>().dump() );
    else
      return exception( value["code"], value["message"] );
  }
  return exception( internal_error, R"(invalid error response: "code" (integer number) and "message" (string) are required)" );
}

using positional_parameter = std::vector<nlohmann::json>;
using named_parameter      = std::map<std::string, nlohmann::json>;

class BatchRequest
{
public:
  CXX_JSONRPC_API BatchRequest() noexcept = default;

  CXX_JSONRPC_API BatchRequest& AddMethodCall( const id_t& id, const std::string& name, const positional_parameter& params = {} )
  {
    nlohmann::json request = { { "method", name }, { "params", params }, { "jsonrpc", "2.0" } };
    if( std::get_if<std::int64_t>( &id ) != nullptr ) request["id"] = std::get<std::int64_t>( id );
    else
      request["id"] = std::get<std::string>( id );
    call.push_back( request );
    return *this;
  }

  CXX_JSONRPC_API BatchRequest& AddNamedMethodCall( const id_t& id, const std::string& name, const named_parameter& params = {} )
  {
    nlohmann::json request = { { "method", name }, { "params", params }, { "jsonrpc", "2.0" } };
    if( std::get_if<std::int64_t>( &id ) != nullptr ) request["id"] = std::get<std::int64_t>( id );
    else
      request["id"] = std::get<std::string>( id );
    call.push_back( request );
    return *this;
  }

  CXX_JSONRPC_API BatchRequest& AddNotificationCall( const std::string& name, const positional_parameter& params = {} )
  {
    call.push_back( { { "method", name }, { "params", params }, { "jsonrpc", "2.0" } } );
    return *this;
  }

  CXX_JSONRPC_API BatchRequest& AddNamedNotificationCall( const std::string& name, const named_parameter& params = {} )
  {
    call.push_back( { { "method", name }, { "params", params }, { "jsonrpc", "2.0" } } );
    return *this;
  }

  CXX_JSONRPC_API const nlohmann::json& Build() const { return call; }

private:
  nlohmann::json call = nlohmann::json::array();
};

class BatchResponse
{
public:
#if !defined( _WIN32 )
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Weffc++"
#endif

  CXX_JSONRPC_API explicit BatchResponse( nlohmann::json&& response ) : response( std::move( response ) )
  {
    for( size_t i = 0; i < this->response.size(); ++i )
    {
      auto& value = this->response[i];

      id_t id;

      if( value.is_object() && value.contains( "id" ) && value["id"].is_number_integer() ) { id = value["id"].get<std::int64_t>(); }
      else if( value.is_object() && value.contains( "id" ) && value["id"].is_string() ) { id = value["id"].get<std::string>(); }
      else
      {
        nullIds.push_back( i );
        continue;
      }

      if( value.contains( "result" ) ) results[id] = i;
      else if( value.contains( "error" ) )
        errors[id] = i;
      else
        nullIds.push_back( i );
    }
  }

#if !defined( _WIN32 )
  #pragma GCC diagnostic pop
#endif

  template<typename T> T Get( const id_t& id )
  {
    // Search in results map
    if( results.find( id ) != results.end() )
    {
      try
      {
        return response[results[id]]["result"].get<T>();
      }
      catch( const nlohmann::json::type_error& e )
      {
        throw exception( parse_error, "invalid return type: " + std::string( e.what() ) );
      }
    }

    // Search in errors map
    if( errors.find( id ) != errors.end() ) { throw fromJson( response[errors[id]]["error"] ); }

    throw exception( parse_error, "no result found for id "
                                    + std::visit(
                                      []( const auto& v )
                                      {
                                        if constexpr( std::is_same_v<std::decay_t<decltype( v )>, std::string> ) return v;
                                        else
                                          return std::to_string( v );
                                      },
                                      id ) );
  }

  CXX_JSONRPC_API bool HasErrors() const noexcept { return !errors.empty() || !nullIds.empty(); }

  CXX_JSONRPC_API std::vector<std::size_t> GetInvalidIndexes() const noexcept { return nullIds; }

  CXX_JSONRPC_API const nlohmann::json& GetResponse() const noexcept { return response; }

private:
  nlohmann::json                        response;
  std::unordered_map<id_t, std::size_t> results;
  std::unordered_map<id_t, std::size_t> errors;
  std::vector<size_t>                   nullIds;
};

}  // namespace jsonrpc
