#pragma once

#include <httplib.h>
#include <jsonrpccxx/iclientconnector.hpp>
#include <jsonrpccxx/server.hpp>
#include <string>
#include <thread>

class CppHttpLibClientConnector : public jsonrpc::ISyncClientConnector
{
public:
  explicit CppHttpLibClientConnector( const std::string& host, const int port ) : httpClient( host.c_str(), port ) {}
  std::string SendAndReceive( const std::string_view request ) override final
  {
    auto res = httpClient.Post( "/jsonrpc", std::string( request ), "application/json" );
    if( !res || res->status != 200 ) throw jsonrpc::exception( -32003, "client connector error, received status != 200" );
    return res->body;
  }

private:
  httplib::Client httpClient;
};

class CppHttpLibServerConnector
{
public:
  explicit CppHttpLibServerConnector( jsonrpc::JsonRpcServer& server, const int port ) : server( server ), port( port )
  {
    httpServer.Post( "/jsonrpc", [this]( const httplib::Request& req, httplib::Response& res ) { this->PostAction( req, res ); } );
  }

  virtual ~CppHttpLibServerConnector() { StopListening(); }

  bool StartListening()
  {
    if( httpServer.is_running() ) return false;
    this->thread = std::thread( [this]() { this->httpServer.listen( "localhost", port ); } );
    return true;
  }

  void StopListening()
  {
    if( httpServer.is_running() )
    {
      httpServer.stop();
      this->thread.join();
    }
  }

private:
  std::thread             thread;
  jsonrpc::JsonRpcServer& server;
  httplib::Server         httpServer;
  int                     port;

  void PostAction( const httplib::Request& req, httplib::Response& res )
  {
    res.status = 200;
    res.set_content( this->server.HandleRequest( req.body ), "application/json" );
  }
};
