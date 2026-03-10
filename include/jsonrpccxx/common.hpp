#pragma once
#include "nlohmann/json.hpp"
#include <string>

namespace jsonrpccxx {
  typedef nlohmann::json json;

  static inline bool has_key_type(const json &v, const std::string &key, json::value_t type) { return v.contains(key) && v.at(key).type() == type; }
  static inline bool valid_id(const json &request) {
    return request.contains("id") && (request["id"].is_number() || request["id"].is_string() || request["id"].is_null());
  }
  static inline bool valid_id_not_null(const json &request) { return request.contains("id") && (request["id"].is_number() || request["id"].is_string()); }

} // namespace jsonrpccxx
