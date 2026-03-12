#pragma once

#include <string>
#include <mutex>
#include <unordered_map>
#include <future>
#include <string>
#include <variant>
#include <cstdint>
#include <nlohmann/json.hpp>
#include "jsonrpccxx/error_code.hpp"
#include <iostream>

namespace jsonrpccxx
{

using id_t =  std::variant<std::int64_t, std::string>;


class IClientConnector
{
public:
  // General timeout setter (template)
  template<class Rep = double, class Period = std::milli> void setTimeout(const std::chrono::duration<Rep, Period>& timeout = std::chrono::milliseconds(1000))
  {
    m_timeout = timeout;
  }
  virtual ~IClientConnector() noexcept = default;
  // Unified Send method
  virtual std::string SendRequest(const std::string_view request) = 0 ;
protected:
  explicit IClientConnector() noexcept = default;
  std::chrono::duration<double,std::micro> m_timeout{std::chrono::milliseconds(1000)};
};


class ISyncClientConnector : public IClientConnector
{
public:
  explicit ISyncClientConnector() noexcept = default;
  virtual ~ISyncClientConnector() noexcept = default;
  std::string SendRequest(const std::string_view request) override final
  {
    auto fut = std::async(std::launch::async, [this, request]() { return SendAndReceive(request); });
    if (fut.wait_for(m_timeout) == std::future_status::ready) return fut.get();
    else
    {
      // Timeout occurred, return JSON-RPC error object
      nlohmann::json j = nlohmann::json::parse(request);

      nlohmann::json error_json;
      error_json["jsonrpc"] = "2.0";
      error_json["error"]["code"] = timeout; // custom timeout
      error_json["error"]["message"] = "timeout";
      // Include timeout in milliseconds
      double timeout_ms = std::chrono::duration<double, std::milli>(m_timeout).count();
      error_json["error"]["data"]["timeout_ms"] = timeout_ms;
      // Preserve id from original request
      if (j.contains("id")) error_json["id"] = j["id"];
      else error_json["id"] = nullptr;

      return error_json.dump();
    }
  }
protected:
  virtual std::string SendAndReceive(const std::string_view request) = 0;
};


class IAsyncClientConnector : public IClientConnector
{
public:
  explicit IAsyncClientConnector() noexcept = default;
  virtual ~IAsyncClientConnector() noexcept = default;
  std::string SendRequest(const std::string_view request) override final
  {
    nlohmann::json j = nlohmann::json::parse(request);
    // Prepare promise/future for async
    id_t id{0};
    if(!j.contains("id"))
    {
      Send(request);
      return "";
    }
    if(j["id"].is_string()) id=j["id"].get<std::string>();
    else id=j["id"].get<std::int64_t>();
    std::promise<std::string> prom;
    auto fut = prom.get_future();
    {
      std::lock_guard<std::mutex> lock(mtx_);
      promises_[id] = std::move(prom);
    }
    Send(request);
    // Wait for response with m_timeout
    if(fut.wait_for(m_timeout) == std::future_status::ready)
    {
      std::string ret = fut.get();
      std::lock_guard<std::mutex> lock(mtx_);
      promises_.erase(id);
      return ret;
    }
    else
    {
      // Timeout occurred
      {
        std::lock_guard<std::mutex> lock(mtx_);
        promises_.erase(id);
      }
      nlohmann::json error_json;
      error_json["jsonrpc"] = "2.0";
      error_json["error"]["code"] = timeout;
      error_json["error"]["message"] = "timeout";
      // Convert m_timeout to milliseconds for JSON
      double timeout_ms = std::chrono::duration<double, std::milli>(m_timeout).count();
      error_json["error"]["data"]["timeout_ms"] = timeout_ms;
      // Preserve the request id
      if (j["id"].is_null()) error_json["id"] = nullptr;
      else error_json["id"] = j["id"];
      return error_json.dump();
    }
  }
protected:
  virtual void Send(const std::string_view request) = 0;
  void Receive(const std::string_view response)
  {
    nlohmann::json j = nlohmann::json::parse(response);
    id_t id{0};
    if(j["id"].is_string()) id=j["id"].get<std::string>();
    else id=j["id"].get<std::int64_t>();
    {
      std::lock_guard<std::mutex> lock(mtx_);
      auto it = promises_.find(id);
      if (it != promises_.end()) it->second.set_value(std::string(response));
    }
  }

private:
  std::mutex mtx_;
  std::unordered_map<id_t, std::promise<std::string>> promises_;
  bool m_async;
};


}