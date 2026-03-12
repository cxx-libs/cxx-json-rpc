#pragma once
#include "nlohmann/json.hpp"
#include <exception>
#include <string>
#include "jsonrpccxx/error_code.hpp"

namespace jsonrpccxx 
{
  class exception : public std::exception {
  public:
  exception(int code, const std::string &message) noexcept : code(code), message(message), data(nullptr), err(std::to_string(code) + ": " + message) {}
  exception(int code, const std::string &message, const nlohmann::json &data) noexcept
        : code(code), message(message), data(data), err(std::to_string(code) + ": " + message + ", data: " + data.dump()) {}

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
    const nlohmann::json &Data() const { return data; }

    const char* what() const noexcept override { return err.c_str(); }

    static inline exception fromJson(const nlohmann::json &value)
    {
      if (value.contains("code") && value["code"].is_number_integer() && value.contains("message") && value["message"].is_string())
      {
        if (value.contains("data")) {
          return exception(value["code"], value["message"], value["data"].get<nlohmann::json>());
        } else {
          return exception(value["code"], value["message"]);
        }
      }
      return exception(internal_error, R"(invalid error response: "code" (integer number) and "message" (string) are required)");
    }

  private:
    int code;
    std::string message;
    nlohmann::json data;
    std::string err;
  };
} // namespace jsonrpccxx
