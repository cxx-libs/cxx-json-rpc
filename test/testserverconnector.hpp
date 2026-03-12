#pragma once
#include "doctest/doctest.h"
#include <jsonrpccxx/server.hpp>

using namespace jsonrpc;

class TestServerConnector {
public:
  explicit TestServerConnector(JsonRpcServer &handler) : handler(handler), raw_response() {}

    void SendRawRequest(const std::string &request) { this->raw_response = handler.HandleRequest(request); }
    void SendRequest(const nlohmann::json &request) { SendRawRequest(request.dump()); }
    static nlohmann::json BuildMethodCall(const nlohmann::json &id, const std::string &name, const nlohmann::json &params) { return {{"id", id}, {"method", name}, {"params", params}, {"jsonrpc", "2.0"}}; }
    void CallMethod(const nlohmann::json &id, const std::string &name, const nlohmann::json &params) { SendRequest(BuildMethodCall(id, name, params)); }
    static nlohmann::json BuildNotificationCall(const std::string &name, const nlohmann::json &params) { return {{"method", name}, {"params", params}, {"jsonrpc", "2.0"}}; }
    void CallNotification(const std::string &name, const nlohmann::json &params) { SendRequest(BuildNotificationCall(name, params)); }

    nlohmann::json VerifyMethodResult(const nlohmann::json &id) {
        nlohmann::json result = nlohmann::json::parse(this->raw_response);
        return VerifyMethodResult(id, result);
    }

    static nlohmann::json VerifyMethodResult(const nlohmann::json &id, nlohmann::json &result) {
        REQUIRE(!result.contains("error"));
        REQUIRE(result["jsonrpc"] == "2.0");
        REQUIRE(result["id"] == id);
        REQUIRE(result.contains("result"));
        return result["result"];
    }

    nlohmann::json VerifyBatchResponse() {
        nlohmann::json result = nlohmann::json::parse(raw_response);
        REQUIRE(result.is_array());
        return result;
    }

    void VerifyNotificationResult() { VerifyNotificationResult(this->raw_response); }

    static void VerifyNotificationResult(std::string &raw_response) { REQUIRE(raw_response.empty()); }

    nlohmann::json VerifyMethodError(int code, const std::string &message, const nlohmann::json &id) {
        nlohmann::json error = nlohmann::json::parse(this->raw_response);
        return VerifyMethodError(code, message, id, error);
    }

    static nlohmann::json VerifyMethodError(int code, const std::string &message, const nlohmann::json &id, nlohmann::json &result) {
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
