#pragma once

namespace jsonrpc
{

enum error_type
{
  parse_error      = -32700,
  invalid_request  = -32600,
  method_not_found = -32601,
  invalid_params   = -32602,
  internal_error   = -32603,
  server_error     = -32000,
  timeout          = -32001,
  invalid
};

}
