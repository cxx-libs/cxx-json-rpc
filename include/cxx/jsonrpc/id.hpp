// include/cxx/jsonrpc/id.hpp
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <variant>

namespace jsonrpc
{

using id_t = std::variant<std::int64_t, std::string>;

struct id_hash
{
  CXX_JSONRPC_API std::size_t operator()( const id_t& id ) const noexcept
  {
    return std::visit( []( const auto& v ) { return std::hash<std::decay_t<decltype( v )>>{}( v ); }, id );
  }
};

}  // namespace jsonrpc
