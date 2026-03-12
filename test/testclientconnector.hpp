#pragma once
#include "doctest/doctest.h"
#include <jsonrpccxx/client.hpp>
#include <jsonrpccxx/exception.hpp>
#include <jsonrpccxx/iclientconnector.hpp>
#include <nlohmann/json.hpp>
#include <string>

using namespace jsonrpccxx;

class TestClientConnector : public jsonrpccxx::ISyncClientConnector {
public:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
  explicit TestClientConnector() noexcept = default;
#pragma GCC diagnostic push
  json request;
  std::string raw_response;

  std::string SendAndReceive(const std::string_view r) override {
    this->request = json::parse(r);
    return raw_response;
  }

  void SetBatchResult(const json &result) {
    raw_response = result.dump();
  }

  static json BuildResult(const json &result, int id) {
    return {{"jsonrpc", "2.0"}, {"id", id}, {"result", result}};
  }

  void SetResult(const json &result) {
    json response = {{"jsonrpc", "2.0"}, {"id", "1"}, {"result", result}};
    raw_response = response.dump();
  }

  void SetError(const exception &e) {
    json response = {{"jsonrpc", "2.0"}, {"id", "1"}};
    if (!e.Data().empty()) {
      response["error"] = {{"code", e.Code()}, {"message", e.Message()}, {"data", e.Data()}};
    } else {
      response["error"] = {{"code", e.Code()}, {"message", e.Message()}};
    }
    raw_response = response.dump();
  }

  void VerifyMethodRequest(const std::string &name, json id) {
    CHECK(request["method"] == name);
    CHECK(request["id"] == id);
    CHECK(request["jsonrpc"] == "2.0");
  }

  void VerifyNotificationRequest(const std::string &name) {
    CHECK(request["method"] == name);
    CHECK(request["jsonrpc"] == "2.0");
    CHECK(!request.contains("id"));
  }
};
