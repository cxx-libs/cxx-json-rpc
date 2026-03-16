#pragma once

#include <cxx/jsonrpc/iclientconnector.hpp>
#include <cxx/jsonrpc/server.hpp>
#include <iostream>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXSocket.h>
#include <ixwebsocket/IXSocketServer.h>
#include <ixwebsocket/IXUserAgent.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <nlohmann/json.hpp>
#include <string>

class WebsocketClientConnector : public jsonrpc::IAsyncClientConnector
{
public:
  explicit WebsocketClientConnector( const std::string& host, int port )
  {
    ix::initNetSystem();
    client.setUrl( "ws://" + host + ":" + std::to_string( port ) );
    client.start();
    // Setup a callback to be fired (in a background thread, watch out for race conditions !)
    // when a message or an event (open, close, error) is received
    client.setOnMessageCallback(
      [this]( const ix::WebSocketMessagePtr& msg )
      {
        if( msg->type == ix::WebSocketMessageType::Message ) { this->Receive( msg->str ); }
        else if( msg->type == ix::WebSocketMessageType::Open ) { std::cout << "Connection established" << std::endl; }
        else if( msg->type == ix::WebSocketMessageType::Error ) { std::cout << "Connection error: " << msg->errorInfo.reason << std::endl; }
      } );
  }
  ~WebsocketClientConnector() {}
  void Send( const std::string_view request ) override final { client.send( std::string( request ) ); }

private:
  ix::WebSocket client;
};

class WebsocketServer
{
public:
  explicit WebsocketServer( jsonrpc::JsonRpcServer& server, const std::string& host, int port ) : server( server ), m_server( port, host )
  {
    ix::initNetSystem();
    m_server.setOnClientMessageCallback(
      [this]( std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr& msg )
      {
        if( msg->type == ix::WebSocketMessageType::Open ) { std::cout << "New connection" << std::endl; }
        else if( msg->type == ix::WebSocketMessageType::Message )
        {
          const std::string ret = this->server.HandleRequest( msg->str );
          if( !ret.empty() ) webSocket.send( ret );
        }
      } );
  }

  virtual ~WebsocketServer() { m_server.stop(); }

  bool start()
  {
    auto res = m_server.listen();
    if( !res.first ) return 1;
    m_server.start();
    return true;
  }

  void wait() { m_server.wait(); }

private:
  jsonrpc::JsonRpcServer& server;
  ix::WebSocketServer     m_server;
};
