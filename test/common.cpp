#include "doctest/doctest.h"

#include <jsonrpccxx/exception.hpp>

TEST_CASE( "exception error type" )
{
  CHECK( jsonrpc::exception( -32700, "" ).Type() == jsonrpc::parse_error );
  CHECK( jsonrpc::exception( -32600, "" ).Type() == jsonrpc::invalid_request );
  CHECK( jsonrpc::exception( -32601, "" ).Type() == jsonrpc::method_not_found );
  CHECK( jsonrpc::exception( -32602, "" ).Type() == jsonrpc::invalid_params );
  CHECK( jsonrpc::exception( -32603, "" ).Type() == jsonrpc::internal_error );

  for( int c = -32000; c >= -32099; c-- ) CHECK( jsonrpc::exception( c, "" ).Type() == jsonrpc::server_error );

  CHECK( jsonrpc::exception( 0, "" ).Type() == jsonrpc::invalid );
  CHECK( jsonrpc::exception( 32700, "" ).Type() == jsonrpc::invalid );
  CHECK( jsonrpc::exception( 33000, "" ).Type() == jsonrpc::invalid );
}
