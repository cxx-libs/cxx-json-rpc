#pragma once
#include <jsonrpccxx/iclientconnector.hpp>
#include <jsonrpccxx/server.hpp>

//This class is server and client connector at the same time.
class InMemoryConnector : public jsonrpc::ISyncClientConnector {
public:
  explicit InMemoryConnector(jsonrpc::JsonRpcServer &server) : server(server) {}
  std::string SendAndReceive(const std::string_view request) override final { return server.HandleRequest(std::string(request)); }
private:
  jsonrpc::JsonRpcServer &server;
};