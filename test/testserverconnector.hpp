#pragma once
#include "doctest/doctest.h"
#include <jsonrpccxx/server.hpp>

using namespace jsonrpccxx;

class TestServerConnector {
public:
  explicit TestServerConnector(JsonRpcServer &handler) : handler(handler), raw_response() {}

    void SendRawRequest(const std::string &request) { this->raw_response = handler.HandleRequest(request); }
    void SendRequest(const json &request) { SendRawRequest(request.dump()); }
    static json BuildMethodCall(const json &id, const std::string &name, const json &params) { return {{"id", id}, {"method", name}, {"params", params}, {"jsonrpc", "2.0"}}; }
    void CallMethod(const json &id, const std::string &name, const json &params) { SendRequest(BuildMethodCall(id, name, params)); }
    static json BuildNotificationCall(const std::string &name, const json &params) { return {{"method", name}, {"params", params}, {"jsonrpc", "2.0"}}; }
    void CallNotification(const std::string &name, const json &params) { SendRequest(BuildNotificationCall(name, params)); }

    json VerifyMethodResult(const json &id) {
        json result = json::parse(this->raw_response);
        return VerifyMethodResult(id, result);
    }

    static json VerifyMethodResult(const json &id, json &result) {
        REQUIRE(!result.contains("error"));
        REQUIRE(result["jsonrpc"] == "2.0");
        REQUIRE(result["id"] == id);
        REQUIRE(result.contains("result"));
        return result["result"];
    }

    json VerifyBatchResponse() {
        json result = json::parse(raw_response);
        REQUIRE(result.is_array());
        return result;
    }

    void VerifyNotificationResult() { VerifyNotificationResult(this->raw_response); }

    static void VerifyNotificationResult(std::string &raw_response) { REQUIRE(raw_response.empty()); }

    json VerifyMethodError(int code, const std::string &message, const json &id) {
        json error = json::parse(this->raw_response);
        return VerifyMethodError(code, message, id, error);
    }

    static json VerifyMethodError(int code, const std::string &message, const json &id, json &result) {
        REQUIRE(!result.contains("result"));
        REQUIRE(result["jsonrpc"] == "2.0");
        REQUIRE(result["id"] == id);
        REQUIRE((result.contains("error") && result["error"].is_object()));
        REQUIRE((result["error"].contains("code") && result["error"]["code"].is_number_integer()));
        REQUIRE(result["error"]["code"] == code);
        REQUIRE((result["error"].contains("message") && result["error"]["message"].is_string()));
        REQUIRE(result["error"]["message"].get<std::string>().find(message) != std::string::npos);

        return result["error"];
    }

private:
    JsonRpcServer &handler;
    std::string raw_response;
};
