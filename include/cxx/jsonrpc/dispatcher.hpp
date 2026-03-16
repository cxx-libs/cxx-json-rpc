#pragma once

#include "cxx/jsonrpc/typemapper.hpp"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace jsonrpc
{

class Dispatcher
{
public:
#if !defined( _WIN32 )
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Weffc++"
#endif
  explicit Dispatcher() = default;
#if !defined( _WIN32 )
  #pragma GCC diagnostic pop
#endif
  ~Dispatcher() noexcept = default;

  bool Add( const std::string& name, Method callback, const std::optional<std::vector<std::string>>& mapping )
  {
    if( contains( name ) ) return false;
    if( mapping ) callback.setParameterNames( mapping.value() );
    methods.try_emplace( name, std::make_unique<Method>( std::move( callback ) ) );
    return true;
  }

  bool Add( const std::string& name, Notification callback, const std::optional<std::vector<std::string>>& mapping )
  {
    if( contains( name ) ) return false;
    if( mapping ) callback.setParameterNames( mapping.value() );
    notifications.try_emplace( name, std::make_unique<Notification>( std::move( callback ) ) );
    return true;
  }

  nlohmann::json InvokeMethod( const std::string& name, const nlohmann::json& params ) const
  {
    auto method = methods.find( name );
    if( method == methods.end() ) throw exception( method_not_found, "method not found: " + name );
    return method->second.get()->operator()( params );
  }

  void InvokeNotification( const std::string& name, const nlohmann::json& params ) const
  {
    auto notification = notifications.find( name );
    if( notification == notifications.end() ) throw exception( method_not_found, "notification not found: " + name );
    return notification->second.get()->operator()( params );
  }

  inline bool contains( const std::string& name ) const noexcept { return ( methods.find( name ) != methods.end() || notifications.find( name ) != notifications.end() ); }

  nlohmann::json listMethods() const
  {
    nlohmann::json json = nlohmann::json::array();
    for( const auto& [name, ptr]: methods )
    {
      nlohmann::json j;
      j["name"]        = name;
      j["description"] = ptr->getDescription();
      j["params"]      = nlohmann::json::array();
      for( const auto& param: ptr->getParameters() )
      {
        nlohmann::json p;
        p["cpp_type"]  = param.getType();
        p["json_type"] = param.getJSONType();
        p["name"]      = param.getName();
        j["params"].push_back( p );
      }
      json.push_back( j );
    }
    return json;
  }
  nlohmann::json listNotifications() const
  {
    nlohmann::json json = nlohmann::json::array();
    for( const auto& [name, ptr]: notifications )
    {
      nlohmann::json j;
      j["name"]        = name;
      j["description"] = ptr->getDescription();
      j["params"]      = nlohmann::json::array();
      for( const auto& param: ptr->getParameters() )
      {
        nlohmann::json p;
        p["cpp_type"]  = param.getType();
        p["json_type"] = param.getJSONType();
        p["name"]      = param.getName();
        j["params"].push_back( p );
      }
      json.push_back( j );
    }
    return json;
  }

  nlohmann::json listProcedures() const
  {
    return { { "notification", listNotifications() }, { "methods", listMethods() } };
    //json["notifications"] = listNotifications();
    ///json["methods"] = listMethods();
    //return json;
  }

private:
  std::unordered_map<std::string, std::unique_ptr<Method>>       methods;
  std::unordered_map<std::string, std::unique_ptr<Notification>> notifications;
};

}  // namespace jsonrpc
