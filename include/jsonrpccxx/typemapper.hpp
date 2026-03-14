#pragma once

#include "nlohmann/json.hpp"
#include "jsonrpccxx/exception.hpp"

#include <cstddef>

#include <typeinfo>


#include <type_traits>
#include <memory>
#include <string>

#include <functional>
#include <limits>
#include <utility>
#include <string>
#include <variant>
#include <deque>
#include<set>
#include <unordered_map>
#include <unordered_set>

namespace std
{
  // Forward declarations for STL containers
  template<typename T, typename Alloc> class vector;
  template<typename T, std::size_t N> class array;
  template <typename Key, typename T, typename Compare, typename Alloc> class map;
  template <typename... Ts> struct tuple;
}

namespace jsonrpc
{

template<typename T> constexpr auto cpp_type_name() noexcept
{
#if defined(__clang__)
  constexpr std::string_view name{__PRETTY_FUNCTION__};
  constexpr std::string_view prefix{"[T = "};
  constexpr std::string_view suffix{"]"};
#elif defined(__GNUC__)
  constexpr std::string_view name{__PRETTY_FUNCTION__};
  constexpr std::string_view prefix{"[with T = "};
  constexpr std::string_view suffix{"]"};
#elif defined(_MSC_VER)
  constexpr std::string_view name{__FUNCSIG__};
  constexpr std::string_view prefix{"cpp_type_name<"};
  constexpr std::string_view suffix{">(void)"};
#endif
  constexpr std::size_t beg{name.find(prefix)};
  constexpr std::size_t end{name.find(suffix)};
  return name.substr(beg+prefix.size(),end - (beg + prefix.size()));
}



    // -------------------- Type name mapping --------------------
    [[nodiscard]] constexpr std::string_view type_name(const nlohmann::json::value_t& t) noexcept
    {
      switch (t)
      {
        case nlohmann::json::value_t::null:              return "null";
        case nlohmann::json::value_t::object:            return "object";
        case nlohmann::json::value_t::array:             return "array";
        case nlohmann::json::value_t::string:            return "string";
        case nlohmann::json::value_t::boolean:           return "boolean";
        case nlohmann::json::value_t::number_integer:    return "integer";
        case nlohmann::json::value_t::number_unsigned:   return "unsigned integer";
        case nlohmann::json::value_t::number_float:      return "float";
        case nlohmann::json::value_t::discarded:         return "discarded";
        default:                                         return "unknown";
      }
    }



class param_t
{
public:
  param_t() = default;
  param_t(std::string_view cppType, nlohmann::json::value_t jt) : cpp_type(std::move(cppType)), json_type(jt) {}
  std::string getType() const noexcept { return cpp_type; }
  std::string getJSONType() const noexcept { return std::string(type_name(json_type)); }

private:
  std::string cpp_type;
  nlohmann::json::value_t json_type;
};

class Method
{
public:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
  Method(const Method&) = delete;
  Method& operator=(const Method&) = delete;
  Method(Method&&) = default;
  Method& operator=(Method&&) = default;
  Method(std::function<nlohmann::json(const nlohmann::json &)> f,const std::vector<param_t>& params) : m_method(std::move(f)) , m_params(std::move(params)){}
#pragma GCC diagnostic pop
  nlohmann::json operator()(const nlohmann::json & request) const
  { 
    return m_method(normalize_parameter(request));
  }
  void setParameterNames(const std::vector<std::string>& names)
  {
    m_names = names;
  }
  std::size_t arity() const noexcept { return m_params.size(); }
  std::vector<param_t> getParameters() const noexcept { return m_params; }
  const std::vector<std::string> getNames()
  {
    return m_names;
  }
private:
  nlohmann::json normalize_parameter(const nlohmann::json &params) const
  {
    if(params.is_array())
    {
        testParameters(params.size());
        return params;
    }
    else if(params.is_object())
    {
      if(m_names.empty()) throw exception(invalid_params, "invalid parameter: procedure doesn't support named parameter");
      nlohmann::json result;
      for(auto const &p : m_names)
      {
        if(params.find(p) == params.end()) throw exception(invalid_params, "invalid parameter: missing named parameter \"" + p + "\"");
        result.push_back(params[p]);
      }
      testParameters(params.size());
      return result;
    }
    throw exception(invalid_request, "invalid request: the 'params' field must be either an array or an object.");
  }


  void testParameters(const std::size_t json_params_size) const
  {
    if(json_params_size != m_params.size()) throw exception(invalid_params,"invalid parameter: expected " + std::to_string(m_params.size()) + " argument(s), but found " + std::to_string(json_params_size));
  }
  std::function<nlohmann::json(const nlohmann::json &)> m_method;
  const std::vector<param_t> m_params;
  std::vector<std::string> m_names;
};

class Notification
{
public:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
  Notification(std::function<void(const nlohmann::json &)> f,const std::vector<param_t>& params) : m_notif(std::move(f)) , m_params(std::move(params)){}
  Notification(const Method&) = delete;
  Notification& operator=(const Method&) = delete;
  Notification(Notification&&) = default;
  Notification& operator=(Notification&&) = default;
#pragma GCC diagnostic pop
  void operator()(const nlohmann::json & request) const
  { 
    m_notif(normalize_parameter(request));
  }
  std::size_t arity() const noexcept { return m_params.size(); }
  std::vector<param_t> getParameters() const noexcept { return m_params; }
  void setParameterNames(const std::vector<std::string>& names)
  {
    m_names = names;
  }
  const std::vector<std::string> getNames()
  {
    return m_names;
  }
private:
nlohmann::json normalize_parameter(const nlohmann::json &params) const
{
  if(params.is_array())
  {
      testParameters(params.size());
      return params;
  }
  else if(params.is_object())
  {
    if(m_names.empty()) throw exception(invalid_params, "invalid parameter: procedure doesn't support named parameter");
    nlohmann::json result;
    for(auto const &p : m_names)
    {
      if(params.find(p) == params.end()) throw exception(invalid_params, "invalid parameter: missing named parameter \"" + p + "\"");
      result.push_back(params[p]);
    }
    testParameters(params.size());
    return result;
  }
  throw exception(invalid_request, "invalid request: the 'params' field must be either an array or an object.");
}
  void testParameters(const std::size_t json_params_size) const
  {
    if(json_params_size != m_params.size()) throw exception(invalid_params,"invalid parameter: expected " + std::to_string(m_params.size()) + " argument(s), but found " + std::to_string(json_params_size));
  }
  std::function<void(const nlohmann::json &)> m_notif;
  const std::vector<param_t> m_params;
  std::vector<std::string> m_names;
};

// ================== Compile-time type mapping ==================
template<typename T, typename Enable = void> struct type_mapper { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::object; };
// -------------------- Basic types --------------------
template<> struct type_mapper<void> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::null; };
template<> struct type_mapper<bool> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::boolean; };
template<> struct type_mapper<char> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::string; };
template<> struct type_mapper<signed char> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::number_integer; };
template<> struct type_mapper<unsigned char> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::number_unsigned; };
template<> struct type_mapper<std::string> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::string; };
// -------------------- Numeric types --------------------
template<typename T> struct type_mapper<T, typename std::enable_if_t<std::is_floating_point<T>::value>> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::number_float; };
template<typename T> struct type_mapper<T, typename std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value>> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::number_integer; };
template<typename T> struct type_mapper<T, typename std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value>> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::number_unsigned; };
// -------------------- Enum types --------------------
template<typename T> struct type_mapper<T, typename std::enable_if_t<std::is_enum<T>::value>> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::string; };
// -------------------- Optional --------------------
template<typename T> struct type_mapper<std::optional<T>> { static constexpr nlohmann::json::value_t value = type_mapper<T>::value; };
// -------------------- Variant --------------------
template<typename... Ts> struct type_mapper<std::variant<Ts...>> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::object; };
// -------------------- Pair / Tuple --------------------
template<typename T1, typename T2> struct type_mapper<std::pair<T1, T2>> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::array; };
template<typename... Ts> struct type_mapper<std::tuple<Ts...>> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::array; };
// -------------------- Sequence containers --------------------
template<typename T> struct type_mapper<std::vector<T>> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::array; };
template<typename T, std::size_t N> struct type_mapper<std::array<T, N>> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::array; };
template<typename T> struct type_mapper<std::deque<T>> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::array; };
template<typename T> struct type_mapper<std::set<T>> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::array; };
template<typename T> struct type_mapper<std::unordered_set<T>> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::array; };
// -------------------- Map containers --------------------
template<typename K, typename V> struct type_mapper<std::map<K, V>> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::object; };
template<typename K, typename V> struct type_mapper<std::unordered_map<K, V>> { static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::object; };

// -------------------- Parameter type checking --------------------
template<typename T> inline void check_param_type(const std::size_t index,const nlohmann::json& x,const nlohmann::json::value_t expectedType)
{
        if constexpr (std::is_arithmetic_v<T>) {
            // Handle unsigned expectation but JSON is signed
            if (expectedType == nlohmann::json::value_t::number_unsigned && x.type() == nlohmann::json::value_t::number_integer) {
                if (x.get<long long>() < 0) {
                    throw exception(
                        invalid_params,
                        "invalid parameter: must be " + std::string(type_name(expectedType)) +
                        ", but is " + std::string(type_name(x.type())),
                        std::to_string(index)
                    );
                }
            }
            // Handle signed expectation but JSON is unsigned
            else if (expectedType == nlohmann::json::value_t::number_integer && x.type() == nlohmann::json::value_t::number_unsigned) {
                if (x.get<unsigned long long>() > static_cast<unsigned long long>(std::numeric_limits<T>::max())) {
                    throw exception(
                        invalid_params,
                        "invalid parameter: exceeds value range of " + std::string(type_name(expectedType)),
                        std::to_string(index)
                    );
                }
            }
            // Handle float expectation but JSON is integer or unsigned
            else if (expectedType == nlohmann::json::value_t::number_float &&
                     (x.type() == nlohmann::json::value_t::number_integer || x.type() == nlohmann::json::value_t::number_unsigned)) {
                double val = x.get<double>();
                if (val > std::numeric_limits<T>::max() || val < std::numeric_limits<T>::lowest()) {
                    throw exception(
                        invalid_params,
                        "invalid parameter: exceeds value range of " + std::string(type_name(expectedType)),
                        std::to_string(index)
                    );
                }
            }
            // Type mismatch
            else if (x.type() != expectedType) {
                throw exception(
                    invalid_params,
                    "invalid parameter: must be " + std::string(type_name(expectedType)) +
                    ", but is " + std::string(type_name(x.type())),
                    std::to_string(index)
                );
            }
        } else {
            // Non-arithmetic types: just check type
            if (x.type() != expectedType) {
                throw exception(
                    invalid_params,
                    "invalid parameter: must be " + std::string(type_name(expectedType)) +
                    ", but is " + std::string(type_name(x.type())),
                    std::to_string(index)
                );
            }
        }
    }
    
    // ================= Free function Methods =================
    template<typename ReturnType, typename... ParamTypes, std::size_t... I>
    Method createMethod(std::function<ReturnType(ParamTypes...)> method, std::index_sequence<I...>)
    {
        std::vector<param_t> params = { param_t(cpp_type_name<ParamTypes>(), type_mapper<std::decay_t<ParamTypes>>::value)... };
        return Method([method](const nlohmann::json &params) -> nlohmann::json {
            
            // Check parameter types
            (check_param_type<std::decay_t<ParamTypes>>(I, params[I], type_mapper<std::decay_t<ParamTypes>>::value), ...);
    
            // Call function
            return method(params[I].get<std::decay_t<ParamTypes>>()...);
        },std::move(params));
    }
    
    template<typename ReturnType, typename... ParamTypes>
    Method GetHandle(std::function<ReturnType(ParamTypes...)> f)
    {
        return createMethod(f, std::index_sequence_for<ParamTypes...>{});
    }
    
    template<typename ReturnType, typename... ParamTypes>
    Method GetHandle(ReturnType (*f)(ParamTypes...))
    {
        return createMethod(std::function<ReturnType(ParamTypes...)>(f), std::index_sequence_for<ParamTypes...>{});
    }
    
    // ================= Member function Methods (non-const) =================
    template<typename T, typename ReturnType, typename... ParamTypes, std::size_t... I>
    Method createMethod(ReturnType (T::*method)(ParamTypes...), T &instance, std::index_sequence<I...>)
    {
        std::vector<param_t> params = { param_t(cpp_type_name<ParamTypes>(), type_mapper<std::decay_t<ParamTypes>>::value)... };
        return Method([&instance, method](const nlohmann::json &params) -> nlohmann::json {

            (check_param_type<std::decay_t<ParamTypes>>(I, params[I], type_mapper<std::decay_t<ParamTypes>>::value), ...);
    
            return (instance.*method)(params[I].get<std::decay_t<ParamTypes>>()...);
        },std::move(params));
    }
    
    template<typename T, typename ReturnType, typename... ParamTypes>
    Method GetHandle(ReturnType (T::*method)(ParamTypes...), T &instance)
    {
        return createMethod(method, instance, std::index_sequence_for<ParamTypes...>{});
    }
    
    // ================= Member function Methods (const) =================
    template<typename T, typename ReturnType, typename... ParamTypes, std::size_t... I>
    Method createMethod(ReturnType (T::*method)(ParamTypes...) const, const T &instance, std::index_sequence<I...>)
    {
        std::vector<param_t> params = { param_t(cpp_type_name<ParamTypes>(), type_mapper<std::decay_t<ParamTypes>>::value)... };
        return Method([&instance, method](const nlohmann::json &params) -> nlohmann::json {

            (check_param_type<std::decay_t<ParamTypes>>(I, params[I], type_mapper<std::decay_t<ParamTypes>>::value), ...);
    
            return (instance.*method)(params[I].get<std::decay_t<ParamTypes>>()...);
        },std::move(params));
    }
    
    template<typename T, typename ReturnType, typename... ParamTypes>
    Method GetHandle(ReturnType (T::*method)(ParamTypes...) const, const T &instance)
    {
        return createMethod(method, instance, std::index_sequence_for<ParamTypes...>{});
    }
    
    // ================= Free function Notifications =================
    template<typename... ParamTypes, std::size_t... I>
    Notification createNotification(std::function<void(ParamTypes...)> method, std::index_sequence<I...>)
    {
        std::vector<param_t> params = { param_t(cpp_type_name<ParamTypes>(), type_mapper<std::decay_t<ParamTypes>>::value)... };
        return Notification([method](const nlohmann::json &params) {

            (check_param_type<std::decay_t<ParamTypes>>(I, params[I], type_mapper<std::decay_t<ParamTypes>>::value), ...);
    
            method(params[I].get<std::decay_t<ParamTypes>>()...);
        },std::move(params));
    }
    
    template<typename... ParamTypes>
    Notification GetHandle(std::function<void(ParamTypes...)> f)
    {
        return createNotification(f, std::index_sequence_for<ParamTypes...>{});
    }
    
    template<typename... ParamTypes>
    Notification GetHandle(void (*f)(ParamTypes...))
    {
        return createNotification(std::function<void(ParamTypes...)>(f), std::index_sequence_for<ParamTypes...>{});
    }
    
    // ================= Member function Notifications (non-const) =================
    template<typename T, typename... ParamTypes, std::size_t... I>
    Notification createNotification(void (T::*method)(ParamTypes...), T &instance, std::index_sequence<I...>)
    {
        std::vector<param_t> params = { param_t(cpp_type_name<ParamTypes>(), type_mapper<std::decay_t<ParamTypes>>::value)... };
        return Notification([&instance, method](const nlohmann::json &params) {
    
            (check_param_type<std::decay_t<ParamTypes>>(I, params[I], type_mapper<std::decay_t<ParamTypes>>::value), ...);
    
            (instance.*method)(params[I].get<std::decay_t<ParamTypes>>()...);
        },std::move(params));
    }
    
    template<typename T, typename... ParamTypes>
    Notification GetHandle(void (T::*method)(ParamTypes...), T &instance)
    {
        return createNotification(method, instance, std::index_sequence_for<ParamTypes...>{});
    }
    
    // ================= Member function Notifications (const) =================
    template<typename T, typename... ParamTypes, std::size_t... I>
    Notification createNotification(void (T::*method)(ParamTypes...) const, const T &instance, std::index_sequence<I...>)
    {

        std::vector<param_t> params = { param_t(cpp_type_name<ParamTypes>(), type_mapper<std::decay_t<ParamTypes>>::value)... };
        return Notification([&instance, method](const nlohmann::json &params) {
    
            (check_param_type<std::decay_t<ParamTypes>>(I, params[I], type_mapper<std::decay_t<ParamTypes>>::value), ...);
    
            (instance.*method)(params[I].get<std::decay_t<ParamTypes>>()...);
        },std::move(params));
    }
    
    template<typename T, typename... ParamTypes>
    Notification GetHandle(void (T::*method)(ParamTypes...) const, const T &instance)
    {
        return createNotification(method, instance, std::index_sequence_for<ParamTypes...>{});
    }

} // namespace json