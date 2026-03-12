#pragma once
#include <exception>
#include <string>
#include "jsonrpccxx/error_code.hpp"

namespace jsonrpccxx 
{
  class exception : public std::exception {
  public:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
  exception(int code, const std::string &message) noexcept : code(code), message(message), err(std::to_string(code) + ": " + message) {}
  exception(int code, const std::string &message, const std::string & data) noexcept : code(code), message(message), data(data), err(std::to_string(code) + ": " + message + ", data: " + data) {}
#pragma GCC diagnostic pop

    error_type Type() const {
      if (code >= -32603 && code <= -32600)
        return static_cast<error_type>(code);
      if (code >= -32099 && code <= -32000)
        return server_error;
      if (code == -32700)
        return parse_error;
      return invalid;
    }

    int Code() const { return code; }
    const std::string& Message() const { return message; }
    const std::string &Data() const { return data; }

    const char* what() const noexcept override { return err.c_str(); }



  private:
    int code;
    std::string message;
    std::string data;
    std::string err;
  };
} // namespace jsonrpccxx
