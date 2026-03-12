#pragma once

#include "../examples/inmemoryconnector.hpp"
#include <jsonrpccxx/client.hpp>
#include <jsonrpccxx/server.hpp>

using namespace jsonrpc;

struct IntegrationTest
{
  IntegrationTest() : rpcServer(), connector(rpcServer), client(connector) {}
  JsonRpcServer rpcServer;
  InMemoryConnector connector;
  JsonRpcClient client;
};
