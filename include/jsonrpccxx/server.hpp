#pragma once

#include "jsonrpccxx/dispatcher.hpp"
#include <string>

namespace jsonrpccxx
{

class JsonRpcServer
{
public:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
  explicit JsonRpcServer() = default;
#pragma GCC diagnostic pop
  ~JsonRpcServer() noexcept = default;

  bool Add(const std::string &name, MethodHandle callback, const NamedParamMapping &mapping = {})
  {
    if(name.rfind("rpc.", 0) == 0) return false;
    return m_dispatcher.Add(name, callback, mapping);
  }
    
  bool Add(const std::string &name, NotificationHandle callback, const NamedParamMapping &mapping = {})
  {
    if(name.rfind("rpc.", 0) == 0) return false;
    return m_dispatcher.Add(name, callback, mapping);
  }

  std::string HandleRequest(const std::string &requestString)
  {
  try
  {
      json request = json::parse(requestString);
      
      if(request.is_array())
      {
        json result = json::array();
        for(json &r : request)
        {
          json res = this->HandleSingleRequest(r);
          if(!res.is_null()) result.push_back(std::move(res));
        }
        return result.dump();
      }
      else if(request.is_object())
      {
        const json res = HandleSingleRequest(request);
        if(!res.is_null()) return res.dump();
        else return {};
      }
      else return json{{"id", nullptr}, {"error", {{"code", invalid_request}, {"message", "invalid request: expected array or object"}}}, {"jsonrpc", "2.0"}}.dump();
    }
  catch(const json::parse_error &e)
  {
    return json{{"id", nullptr}, {"error", {{"code", parse_error}, {"message", std::string("parse error: ") + e.what()}}}, {"jsonrpc", "2.0"}}.dump();
  }
}
protected:
  Dispatcher m_dispatcher;
private:
  json HandleSingleRequest( json &request)
  {
    json id = nullptr;
      if (request.contains("id") && (request["id"].is_number() || request["id"].is_string() || request["id"].is_null())) {
        id = request["id"];
      }
      try {
        return ProcessSingleRequest(request);
      } catch (const exception &e) {
        json error = {{"code", e.Code()}, {"message", e.Message()}};
        if (!e.Data().is_null()) {
          error["data"] = e.Data();
        }
        return json{{"id", id}, {"error", error}, {"jsonrpc", "2.0"}};
      } catch (const std::exception &e) {
        return json{{"id", id}, {"error", {{"code", internal_error}, {"message", std::string("internal server error: ") + e.what()}}}, {"jsonrpc", "2.0"}};
      } catch (...) {
        return json{{"id", id}, {"error", {{"code", internal_error}, {"message", std::string("internal server error")}}}, {"jsonrpc", "2.0"}};
      }
  }

  json ProcessSingleRequest( json &request)
  {
      if (!request.contains("jsonrpc") || !request["jsonrpc"].is_string() || request["jsonrpc"] != "2.0") {
        throw exception(invalid_request, R"(invalid request: missing jsonrpc field set to "2.0")");
      }
      if (!request.contains("method") || !request["method"].is_string()) {
        throw exception(invalid_request, "invalid request: method field must be a string");
      }
      if ( request.contains("id") && !(request.contains("id") && (request["id"].is_number() || request["id"].is_string() || request["id"].is_null()))) {
        throw exception(invalid_request, "invalid request: id field must be a number, string or null");
      }
      if ( request.contains("params") && !(request["params"].is_array() || request["params"].is_object() || request["params"].is_null())) {
        throw exception(invalid_request, "invalid request: params field must be an array, object or null");
      }
      if (! request.contains("params") || request["params"].is_null()) {
        request["params"] = json::array();
      }
      if (!request.contains("id")) {
        try {
          m_dispatcher.InvokeNotification(request["method"], request["params"]);
          return json();
        } catch (const std::exception &) {
          return json();
        }
      } else {
        return {{"jsonrpc", "2.0"}, {"id", request["id"]}, {"result", m_dispatcher.InvokeMethod(request["method"], request["params"])}};
      }
    }
  };
}
