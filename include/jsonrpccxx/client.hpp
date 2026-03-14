#pragma once

#include "jsonrpccxx/iclientconnector.hpp"
#include "jsonrpccxx/exception.hpp"
#include <cstdint>
#include <nlohmann/json.hpp>
#include "jsonrpccxx/batch.hpp"
#include <string>

namespace jsonrpc
{

struct Response
{
  id_t id;
  nlohmann::json result;
};

class JsonRpcClient
{
public:
  explicit JsonRpcClient(IClientConnector &connector) noexcept : connector(connector) {}
  explicit JsonRpcClient() =delete;
  virtual ~JsonRpcClient() = default;

  template <typename T = nlohmann::json> T CallMethod(const id_t &id, const std::string &name) { return call_method(id, name, nlohmann::json::object()).result.get<T>(); }
  template <typename T = nlohmann::json> T CallMethod(const id_t &id, const std::string &name, const positional_parameter &params) { return call_method(id, name, params).result.get<T>(); }
  template <typename T = nlohmann::json> T CallMethodNamed(const id_t &id, const std::string &name, const named_parameter &params = {}) { return call_method(id, name, params).result.get<T>(); }

  void CallNotification(const std::string &name, const positional_parameter &params = {}) { call_notification(name, params); }
  void CallNotificationNamed(const std::string &name, const named_parameter &params = {}) { call_notification(name, params); }

  BatchResponse BatchCall(const BatchRequest &request)
  {
    try
    {
      nlohmann::json response = nlohmann::json::parse(connector.SendRequest(request.Build().dump()));
      if(!response.is_array()) throw exception(parse_error, "invalid JSON response from server: expected array");
      return BatchResponse(std::move(response));
    }
    catch(const nlohmann::json::parse_error &e)
    {
      throw exception(parse_error, std::string("invalid JSON response from server: ") + e.what());
    }
  }

protected:
  IClientConnector& connector;

private:
  Response call_method(const id_t &id, const std::string &name, const nlohmann::json &params) const
  {
    nlohmann::json j = {{"method", name}, {"jsonrpc", "2.0"}};
    if(std::get_if<std::int64_t>(&id) != nullptr) j["id"] = std::get<std::int64_t>(id);
    else j["id"] = std::get<std::string>(id);
    if(!params.empty() && !params.is_null()) j["params"] = params;
    else if(params.is_array()) j["params"] = params;
    try
    {
      nlohmann::json response = nlohmann::json::parse(connector.SendRequest(j.dump()));
      if(!response.contains("jsonrpc") || !response["jsonrpc"].is_string() || response["jsonrpc"] != "2.0" ) throw exception(internal_error, "The 'jsonrpc' key is either missing or its value is invalid (expected '2.0').");
      if(!response.contains("id") || !( response["id"].is_null() || response["id"].is_string() || response["id"].is_number_integer())) throw exception(internal_error, "The 'id' key is either missing or its type is invalid (expected 'null', 'string', 'integer').");
      if(response.contains("error") && response.contains("result")) throw exception(internal_error, "'error' and 'result' keys cannot both be present.");

      if(response.contains("error") && response["error"].is_object()) throw fromJson(response["error"]);
      else if(response.contains("error") && response["error"].is_string()) throw exception(internal_error, response["error"]);
      
      if(response.contains("result")) 
      {
        if(response["id"].is_string()) return Response{response["id"].get<std::string>(), response["result"].get<nlohmann::json>()};
        else return Response{response["id"].get<int>(), response["result"].get<nlohmann::json>()};
      }
      throw exception(internal_error, "invalid server response: neither 'result' nor 'error' fields found");
    }
    catch(const nlohmann::json::parse_error &e)
    {
      throw exception(parse_error, std::string("invalid JSON response from server: ") + e.what());
    }
  }

  void call_notification(const std::string &name, const nlohmann::json &params)
  {
    nlohmann::json j = {{"method", name}, {"jsonrpc","2.0"}};
    if(!params.empty() && !params.is_null()) j["params"] = params;
    else if(params.is_array()) j["params"] = params;
    connector.SendRequest(j.dump());
  }

};

} // namespace jsonrpc
