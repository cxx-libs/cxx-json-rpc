#include "doctest/doctest.h"

#include <cxx/jsonrpc/exception.hpp>

TEST_CASE( "exception error type" )
{
  CHECK( jsonrpc::exception( -32700, "" ).type() == jsonrpc::parse_error );
  CHECK( jsonrpc::exception( -32600, "" ).type() == jsonrpc::invalid_request );
  CHECK( jsonrpc::exception( -32601, "" ).type() == jsonrpc::method_not_found );
  CHECK( jsonrpc::exception( -32602, "" ).type() == jsonrpc::invalid_params );
  CHECK( jsonrpc::exception( -32603, "" ).type() == jsonrpc::internal_error );

  for( int c = -32000; c >= -32099; c-- ) CHECK( jsonrpc::exception( c, "" ).type() == jsonrpc::server_error );

  CHECK( jsonrpc::exception( 0, "" ).type() == jsonrpc::invalid );
  CHECK( jsonrpc::exception( 32700, "" ).type() == jsonrpc::invalid );
  CHECK( jsonrpc::exception( 33000, "" ).type() == jsonrpc::invalid );
}
