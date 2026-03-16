#include "warehouse.hpp"

#include <cxx/jsonrpc/exception.hpp>

using namespace jsonrpc;

bool WarehouseServer::AddProduct( const Product& p )
{
  if( products.find( p.id ) != products.end() ) return false;
  products[p.id] = p;
  return true;
}

const Product& WarehouseServer::GetProduct( const std::string& id ) const
{
  if( products.find( id ) == products.end() ) throw exception( -33000, "No product listed for id: " + id );
  return products.at( id );
}
std::vector<Product> WarehouseServer::AllProducts()
{
  std::vector<Product> result;
  for( const auto& p: products ) result.push_back( p.second );
  return result;
}
