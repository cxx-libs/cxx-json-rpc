#include "warehouse.hpp"
#include "websocketconnector.hpp"

#include <cxx/jsonrpc/client.hpp>
#include <cxx/jsonrpc/server.hpp>
#include <iostream>
#include <vector>

using namespace jsonrpc;
using namespace std;

class WareHouseClient
{
public:
  explicit WareHouseClient( JsonRpcClient& client ) : client( client ) {}
  bool AddProduct( const Product& p ) { return client.CallMethod<bool>( 1, "AddProduct", { p } ); }

  void            NotifySum( const int a, const int b ) { client.CallNotification( "Notify", { a, b } ); }
  Product         GetProduct( const std::string& id ) { return client.CallMethod<Product>( 5, "GetProduct", { id } ); }
  vector<Product> AllProducts() { return client.CallMethod<vector<Product>>( 7, "AllProducts", {} ); }

private:
  JsonRpcClient& client;
};

void doWarehouseStuff2( IClientConnector& clientConnector )
{
  JsonRpcClient   client2( clientConnector );
  WareHouseClient appClient2( client2 );
  appClient2.NotifySum( 41, 1 );
}

void doWarehouseStuff( IClientConnector& clientConnector )
{
  JsonRpcClient   client( clientConnector );
  WareHouseClient appClient( client );
  Product         p;
  p.id    = "0xff";
  p.price = 22.4;
  p.name  = "Product 1";
  p.cat   = category::cash_carry;
  cout << "Adding product: " << std::boolalpha << appClient.AddProduct( p ) << "\n";

  Product g;
  p.id    = "0x2f";
  p.price = 88.4;
  p.name  = "MyProduct";
  p.cat   = category::cash_carry;
  cout << "Adding product: " << std::boolalpha << appClient.AddProduct( p ) << "\n";

  Product p2 = appClient.GetProduct( "0xff" );
  cout << "Found product: " << p2.name << "\n";
  try
  {
    appClient.GetProduct( "0xff2" );
  }
  catch( const jsonrpc::exception& e )
  {
    cerr << "Error finding product: " << e.what() << "\n";
  }

  try
  {
    Product p3 = appClient.GetProduct( "0x2f" );
    cout << "Found product: " << p3.name << " at price : " << p3.price << "\n";
  }
  catch( const jsonrpc::exception& e )
  {
    cerr << "Error finding product: " << e.what() << "\n";
  }

  auto all = appClient.AllProducts();
  for( const auto& p: all ) { cout << p.name << endl; }
}

int main()
{
  JsonRpcServer rpcServer;

  // Bindings
  WarehouseServer app;
  rpcServer.Add( "GetProduct", GetHandle( &WarehouseServer::GetProduct, app, "get the product" ), { "id" } );
  rpcServer.Add( "AddProduct", GetHandle( &WarehouseServer::AddProduct, app ), { "product" } );
  rpcServer.Add( "AllProducts", GetHandle( &WarehouseServer::AllProducts, app ), {} );
  rpcServer.Add( "Notify", GetHandle( &WarehouseServer::Notify, app ), { "a", "b" } );

  std::cout << rpcServer.getMethodList().dump( 2 ) << std::endl;
  std::cout << rpcServer.getNotificationList().dump( 2 ) << std::endl;
  std::cout << rpcServer.getProcedures().dump( 2 ) << std::endl;

  cout << "Running websocket example" << "\n";
  WebsocketServer WebsocketServer( rpcServer, "127.0.0.1", 8888 );
  cout << "Starting websocket server: " << std::boolalpha << WebsocketServer.start() << "\n";
  WebsocketClientConnector websocketClient( "127.0.0.1", 8888 );
  WebsocketClientConnector websocketClient2( "127.0.0.1", 8888 );
  std::this_thread::sleep_for( 0.5s );
  doWarehouseStuff( websocketClient );
  doWarehouseStuff2( websocketClient2 );

  return 0;
}
