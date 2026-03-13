#pragma once

#include "jsonrpccxx/typemapper.hpp"
#include "typemapper.hpp"
#include <unordered_map>
#include <string>
#include <vector>

namespace jsonrpc
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

  bool Add(const std::string &name,Method callback, const NamedParamMapping &mapping = {})
  {
    if(contains(name)) return false;
    methods.try_emplace(name,std::make_unique<Method>(std::move(callback)));
    methods[name]->setParameterNames(mapping);
    if(!mapping.empty()) this->mapping[name] = mapping;
    return true;
  }

  bool Add(const std::string &name,Notification callback, const NamedParamMapping &mapping = {})
  {
    if(contains(name)) return false;
    notifications.try_emplace(name,std::make_unique<Notification>(std::move(callback)));
    notifications[name]->setParameterNames(mapping);
    if(!mapping.empty()) this->mapping[name] = mapping;
    return true;
  }

  exception process_type_error(const std::string& name,const exception &e) const
  {
    if(e.Code() == invalid_params && !e.Data().empty())
    {
      std::string message = e.Message() + " for parameter ";
      const auto found = mapping.find(name);
      if(found != mapping.end()) message += "\"" + found->second[std::stoi(e.Data())] + "\"";
      else message += e.Data();
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
      return method->second.get()->operator()(normalize_parameter(name, params));
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
      return notification->second.get()->operator()(normalize_parameter(name, params));
    }
    catch(const exception &e)
    {
      throw process_type_error(name, e);
    }
  }

  inline bool contains(const std::string& name) const noexcept { return (methods.find(name) != methods.end() || notifications.find(name) != notifications.end()); }

  nlohmann::json listMethods() const
  {
    nlohmann::json json = nlohmann::json::array();
    for (const auto& [name, methodPtr] : methods)
    {
        nlohmann::json j;
        j["name"] = name;
        j["params"] = nlohmann::json::array();
        for (const auto& param : methodPtr->getParameters())
        {
            nlohmann::json p;
            p["cpp_type"] = param.getType();
            p["json_type"] = param.getJSONType();
            j["params"].push_back(p);
        }
        j["names"] = methodPtr->getNames();
        json.push_back(j);
    }
    return json;
  }
private:
  std::unordered_map<std::string, std::unique_ptr<Method>> methods;
  std::unordered_map<std::string, std::unique_ptr<Notification>> notifications;
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

} // namespace jsonrpc
