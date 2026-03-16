#pragma once
#include "doctest/doctest.h"

#include <jsonrpccxx/client.hpp>
#include <jsonrpccxx/exception.hpp>
#include <jsonrpccxx/iclientconnector.hpp>
#include <nlohmann/json.hpp>
#include <string>

using namespace jsonrpc;

class TestClientConnector : public jsonrpc::ISyncClientConnector
{
public:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
  explicit TestClientConnector() noexcept = default;
#pragma GCC diagnostic push
  nlohmann::json request;
  std::string    raw_response;

  std::string SendAndReceive( const std::string_view r ) override
  {
    this->request = nlohmann::json::parse( r );
    return raw_response;
  }

  void SetBatchResult( const nlohmann::json& result ) { raw_response = result.dump(); }

  static nlohmann::json BuildResult( const nlohmann::json& result, int id ) { return { { "jsonrpc", "2.0" }, { "id", id }, { "result", result } }; }

  void SetResult( const nlohmann::json& result )
  {
    nlohmann::json response = { { "jsonrpc", "2.0" }, { "id", "1" }, { "result", result } };
    raw_response            = response.dump();
  }

  void SetError( const exception& e )
  {
    nlohmann::json response = { { "jsonrpc", "2.0" }, { "id", "1" } };
    if( !e.Data().empty() ) { response["error"] = { { "code", e.Code() }, { "message", e.Message() }, { "data", e.Data() } }; }
    else
    {
      response["error"] = { { "code", e.Code() }, { "message", e.Message() } };
    }
    raw_response = response.dump();
  }

  void VerifyMethodRequest( const std::string& name, nlohmann::json id )
  {
    CHECK( request["method"] == name );
    CHECK( request["id"] == id );
    CHECK( request["jsonrpc"] == "2.0" );
  }

  void VerifyNotificationRequest( const std::string& name )
  {
    CHECK( request["method"] == name );
    CHECK( request["jsonrpc"] == "2.0" );
    CHECK( !request.contains( "id" ) );
  }
};
