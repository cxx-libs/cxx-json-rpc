#pragma once

#include "nlohmann/json.hpp"
#include "jsonrpccxx/exception.hpp"

#include <cstddef>



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

namespace jsonrpccxx
{

typedef std::function<json(const nlohmann::json &)> MethodHandle;
typedef std::function<void(const nlohmann::json &)> NotificationHandle;

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

// -------------------- Parameter type checking --------------------
template<typename T> inline void check_param_type(const std::size_t index,const json& x,const json::value_t expectedType)
{
        if constexpr (std::is_arithmetic_v<T>) {
            // Handle unsigned expectation but JSON is signed
            if (expectedType == json::value_t::number_unsigned && x.type() == json::value_t::number_integer) {
                if (x.get<long long>() < 0) {
                    throw JsonRpcException(
                        invalid_params,
                        "invalid parameter: must be " + std::string(type_name(expectedType)) +
                        ", but is " + std::string(type_name(x.type())),
                        index
                    );
                }
            }
            // Handle signed expectation but JSON is unsigned
            else if (expectedType == json::value_t::number_integer && x.type() == json::value_t::number_unsigned) {
                if (x.get<unsigned long long>() > static_cast<unsigned long long>(std::numeric_limits<T>::max())) {
                    throw JsonRpcException(
                        invalid_params,
                        "invalid parameter: exceeds value range of " + std::string(type_name(expectedType)),
                        index
                    );
                }
            }
            // Handle float expectation but JSON is integer or unsigned
            else if (expectedType == json::value_t::number_float &&
                     (x.type() == json::value_t::number_integer || x.type() == json::value_t::number_unsigned)) {
                double val = x.get<double>();
                if (val > std::numeric_limits<T>::max() || val < std::numeric_limits<T>::lowest()) {
                    throw JsonRpcException(
                        invalid_params,
                        "invalid parameter: exceeds value range of " + std::string(type_name(expectedType)),
                        index
                    );
                }
            }
            // Type mismatch
            else if (x.type() != expectedType) {
                throw JsonRpcException(
                    invalid_params,
                    "invalid parameter: must be " + std::string(type_name(expectedType)) +
                    ", but is " + std::string(type_name(x.type())),
                    index
                );
            }
        } else {
            // Non-arithmetic types: just check type
            if (x.type() != expectedType) {
                throw JsonRpcException(
                    invalid_params,
                    "invalid parameter: must be " + std::string(type_name(expectedType)) +
                    ", but is " + std::string(type_name(x.type())),
                    index
                );
            }
        }
    }

    // -------------------- MethodHandle --------------------

    template <typename ReturnType, typename... ParamTypes, std::size_t... index>
    MethodHandle createMethodHandle(std::function<ReturnType(ParamTypes...)> method, std::index_sequence<index...>) {
        return [method](const json &params) -> json {
            constexpr size_t formalSize = sizeof...(ParamTypes);
            const size_t actualSize = params.size();

            if (actualSize != formalSize) {
                throw JsonRpcException(
                    invalid_params,
                    "invalid parameter: expected " + std::to_string(formalSize) +
                    " argument(s), but found " + std::to_string(actualSize)
                );
            }

            (check_param_type<std::decay_t<ParamTypes>>(index, params[index], type_mapper<std::decay_t<ParamTypes>>::value), ...);

            return method(params[index].get<std::decay_t<ParamTypes>>()...);
        };
    }

    template <typename ReturnType, typename... ParamTypes>
    MethodHandle methodHandle(std::function<ReturnType(ParamTypes...)> method) {
        return createMethodHandle(method, std::index_sequence_for<ParamTypes...>{});
    }

    template <typename ReturnType, typename... ParamTypes>
    MethodHandle GetHandle(std::function<ReturnType(ParamTypes...)> f) {
        return methodHandle(f);
    }

    template <typename ReturnType, typename... ParamTypes>
    MethodHandle GetHandle(ReturnType (*f)(ParamTypes...)) {
        return GetHandle(std::function<ReturnType(ParamTypes...)>(f));
    }

    inline MethodHandle GetUncheckedHandle(std::function<json(const json&)> f) {
        return [f](const json &params) -> json { return f(params); };
    }

// -------------------- NotificationHandle --------------------
template<typename... ParamTypes, std::size_t... index> NotificationHandle createNotificationHandle(std::function<void(ParamTypes...)> method, std::index_sequence<index...>)
{
  return [method](const json& params) -> void 
  {
    constexpr std::size_t formalSize{sizeof...(ParamTypes)};
    const std::size_t actualSize{params.size()};
    if(actualSize != formalSize) throw JsonRpcException(invalid_params,"invalid parameter: expected " + std::to_string(formalSize) + " argument(s), but found " + std::to_string(actualSize));
    (check_param_type<std::decay_t<ParamTypes>>(index, params[index], type_mapper<std::decay_t<ParamTypes>>::value), ...);
    method(params[index].get<std::decay_t<ParamTypes>>()...);
  };
}

template<typename... ParamTypes> NotificationHandle notificationHandle(std::function<void(ParamTypes...)> method)
{
  return createNotificationHandle(method, std::index_sequence_for<ParamTypes...>{});
}

template<typename... ParamTypes> NotificationHandle GetHandle(std::function<void(ParamTypes...)> f) { return notificationHandle(f); }

template<typename... ParamTypes> NotificationHandle GetHandle(void (*f)(ParamTypes...)) { return GetHandle(std::function<void(ParamTypes...)>(f)); }

inline NotificationHandle GetUncheckedNotificationHandle(std::function<void(const json&)> f) { return [f](const json &params) -> void { f(params); }; }

// -------------------- Member methods --------------------
template<typename T, typename ReturnType, typename... ParamTypes> MethodHandle methodHandle(ReturnType (T::*method)(ParamTypes...), T &instance)
{
  auto function = [&instance, method](ParamTypes &&... params) -> ReturnType
  {
    return (instance.*method)(std::forward<ParamTypes>(params)...);
  };
  return methodHandle(std::function<ReturnType(ParamTypes...)>(function));
}

template<typename T, typename... ParamTypes> NotificationHandle notificationHandle(void (T::*method)(ParamTypes...), T &instance)
{
  auto function = [&instance, method](ParamTypes &&... params) -> void 
  {
    (instance.*method)(std::forward<ParamTypes>(params)...);
  };
  return notificationHandle(std::function<void(ParamTypes...)>(function));
}

template<typename T, typename ReturnType, typename... ParamTypes> MethodHandle GetHandle(ReturnType (T::*method)(ParamTypes...), T &instance)
{
  auto function = [&instance, method](ParamTypes &&... params) -> ReturnType 
  {
    return (instance.*method)(std::forward<ParamTypes>(params)...);
  };
  return GetHandle(std::function<ReturnType(ParamTypes...)>(function));
}

template<typename T, typename... ParamTypes> NotificationHandle GetHandle(void (T::*method)(ParamTypes...), T &instance)
{
  auto function = [&instance, method](ParamTypes &&... params) -> void
  {
    (instance.*method)(std::forward<ParamTypes>(params)...);
  };
  return GetHandle(std::function<void(ParamTypes...)>(function));
}

// -------------------- Const member methods --------------------

    template<typename T, typename ReturnType, typename... ParamTypes>
    MethodHandle GetHandle(ReturnType (T::*method)(ParamTypes...) const, const T &instance) {
        auto function = [&instance, method](ParamTypes &&... params) -> ReturnType {
            return (instance.*method)(std::forward<ParamTypes>(params)...);
        };
        return GetHandle(std::function<ReturnType(ParamTypes...)>(function));
    }

    template<typename T, typename... ParamTypes>
    NotificationHandle GetHandle(void (T::*method)(ParamTypes...) const, const T &instance) {
        auto function = [&instance, method](ParamTypes &&... params) -> void {
            (instance.*method)(std::forward<ParamTypes>(params)...);
        };
        return GetHandle(std::function<void(ParamTypes...)>(function));
    }

    template <typename T, typename ReturnType, typename... ParamTypes>
    MethodHandle methodHandle(ReturnType (T::*method)(ParamTypes...) const, const T &instance) {
        auto function = [&instance, method](ParamTypes &&... params) -> ReturnType {
            return (instance.*method)(std::forward<ParamTypes>(params)...);
        };
        return methodHandle(std::function<ReturnType(ParamTypes...)>(function));
    }

    template <typename T, typename... ParamTypes>
    NotificationHandle notificationHandle(void (T::*method)(ParamTypes...) const, const T &instance) {
        auto function = [&instance, method](ParamTypes &&... params) -> void {
            (instance.*method)(std::forward<ParamTypes>(params)...);
        };
        return notificationHandle(std::function<void(ParamTypes...)>(function));
    }

} // namespace jsonrpccxx