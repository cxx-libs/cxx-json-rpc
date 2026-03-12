#include "doctest/doctest.h"
#include <jsonrpccxx/exception.hpp>

TEST_CASE("exception error type") {
  CHECK(jsonrpccxx::exception(-32700, "").Type() == jsonrpccxx::parse_error);
  CHECK(jsonrpccxx::exception(-32600, "").Type() == jsonrpccxx::invalid_request);
  CHECK(jsonrpccxx::exception(-32601, "").Type() == jsonrpccxx::method_not_found);
  CHECK(jsonrpccxx::exception(-32602, "").Type() == jsonrpccxx::invalid_params);
  CHECK(jsonrpccxx::exception(-32603, "").Type() == jsonrpccxx::internal_error);

  for(int c = -32000; c >= -32099; c--)
    CHECK(jsonrpccxx::exception(c, "").Type() == jsonrpccxx::server_error);

  CHECK(jsonrpccxx::exception(0, "").Type() == jsonrpccxx::invalid);
  CHECK(jsonrpccxx::exception(32700, "").Type() == jsonrpccxx::invalid);
  CHECK(jsonrpccxx::exception(33000, "").Type() == jsonrpccxx::invalid);
}