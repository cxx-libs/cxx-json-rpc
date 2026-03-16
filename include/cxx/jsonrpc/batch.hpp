#pragma once

#include "cxx/jsonrpc/exception.hpp"
#include "cxx/jsonrpc/export.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <variant>

namespace jsonrpc
{

using id_t = std::variant<std::int64_t, std::string>;

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

typedef std::vector<nlohmann::json>           positional_parameter;
typedef std::map<std::string, nlohmann::json> named_parameter;

class BatchRequest
{
public:
  BatchRequest() : call( nlohmann::json::array() ) {}

  BatchRequest& AddMethodCall( const id_t& id, const std::string& name, const positional_parameter& params = {} )
  {
    nlohmann::json request = { { "method", name }, { "params", params }, { "jsonrpc", "2.0" } };
    if( std::get_if<std::int64_t>( &id ) != nullptr ) request["id"] = std::get<std::int64_t>( id );
    else
      request["id"] = std::get<std::string>( id );
    call.push_back( request );
    return *this;
  }

  BatchRequest& AddNamedMethodCall( const id_t& id, const std::string& name, const named_parameter& params = {} )
  {
    nlohmann::json request = { { "method", name }, { "params", params }, { "jsonrpc", "2.0" } };
    if( std::get_if<std::int64_t>( &id ) != nullptr ) request["id"] = std::get<std::int64_t>( id );
    else
      request["id"] = std::get<std::string>( id );
    call.push_back( request );
    return *this;
  }

  BatchRequest& AddNotificationCall( const std::string& name, const positional_parameter& params = {} )
  {
    call.push_back( { { "method", name }, { "params", params }, { "jsonrpc", "2.0" } } );
    return *this;
  }

  BatchRequest& AddNamedNotificationCall( const std::string& name, const named_parameter& params = {} )
  {
    call.push_back( { { "method", name }, { "params", params }, { "jsonrpc", "2.0" } } );
    return *this;
  }

  const nlohmann::json& Build() const { return call; }

private:
  nlohmann::json call;
};

class BatchResponse
{
public:
#if !defined( _WIN32 )
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Weffc++"
#endif
  explicit BatchResponse( nlohmann::json&& response ) : response( response )
  {
    for( auto& [key, value]: response.items() )
    {
      id_t id;
      if( value.is_object() && value.contains( "id" ) && value["id"].is_number() ) id = value["id"].get<std::int64_t>();
      else if( value.is_object() && value.contains( "id" ) && value["id"].is_string() )
        id = value["id"].get<std::string>();
      else
      {
        nullIds.push_back( std::stoi( key ) );
        continue;
      }
      if( value.contains( "result" ) ) results[id] = std::stoi( key );
      else if( value.contains( "error" ) )
        errors[id] = std::stoi( key );
      else
        nullIds.push_back( std::stoi( key ) );
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

  bool HasErrors() const noexcept { return !errors.empty() || !nullIds.empty(); }

  const std::vector<size_t> GetInvalidIndexes() { return nullIds; }

  const nlohmann::json& GetResponse() { return response; }

private:
  nlohmann::json                   response;
  std::unordered_map<id_t, size_t> results;
  std::unordered_map<id_t, size_t> errors;
  std::vector<size_t>              nullIds;
};

}  // namespace jsonrpc
