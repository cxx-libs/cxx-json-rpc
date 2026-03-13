#include "doctest/doctest.h"
#include "testclientconnector.hpp"
#include <jsonrpccxx/client.hpp>

using namespace jsonrpc;

struct F
{
  TestClientConnector c;
  JsonRpcClient client;
  F() : c(), client(c) {}
};

TEST_CASE_FIXTURE(F, "method_noparams") {
  c.SetResult(true);
  client.CallMethod<nlohmann::json>("000-000-000", "some.method_1");
  c.VerifyMethodRequest("some.method_1", "000-000-000");
  CHECK(!c.request.contains("params"));
}

TEST_CASE_FIXTURE(F, "method_call_params_empty") {
  c.SetResult(true);
  client.CallMethod<nlohmann::json>("1", "some.method_1", {});
  c.VerifyMethodRequest("some.method_1", "1");
  CHECK(c.request["params"].is_array());
  CHECK(c.request["params"].empty());
  CHECK(c.request["params"].dump() == "[]");

  c.SetResult(true);
  client.CallMethod<nlohmann::json>("1", "some.method_1", nlohmann::json::array());
  c.VerifyMethodRequest("some.method_1", "1");
  CHECK(c.request["params"].is_array());
  CHECK(c.request["params"].empty());
  CHECK(c.request["params"].dump() == "[]");
}

TEST_CASE_FIXTURE(F, "method_call_params_byname") {
  c.SetResult(true);
  client.CallMethodNamed<nlohmann::json>("1", "some.method_1", {{"a", "hello"}, {"b", 77}, {"c", true}});
  c.VerifyMethodRequest("some.method_1", "1");
  CHECK(c.request["params"]["a"] == "hello");
  CHECK(c.request["params"]["b"] == 77);
  CHECK(c.request["params"]["c"] == true);
}

TEST_CASE_FIXTURE(F, "method_call_params_byposition") {
  c.SetResult(true);
  client.CallMethod<nlohmann::json>("1", "some.method_1", {"hello", 77, true});
  c.VerifyMethodRequest("some.method_1", "1");
  CHECK(c.request["params"][0] == "hello");
  CHECK(c.request["params"][1] == 77);
  CHECK(c.request["params"][2] == true);
}

TEST_CASE_FIXTURE(F, "method_result_simple") {
  c.SetResult(23);
  int r = client.CallMethod<int>("1", "some.method_1", {});
  c.VerifyMethodRequest("some.method_1", "1");
  CHECK(23 == r);
}

TEST_CASE_FIXTURE(F, "method_result_object") {
  c.SetResult({{"a", 3}, {"b", 4}});
  nlohmann::json r = client.CallMethod<nlohmann::json>("1", "some.method_1", {});
  c.VerifyMethodRequest("some.method_1", "1");
  CHECK(r["a"] == 3);
  CHECK(r["b"] == 4);
}

TEST_CASE_FIXTURE(F, "method_result_array") {
  c.SetResult({2, 3, 4});
  nlohmann::json r = client.CallMethod<nlohmann::json>("1", "some.method_1", {});
  c.VerifyMethodRequest("some.method_1", "1");
  CHECK(r[0] == 2);
  CHECK(r[1] == 3);
  CHECK(r[2] == 4);
}

TEST_CASE_FIXTURE(F, "method_result_empty") {
  c.raw_response = "{}";
  REQUIRE_THROWS_WITH(client.CallMethod<nlohmann::json>("1", "some.method_1", {}), "-32603: The 'jsonrpc' key is either missing or its value is invalid (expected '2.0').");
  c.VerifyMethodRequest("some.method_1", "1");
  c.raw_response = "[]";
  REQUIRE_THROWS_WITH(client.CallMethod<nlohmann::json>("1", "some.method_1", {}), "-32603: The 'jsonrpc' key is either missing or its value is invalid (expected '2.0').");
  c.VerifyMethodRequest("some.method_1", "1");
}

/*

TEST_CASE_FIXTURE(F, "method_error") {
  c.SetError(JsonRpcException{-32602, "invalid method name"});
  REQUIRE_THROWS_WITH(client.CallMethod<json>("1", "some.method_1", {}), Contains("-32602") && Contains("invalid method name") && !Contains("data"));
  c.VerifyMethodRequest("some.method_1", "1");
}

TEST_CASE_FIXTURE(F, "method_error_with_data") {
  c.SetError(JsonRpcException{-32602, "invalid method name", {1, 2}});
  REQUIRE_THROWS_WITH(client.CallMethod<json>("1", "some.method_1", {}),
                      Contains("-32602") && Contains("invalid method name") && Contains("data") && Contains("[1,2]"));
  c.VerifyMethodRequest("some.method_1", "1");
}

TEST_CASE_FIXTURE(F, "method_error_invalid_json") {
  c.raw_response = "{asdfasdf,[}";
  REQUIRE_THROWS_WITH(client.CallMethod<json>("1", "some.method_1", {}), Contains("-32700") && Contains("invalid") && Contains("JSON") && Contains("server"));
  c.VerifyMethodRequest("some.method_1", "1");
  c.raw_response = " ";
  REQUIRE_THROWS_WITH(client.CallMethod<json>("1", "some.method_1", {}), Contains("-32700") && Contains("invalid") && Contains("JSON") && Contains("server"));
  c.VerifyMethodRequest("some.method_1", "1");
  c.raw_response = "";
  REQUIRE_THROWS_WITH(client.CallMethod<json>("1", "some.method_1", {}), Contains("-32700") && Contains("invalid") && Contains("JSON") && Contains("server"));
  c.VerifyMethodRequest("some.method_1", "1");
}

TEST_CASE_FIXTURE(F, "notification_call_no_params") {
  c.raw_response = "";
  client.CallNotification("some.notification_1", {});
  c.VerifyNotificationRequest("some.notification_1");
  CHECK(!c.request.contains("params"));

  c.raw_response = "";
  client.CallNotification("some.notification_1");
  c.VerifyNotificationRequest("some.notification_1");
  CHECK(!c.request.contains("params"));
}

TEST_CASE_FIXTURE(F, "notification_call_params_byname") {
  c.raw_response = "";
  client.CallNotificationNamed("some.notification_1", {{"a", "hello"}, {"b", 77}, {"c", true}});
  c.VerifyNotificationRequest("some.notification_1");
  CHECK(c.request["params"]["a"] == "hello");
  CHECK(c.request["params"]["b"] == 77);
  CHECK(c.request["params"]["c"] == true);
}

TEST_CASE_FIXTURE(F, "notification_call_params_byposition") {
  c.raw_response = "";
  client.CallNotification("some.notification_1", {"hello", 77, true});
  c.VerifyNotificationRequest("some.notification_1");
  CHECK(c.request["params"][0] == "hello");
  CHECK(c.request["params"][1] == 77);
  CHECK(c.request["params"][2] == true);
}
*/

// TODO: test cases with return type mapping and param mapping for v1/v2 method and notification
