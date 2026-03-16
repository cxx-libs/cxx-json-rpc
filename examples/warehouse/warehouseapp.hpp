#pragma once
#include "types.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>

class WarehouseServer
{
public:
  WarehouseServer() : products() {}

  bool                 AddProduct( const Product& p );
  const Product&       GetProduct( const std::string& id ) const;
  std::vector<Product> AllProducts();

  void Notify( const int a, const int b ) { std::cout << "**Notify function is launched on server : a(" << a << "+" << b << ")=" << a + b << "**" << std::endl; }

private:
  std::map<std::string, Product> products;
};
