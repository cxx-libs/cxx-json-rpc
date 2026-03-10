#pragma once

#include "jsonrpccxx/iclientconnector.hpp"
#include "jsonrpccxx/exception.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <variant>

namespace jsonrpccxx
{

typedef std::vector<json> positional_parameter;
typedef std::map<std::string, json> named_parameter;
typedef std::variant<int, std::string> id_type;

struct JsonRpcResponse
{
  id_type id;
  json result;
};

class JsonRpcClient
{
public:
  JsonRpcClient(IClientConnector &connector) : connector(connector) {}
  virtual ~JsonRpcClient() = default;

  template <typename T = json> T CallMethod(const id_type &id, const std::string &name) { return call_method(id, name, json::object()).result.get<T>(); }
  template <typename T = json> T CallMethod(const id_type &id, const std::string &name, const positional_parameter &params) { return call_method(id, name, params).result.get<T>(); }
  template <typename T = json> T CallMethodNamed(const id_type &id, const std::string &name, const named_parameter &params = {}) { return call_method(id, name, params).result.get<T>(); }

  void CallNotification(const std::string &name, const positional_parameter &params = {}) { call_notification(name, params); }
  void CallNotificationNamed(const std::string &name, const named_parameter &params = {}) { call_notification(name, params); }

protected:
  IClientConnector &connector;

private:
  JsonRpcResponse call_method(const id_type &id, const std::string &name, const json &params) const
  {
    json j = {{"method", name}, {"jsonrpc", "2.0"}};
    if(std::get_if<int>(&id) != nullptr) j["id"] = std::get<int>(id);
    else j["id"] = std::get<std::string>(id);
    if(!params.empty() && !params.is_null()) j["params"] = params;
    else if(params.is_array()) j["params"] = params;
    try
    {
      json response = json::parse(connector.Send(j.dump()));
      if(!response.contains("jsonrpc") || !response["jsonrpc"].is_string() || response["jsonrpc"] != "2.0" ) throw JsonRpcException(internal_error, "The 'jsonrpc' key is either missing or its value is invalid (expected '2.0').");
      if(!response.contains("id") || !( response["id"].is_null() || response["id"].is_string() || response["id"].is_number_integer())) throw JsonRpcException(internal_error, "The 'id' key is either missing or its type is invalid (expected 'null', 'string', 'integer').");
      if(response.contains("error") && response.contains("result")) throw JsonRpcException(internal_error, "'error' and 'result' keys cannot both be present.");

      if(response.contains("error") && response["error"].is_object()) throw JsonRpcException::fromJson(response["error"]);
      else if(response.contains("error") && response["error"].is_string()) throw JsonRpcException(internal_error, response["error"]);
      
      if(response.contains("result")) 
      {
        if(response["id"].is_string()) return JsonRpcResponse{response["id"].get<std::string>(), response["result"].get<json>()};
        else return JsonRpcResponse{response["id"].get<int>(), response["result"].get<json>()};
      }
      throw JsonRpcException(internal_error, R"(invalid server response: neither "result" nor "error" fields found)");
    }
    catch(const json::parse_error &e)
    {
      throw JsonRpcException(parse_error, std::string("invalid JSON response from server: ") + e.what());
    }
  }

  void call_notification(const std::string &name, const nlohmann::json &params)
  {
    nlohmann::json j = {{"method", name}, {"jsonrpc","2.0"}};
    if(!params.empty() && !params.is_null()) j["params"] = params;
    else if(params.is_array()) j["params"] = params;
    connector.Send(j.dump());
  }
};

} // namespace jsonrpccxx
