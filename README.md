# json-rpc-cxx


[![Build status](https://ci.appveyor.com/api/projects/status/c6rv869h984m1eo2?svg=true)](https://ci.appveyor.com/project/cinemast/json-rpc-cxx)
[![CircleCI](https://circleci.com/gh/jsonrpcx/json-rpc-cxx.svg?style=svg)](https://circleci.com/gh/jsonrpcx/json-rpc-cxx)
![GitHub](https://img.shields.io/github/license/jsonrpcx/json-rpc-cxx.svg)
[![codecov](https://codecov.io/gh/jsonrpcx/json-rpc-cxx/branch/master/graph/badge.svg)](https://codecov.io/gh/jsonrpcx/json-rpc-cxx)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/jsonrpcx/json-rpc-cxx.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/jsonrpcx/json-rpc-cxx/context:cpp)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/16b095ad49964288b524bc0b499c4efb)](https://www.codacy.com/app/cinemast/json-rpc-cxx?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=jsonrpcx/json-rpc-cxx&amp;utm_campaign=Badge_Grade)
![GitHub tag (latest SemVer)](https://img.shields.io/github/tag/jsonrpcx/json-rpc-cxx.svg)

![json-rpc-cxx-icon](doc/icon.png)

A [JSON-RPC](https://www.jsonrpc.org/) 2.0 framework implemented in C++17 using the [nlohmann's json for modern C++](https://github.com/nlohmann/json).

-   JSON-RPC 2.0 compliant client
-   JSON-RPC 2.0 compliant server
-   Transport agnostic interfaces
-   Compile time type mapping (using [nlohmann's arbitrary type conversion](https://github.com/nlohmann/json#arbitrary-types-conversions))
-   Runtime type checking
-   Cross-platform (Windows, Linux, OSX)

## Installation

-   Copy [include/jsonrpccxx](include) to your include path
-   Alternatively use CMake install mechanism

```bash
mkdir build && cd build
cmake ..
sudo make install
```

## Usage

-   [examples/warehouse/main.cpp](examples/warehouse/main.cpp)

## Design goals

-   Easy to use interface
-   Type safety where possible
-   Avoid errors at compile time where possible
-   Test driven development
-   Choose expressiveness over speed
-   Minimal dependencies

## License

This framework is licensed under [MIT](LICENSE).

### Dependencies

-   [nlohmann's JSON for modern C++](https://github.com/nlohmann/json) is licensed under MIT.
-   Optional: [doctest](https://github.com/onqtam/doctest) is licensed under MIT.

## Developer information

-   [CONTRIBUTING.md](CONTRIBUTING.md)
-   [CHANGELOG.md](CHANGELOG.md)
# cxx-jsonrpc

A lightweight, header-only (or easily embeddable) modern C++17 JSON-RPC 2.0 framework built on top of nlohmann/json.

It provides:

- JSON-RPC 2.0 client and server support
- Strongly typed method binding
- Notifications and batch requests
- Synchronous and asynchronous connectors
- Automatic parameter validation
- Named and positional parameters
- Introspection utilities
- Minimal dependencies
- Cross-platform support

---

## Features

- ✅ JSON-RPC 2.0 compliant
- ✅ Type-safe method registration
- ✅ Batch request/response support
- ✅ Sync and async transport abstraction
- ✅ Automatic JSON type validation
- ✅ Named parameter mapping
- ✅ Introspection (`listMethods`, `listNotifications`)
- ✅ Exception-based error handling
- ✅ Supports:
  - free functions
  - member functions
  - lambdas (via `std::function`)
  - notifications
  - methods with return values

---

## Requirements

- C++17 or newer
- nlohmann/json

---

# Installation

## Header-only usage

Copy the `include/cxx/jsonrpc` directory into your project.

Then include:

```cpp
#include <cxx/jsonrpc/server.hpp>
#include <cxx/jsonrpc/client.hpp>
```

---

## CMake

```cmake
find_package(nlohmann_json REQUIRED)

add_library(cxx-jsonrpc INTERFACE)

target_include_directories(cxx-jsonrpc INTERFACE include)

target_link_libraries(cxx-jsonrpc
    INTERFACE
    nlohmann_json::nlohmann_json
)
```

---

# Quick Start

## Server Example

```cpp
#include <cxx/jsonrpc/server.hpp>
#include <iostream>

int add(int a, int b)
{
    return a + b;
}

int main()
{
    jsonrpc::JsonRpcServer server;

    server.Add(
        "add",
        jsonrpc::GetHandle(add, "Add two integers"),
        {"a", "b"}
    );

    std::string request = R"({
        "jsonrpc": "2.0",
        "id": 1,
        "method": "add",
        "params": [3, 5]
    })";

    std::cout << server.HandleRequest(request) << std::endl;
}
```

### Response

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": 8
}
```

---

## Client Example

```cpp
#include <cxx/jsonrpc/client.hpp>
#include <iostream>

class HttpConnector : public jsonrpc::ISyncClientConnector
{
protected:
    std::string SendAndReceive(const std::string_view request) override
    {
        // Send request using your transport layer
        return R"({
            "jsonrpc": "2.0",
            "id": 1,
            "result": 42
        })";
    }
};

int main()
{
    HttpConnector connector;

    jsonrpc::JsonRpcClient client(connector);

    int result = client.CallMethod<int>(1, "answer");

    std::cout << result << std::endl;
}
```

---

# Supported Parameter Types

The framework automatically validates parameter types before invoking callbacks.

## Primitive Types

- `bool`
- signed integers
- unsigned integers
- floating point types
- `std::string`
- enums

## STL Types

- `std::vector`
- `std::array`
- `std::deque`
- `std::set`
- `std::unordered_set`
- `std::map`
- `std::unordered_map`
- `std::pair`
- `std::tuple`
- `std::optional`
- `std::variant`

---

# Registering Methods

## Free Function

```cpp
int multiply(int a, int b)
{
    return a * b;
}

server.Add(
    "multiply",
    jsonrpc::GetHandle(multiply)
);
```

---

## Member Function

```cpp
class Calculator
{
public:
    int sub(int a, int b)
    {
        return a - b;
    }
};

Calculator calc;

server.Add(
    "sub",
    jsonrpc::GetHandle(&Calculator::sub, calc)
);
```

---

## Notification

Notifications do not return a result.

```cpp
void logMessage(std::string text)
{
    std::cout << text << std::endl;
}

server.Add(
    "log",
    jsonrpc::GetHandle(logMessage)
);
```

Client request:

```json
{
  "jsonrpc": "2.0",
  "method": "log",
  "params": ["hello"]
}
```

---

# Named Parameters

You can map parameter names during registration.

```cpp
server.Add(
    "divide",
    jsonrpc::GetHandle(divide),
    {"numerator", "denominator"}
);
```

Then call using:

```json
{
  "jsonrpc": "2.0",
  "id": 10,
  "method": "divide",
  "params": {
    "numerator": 10,
    "denominator": 2
  }
}
```

---

# Batch Requests

## Building Batch Requests

```cpp
jsonrpc::BatchRequest batch;

batch
    .AddMethodCall(1, "sum", {1, 2})
    .AddMethodCall(2, "sum", {5, 7})
    .AddNotificationCall("notify", {"hello"});
```

---

## Sending Batch Requests

```cpp
auto response = client.BatchCall(batch);

int first  = response.Get<int>(1);
int second = response.Get<int>(2);
```

---

# Error Handling

All framework errors throw `jsonrpc::exception`.

```cpp
try
{
    int result = client.CallMethod<int>(1, "sum");
}
catch(const jsonrpc::exception& e)
{
    std::cerr
        << "Code: " << e.code()
        << "\nMessage: " << e.message()
        << std::endl;
}
```

---

# Built-in Error Codes

| Name | Code |
|---|---|
| parse_error | -32700 |
| invalid_request | -32600 |
| method_not_found | -32601 |
| invalid_params | -32602 |
| internal_error | -32603 |
| server_error | -32000 |
| timeout | -32001 |

---

# Asynchronous Connectors

Implement `IAsyncClientConnector` to support asynchronous transports such as:

- WebSocket
- TCP
- IPC
- Message queues

Example:

```cpp
class WsConnector : public jsonrpc::IAsyncClientConnector
{
protected:
    void Send(const std::string_view request) override
    {
        websocket.send(request);
    }

public:
    void OnMessage(const std::string& response)
    {
        Receive(response);
    }
};
```

---

# Introspection

The server can expose metadata about registered procedures.

## List Methods

```cpp
auto methods = server.getMethodList();
```

## List Notifications

```cpp
auto notifications = server.getNotificationList();
```

## List Everything

```cpp
auto procedures = server.getProcedures();
```

Example output:

```json
[
  {
    "name": "add",
    "description": "Add two integers",
    "params": [
      {
        "cpp_type": "int",
        "json_type": "integer",
        "name": "a"
      },
      {
        "cpp_type": "int",
        "json_type": "integer",
        "name": "b"
      }
    ]
  }
]
```

---

# Transport Layer

The library intentionally does not implement networking.

You provide transport behavior by implementing:

- `ISyncClientConnector`
- `IAsyncClientConnector`

This allows integration with:

- HTTP
- WebSocket
- ZeroMQ
- Unix sockets
- Serial communication
- Custom IPC systems

---

# JSON-RPC Compliance

This project follows the JSON-RPC 2.0 specification:

https://www.jsonrpc.org/specification

Supported features:

- Requests
- Responses
- Notifications
- Batch requests
- Named parameters
- Error objects

---

# Example Project Structure

```text
project/
├── include/
│   └── cxx/jsonrpc/
├── src/
├── examples/
├── tests/
└── CMakeLists.txt
```

---

# Roadmap

- [ ] Coroutine support
- [ ] Streaming responses
- [ ] Authentication middleware
- [ ] OpenRPC generation
- [ ] Better compile-time reflection

---

# Contributing

Contributions are welcome.

Please open an issue or submit a pull request.

---

# License

MIT License

---

