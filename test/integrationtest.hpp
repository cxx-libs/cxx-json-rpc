#pragma once

#include "../examples/inmemoryconnector.hpp"

#include <cxx/jsonrpc/client.hpp>
#include <cxx/jsonrpc/server.hpp>

using namespace jsonrpc;

struct IntegrationTest
{
  IntegrationTest() : rpcServer(), connector( rpcServer ), client( connector ) {}
  JsonRpcServer     rpcServer;
  InMemoryConnector connector;
  JsonRpcClient     client;
};
