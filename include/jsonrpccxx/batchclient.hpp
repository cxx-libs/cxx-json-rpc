#pragma once

#include "jsonrpccxx/client.hpp"
#include <cstddef>

namespace jsonrpccxx
{

class BatchRequest
{
public:
  BatchRequest() : call(json::array()) {}
  
  BatchRequest& AddMethodCall(const id_type &id, const std::string &name, const positional_parameter &params = {})
  {
    json request = {{"method", name}, {"params", params}, {"jsonrpc", "2.0"}};
    if(std::get_if<std::int64_t>(&id) != nullptr) request["id"] = std::get<std::int64_t>(id);
    else request["id"] = std::get<std::string>(id);
    call.push_back(request);
    return *this;
  }

  BatchRequest& AddNamedMethodCall(const id_type &id, const std::string &name, const named_parameter &params = {})
  {
    json request = {{"method", name}, {"params", params}, {"jsonrpc", "2.0"}};
    if(std::get_if<std::int64_t>(&id) != nullptr) request["id"] = std::get<std::int64_t>(id);
    else request["id"] = std::get<std::string>(id);
    call.push_back(request);
    return *this;
  }

  BatchRequest& AddNotificationCall(const std::string &name, const positional_parameter &params = {})
  {
    call.push_back({{"method", name}, {"params", params}, {"jsonrpc", "2.0"}});
    return *this;
  }

  BatchRequest& AddNamedNotificationCall(const std::string &name, const named_parameter &params = {})
  {
    call.push_back({{"method", name}, {"params", params}, {"jsonrpc", "2.0"}});
    return *this;
  }

  const json &Build() const { return call; }

private:
  json call;
};

class BatchResponse
{
public:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
  explicit BatchResponse(json &&response) : response(response)
  {
    for(auto &[key, value] : response.items())
    {
      id_type id;
      if(value.is_object() && value.contains("id") && value["id"].is_number()) id = value["id"].get<std::int64_t>();
      else if (value.is_object()&& value.contains("id") && value["id"].is_string()) id = value["id"].get<std::string>();
      else
      {
        nullIds.push_back(std::stoi(key));
        continue;
      }
      if(value.contains("result")) results[id] = std::stoi(key);
      else if (value.contains("error")) errors[id] = std::stoi(key);
      else nullIds.push_back(std::stoi(key));
    }
  }
#pragma GCC diagnostic pop


template<typename T>
T Get(const id_type& id)
{

    // Search in results map
    if (results.find(id) != results.end())
    {
        try
        {
            return response[results[id]]["result"].get<T>();
        }
        catch (const json::type_error& e)
        {
            throw JsonRpcException(parse_error, "invalid return type: " + std::string(e.what()));
        }
    }

    // Search in errors map
    if (errors.find(id) != errors.end())
    {
        throw JsonRpcException::fromJson(response[errors[id]]["error"]);
    }

    throw JsonRpcException(parse_error, "no result found for id " + std::visit([](const auto& v) {
        if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::string>)
            return v;
        else
            return std::to_string(v);
    }, id));
}

  bool HasErrors() { return !errors.empty() || !nullIds.empty(); }
  
  const std::vector<size_t> GetInvalidIndexes() { return nullIds; }
  
  const json& GetResponse() { return response; }

private:
  json response;
  std::map<id_type, size_t> results;
  std::map<id_type, size_t> errors;
  std::vector<size_t> nullIds;
};

class BatchClient : public JsonRpcClient
{
public:
  explicit BatchClient(IClientConnector &connector) : JsonRpcClient(connector) {}
  
  BatchResponse BatchCall(const BatchRequest &request)
  {
    try
    {
      
      json response = json::parse(connector.SendRequest(request.Build().dump()));
      if(!response.is_array()) throw JsonRpcException(parse_error, "invalid JSON response from server: expected array");
      return BatchResponse(std::move(response));
    }
    catch(const json::parse_error &e)
    {
      throw JsonRpcException(parse_error, std::string("invalid JSON response from server: ") + e.what());
    }
  }
};

}
