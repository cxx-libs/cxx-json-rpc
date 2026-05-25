#pragma once

#include "cxx/jsonrpc/error_code.hpp"
#include "cxx/jsonrpc/exception.hpp"
#include "cxx/jsonrpc/export.hpp"
#include "cxx/jsonrpc/id.hpp"

#include <cstdint>
#include <future>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <variant>

namespace jsonrpc
{

class IClientConnector
{
public:
  // General timeout setter (template)
  template<class Rep = double, class Period = std::milli> void setTimeout( const std::chrono::duration<Rep, Period>& timeout = std::chrono::milliseconds( 1000 ) ) { m_timeout = timeout; }
  CXX_JSONRPC_API virtual ~IClientConnector() noexcept                              = default;
  // Unified Send method
  CXX_JSONRPC_API virtual std::string SendRequest( const std::string_view request ) = 0;

protected:
  CXX_JSONRPC_API explicit IClientConnector() noexcept = default;
  std::chrono::duration<double, std::micro> m_timeout{ std::chrono::milliseconds( 1000 ) };
};

class ISyncClientConnector : public IClientConnector
{
public:
  CXX_JSONRPC_API explicit ISyncClientConnector() noexcept = default;
  CXX_JSONRPC_API virtual ~ISyncClientConnector() noexcept = default;
  CXX_JSONRPC_API std::string SendRequest( const std::string_view request ) override final
  {
    auto fut = std::async( std::launch::async, [this, request]() { return SendAndReceive( request ); } );
    if( fut.wait_for( m_timeout ) == std::future_status::ready ) return fut.get();
    else
    {
      // Timeout occurred, return JSON-RPC error object
      nlohmann::json j = nlohmann::json::parse( request );

      nlohmann::json error_json;
      error_json["jsonrpc"]                     = "2.0";
      error_json["error"]["code"]               = timeout;  // custom timeout
      error_json["error"]["message"]            = "timeout";
      // Include timeout in milliseconds
      double timeout_ms                         = std::chrono::duration<double, std::milli>( m_timeout ).count();
      error_json["error"]["data"]["timeout_ms"] = timeout_ms;
      // Preserve id from original request
      if( j.contains( "id" ) ) error_json["id"] = j["id"];
      else
        error_json["id"] = nullptr;

      return error_json.dump();
    }
  }

protected:
  CXX_JSONRPC_API virtual std::string SendAndReceive( const std::string_view request ) = 0;
};

class IAsyncClientConnector : public IClientConnector
{
public:
  explicit IAsyncClientConnector() noexcept = default;
  virtual ~IAsyncClientConnector() noexcept = default;

  std::string SendRequest( const std::string_view request ) override final
  {
    nlohmann::json j = nlohmann::json::parse( request );

    if( !j.is_object() ) throw exception( invalid_request, "async connector only supports single requests" );

    if( !j.contains( "id" ) )
    {
      Send( request );
      return "";
    }

    id_t id;

    if( j["id"].is_string() ) id = j["id"].get<std::string>();
    else
      id = j["id"].get<std::int64_t>();

    auto state = std::make_shared<promise_state>();

    auto fut = state->promise.get_future().share();

    {
      std::lock_guard<std::mutex> lock( mtx_ );
      promises_[id] = state;
    }

    Send( request );

    if( fut.wait_for( m_timeout ) == std::future_status::ready )
    {
      std::string result = fut.get();

      {
        std::lock_guard<std::mutex> lock( mtx_ );
        promises_.erase( id );
      }

      return result;
    }

    // timeout
    {
      std::lock_guard<std::mutex> lock( mtx_ );
      auto                        it = promises_.find( id );

      if( it != promises_.end() )
      {
        it->second->expired = true;
        promises_.erase( it );
      }
    }

    nlohmann::json error_json;
    error_json["jsonrpc"]          = "2.0";
    error_json["error"]["code"]    = timeout;
    error_json["error"]["message"] = "timeout";

    double timeout_ms = std::chrono::duration<double, std::milli>( m_timeout ).count();

    error_json["error"]["data"]["timeout_ms"] = timeout_ms;
    error_json["id"]                          = j["id"];

    return error_json.dump();
  }

protected:
  virtual void Send( const std::string_view request ) = 0;

  void Receive( const std::string_view response )
  {
    try
    {
      nlohmann::json j = nlohmann::json::parse( response );

      if( !j.is_object() || !j.contains( "id" ) ) return;

      id_t id;

      if( j["id"].is_string() ) id = j["id"].get<std::string>();
      else if( j["id"].is_number_integer() )
        id = j["id"].get<std::int64_t>();
      else
        return;

      std::shared_ptr<promise_state> state;

      {
        std::lock_guard<std::mutex> lock( mtx_ );

        auto it = promises_.find( id );

        if( it == promises_.end() ) return;

        state = it->second;
      }

      if( state->expired.load() ) return;

      bool expected = false;

      if( !state->completed.compare_exchange_strong( expected, true ) ) return;

      try
      {
        state->promise.set_value( std::string( response ) );
      }
      catch( const std::future_error& )
      {
        // already satisfied/cancelled
      }
    }
    catch( const nlohmann::json::exception& )
    {
      // malformed transport payload
    }
  }

private:
  struct promise_state
  {
    std::promise<std::string> promise;
    std::atomic<bool>         completed{ false };
    std::atomic<bool>         expired{ false };
  };

  std::mutex mtx_;

  std::unordered_map<id_t, std::shared_ptr<promise_state>, id_hash> promises_;
};

}  // namespace jsonrpc
