#pragma once

#include "jsonrpccxx/typemapper.hpp"
#include <unordered_map>
#include <string>
#include <vector>

namespace jsonrpccxx
{

using NamedParamMapping = std::vector<std::string>;
  
class Dispatcher
{
public:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
  explicit Dispatcher() = default;
#pragma GCC diagnostic pop
  ~Dispatcher() noexcept = default;

  bool Add(const std::string &name, MethodHandle callback, const NamedParamMapping &mapping = {})
  {
    if(contains(name)) return false;
    methods[name] = std::move(callback);
    if(!mapping.empty()) this->mapping[name] = mapping;
    return true;
  }

  bool Add(const std::string &name, NotificationHandle callback, const NamedParamMapping &mapping = {})
  {
    if(contains(name)) return false;
    notifications[name] = std::move(callback);
    if(!mapping.empty()) this->mapping[name] = mapping;
    return true;
  }

  exception process_type_error(const std::string& name,const exception &e) const
  {
    if(e.Code() == invalid_params && !e.Data().empty())
    {
      std::string message = e.Message() + " for parameter ";
      const auto found = mapping.find(name);
      if(found != mapping.end()) message += "\"" + found->second[e.Data().get<std::size_t>()] + "\"";
      else message += std::to_string(e.Data().get<unsigned int>());
      return exception(e.Code(), message);
    }
    else return e;
  }

  nlohmann::json InvokeMethod(const std::string &name, const nlohmann::json &params) const
  {
    auto method = methods.find(name);
    if(method == methods.end()) throw exception(method_not_found, "method not found: " + name);
    try
    {
      return method->second(normalize_parameter(name, params));
    }
    catch(const nlohmann::json::type_error &e)
    {
      throw exception(invalid_params, "invalid parameter: " + std::string(e.what()));
    }
    catch(const exception &e)
    {
      throw process_type_error(name, e);
    }
  }

  void InvokeNotification(const std::string &name, const nlohmann::json &params) const
  {
    auto notification = notifications.find(name);
    if(notification == notifications.end()) throw exception(method_not_found, "notification not found: " + name);
    try
    {
      notification->second(normalize_parameter(name, params));
    }
    catch(const exception &e)
    {
      throw process_type_error(name, e);
    }
  }

  inline bool contains(const std::string& name) const noexcept { return (methods.find(name) != methods.end() || notifications.find(name) != notifications.end()); }
private:
  std::unordered_map<std::string, MethodHandle> methods;
  std::unordered_map<std::string, NotificationHandle> notifications;
  std::unordered_map<std::string, NamedParamMapping> mapping;

  inline nlohmann::json normalize_parameter(const std::string &name, const nlohmann::json &params) const
  {
    if(params.is_array()) return params;
    else if(params.is_object())
    {
      const auto found = mapping.find(name);
      if(found == mapping.end()) throw exception(invalid_params, "invalid parameter: procedure doesn't support named parameter");
      nlohmann::json result;
      for(auto const &p : found->second)
      {
        if(params.find(p) == params.end()) throw exception(invalid_params, "invalid parameter: missing named parameter \"" + p + "\"");
        result.push_back(params[p]);
      }
      return result;
    }
    throw exception(invalid_request, "invalid request: the 'params' field must be either an array or an object.");
  }

};

} // namespace jsonrpccxx
