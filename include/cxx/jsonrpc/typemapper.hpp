#pragma once

#include "cxx/jsonrpc/exception.hpp"
#include "cxx/jsonrpc/export.hpp"
#include "error_code.hpp"
#include "nlohmann/json.hpp"

#include <cstddef>
#include <deque>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>

namespace std
{
// Forward declarations for STL containers
template<typename T, typename Alloc> class vector;
template<typename T, std::size_t N> class array;
template<typename Key, typename T, typename Compare, typename Alloc> class map;
}  // namespace std

namespace jsonrpc
{

template<typename T> [[nodiscard]] constexpr auto cpp_type_name() noexcept
{
#if defined( __clang__ )
  constexpr std::string_view name{ __PRETTY_FUNCTION__ };
  constexpr std::string_view prefix{ "[T = " };
  constexpr std::string_view suffix{ "]" };
#elif defined( __GNUC__ )
  constexpr std::string_view name{ __PRETTY_FUNCTION__ };
  constexpr std::string_view prefix{ "[with T = " };
  constexpr std::string_view suffix{ "]" };
#elif defined( _MSC_VER )
  constexpr std::string_view name{ __FUNCSIG__ };
  constexpr std::string_view prefix{ "cpp_type_name<" };
  constexpr std::string_view suffix{ ">(void)" };
#endif
  constexpr std::size_t beg{ name.find( prefix ) };
  constexpr std::size_t end{ name.find( suffix ) };
  return name.substr( beg + prefix.size(), end - ( beg + prefix.size() ) );
}

// -------------------- Type name mapping --------------------
[[nodiscard]] constexpr std::string_view type_name( const nlohmann::json::value_t& t ) noexcept
{
  switch( t )
  {
    case nlohmann::json::value_t::null: return "null";
    case nlohmann::json::value_t::object: return "object";
    case nlohmann::json::value_t::array: return "array";
    case nlohmann::json::value_t::string: return "string";
    case nlohmann::json::value_t::boolean: return "boolean";
    case nlohmann::json::value_t::number_integer: return "integer";
    case nlohmann::json::value_t::number_unsigned: return "unsigned integer";
    case nlohmann::json::value_t::number_float: return "float";
    case nlohmann::json::value_t::discarded: return "discarded";
    default: return "unknown";
  }
}

// ================== Compile-time type mapping ==================
template<typename T, typename Enable = void> struct type_mapper
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::object;
};
// -------------------- Basic types --------------------
template<> struct type_mapper<void>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::null;
};
template<> struct type_mapper<bool>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::boolean;
};
template<> struct type_mapper<char>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::string;
};
template<> struct type_mapper<signed char>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::number_integer;
};
template<> struct type_mapper<unsigned char>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::number_unsigned;
};
template<> struct type_mapper<std::string>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::string;
};
// -------------------- Numeric types --------------------
template<typename T> struct type_mapper<T, typename std::enable_if_t<std::is_floating_point<T>::value>>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::number_float;
};
template<typename T> struct type_mapper<T, typename std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value>>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::number_integer;
};
template<typename T> struct type_mapper<T, typename std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value>>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::number_unsigned;
};
// -------------------- Enum types --------------------
template<typename T> struct type_mapper<T, typename std::enable_if_t<std::is_enum<T>::value>>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::string;
};
// -------------------- Optional --------------------
template<typename T> struct type_mapper<std::optional<T>>
{
  static constexpr nlohmann::json::value_t value = type_mapper<T>::value;
};
// -------------------- Variant --------------------
template<typename... Ts> struct type_mapper<std::variant<Ts...>>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::object;
};
// -------------------- Pair / Tuple --------------------
template<typename T1, typename T2> struct type_mapper<std::pair<T1, T2>>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::array;
};
template<typename... Ts> struct type_mapper<std::tuple<Ts...>>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::array;
};
// -------------------- Sequence containers --------------------
template<typename T> struct type_mapper<std::vector<T>>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::array;
};
template<typename T, std::size_t N> struct type_mapper<std::array<T, N>>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::array;
};
template<typename T> struct type_mapper<std::deque<T>>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::array;
};
template<typename T> struct type_mapper<std::set<T>>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::array;
};
template<typename T> struct type_mapper<std::unordered_set<T>>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::array;
};
// -------------------- Map containers --------------------
template<typename K, typename V> struct type_mapper<std::map<K, V>>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::object;
};
template<typename K, typename V> struct type_mapper<std::unordered_map<K, V>>
{
  static constexpr nlohmann::json::value_t value = nlohmann::json::value_t::object;
};

class param_t
{
public:
#if !defined( _WIN32 )
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Weffc++"
#endif
  CXX_JSONRPC_API                                     param_t() = default;
  template<typename T> static CXX_JSONRPC_API param_t from_type() { return param_t( cpp_type_name<T>(), type_mapper<std::decay_t<T>>::value ); }

private:
  param_t( std::string_view cppType, nlohmann::json::value_t jt ) : cpp_type( cppType ), json_type( jt ) {}
#if !defined( _WIN32 )
  #pragma GCC diagnostic pop
#endif
public:
  CXX_JSONRPC_API const std::string_view getType() const noexcept { return cpp_type; }
  CXX_JSONRPC_API const std::string_view getJSONType() const noexcept { return type_name( json_type ); }
  CXX_JSONRPC_API std::string getName() const noexcept { return m_name; }
  CXX_JSONRPC_API void        setName( const std::string& name ) { m_name = name; }

private:
  std::string                   m_name;
  const std::string_view        cpp_type;
  const nlohmann::json::value_t json_type;
};

template<typename ReturnType> class Procedure
{
public:
#if !defined( _WIN32 )
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Weffc++"
#endif
  Procedure( const Procedure& )            = delete;
  Procedure& operator=( const Procedure& ) = delete;
  Procedure( Procedure&& )                 = default;
  Procedure& operator=( Procedure&& )      = default;

  // Constructor takes a callable and parameter descriptions
  CXX_JSONRPC_API Procedure( const std::function<ReturnType( const nlohmann::json& )> f, const std::vector<param_t> params, const std::optional<std::string_view> description ) :
    m_callable( std::move( f ) ), m_params( std::move( params ) ), m_description( description ? description.value() : std::string() )
  {
  }
#if !defined( _WIN32 )
  #pragma GCC diagnostic pop
#endif

  // Invoke function
  CXX_JSONRPC_API ReturnType operator()( const nlohmann::json& request ) const
  {
    try
    {
      nlohmann::json args = normalize_parameter( request );
      return m_callable( args );  // works for both void and nlohmann::json
    }
    catch( const exception& e )
    {
      throw process_type_error( e );
    }
  }

  // Parameter helpers
  CXX_JSONRPC_API void setParameterNames( const std::vector<std::string>& names )
  {
    if( arity() != names.size() ) throw exception( server_error, "Callback arity (" + std::to_string( arity() ) + ") does not match mapping size (" + std::to_string( names.size() ) + ')' );
    if( !names.empty() ) m_isNamed = true;

    for( std::size_t i = 0; i < names.size(); ++i ) m_params[i].setName( names[i] );
  }

  CXX_JSONRPC_API std::size_t arity() const noexcept { return m_params.size(); }
  CXX_JSONRPC_API std::vector<param_t> getParameters() const noexcept { return m_params; }
  CXX_JSONRPC_API std::string_view getDescription() const noexcept { return m_description; }

private:
  nlohmann::json normalize_parameter( const nlohmann::json& params ) const
  {
    if( params.is_array() )
    {
      testParameters( params.size() );
      return params;
    }
    else if( params.is_object() )
    {
      if( !m_isNamed ) throw exception( invalid_params, "invalid parameter: procedure doesn't support named parameter" );

      nlohmann::json result;
      for( const auto& param: m_params )
      {
        if( params.find( param.getName() ) == params.end() ) throw exception( invalid_params, "invalid parameter: missing named parameter \"" + param.getName() + "\"" );  // FIXME crash using '
        result.push_back( params[param.getName()] );
      }
      testParameters( params.size() );
      return result;
    }
    throw exception( invalid_request, "invalid request: the 'params' field must be either an array or an object" );
  }

  void testParameters( std::size_t json_params_size ) const
  {
    if( json_params_size != m_params.size() ) throw exception( invalid_params, "invalid parameter: expected " + std::to_string( m_params.size() ) + " argument(s), but found " + std::to_string( json_params_size ) );
  }

  exception process_type_error( const exception& e ) const
  {
    if( e.code() == invalid_params && !e.data().empty() ) { return exception( e.code(), e.message() + " for parameter " + ( m_isNamed ? '\'' + m_params[std::stoi( e.data() )].getName() + '\'' : e.data() ) ); }
    return e;
  }

private:
  const std::function<ReturnType( const nlohmann::json& )> m_callable;
  std::vector<param_t>                                     m_params;
  const std::string                                        m_description;
  bool                                                     m_isNamed{ false };
};
// Method: returns JSON
using Method       = Procedure<nlohmann::json>;
// Notification: returns void
using Notification = Procedure<void>;

// -------------------- Parameter type checking --------------------
template<typename T> CXX_JSONRPC_API inline void check_param_type( const std::size_t index, const nlohmann::json& x, const nlohmann::json::value_t expectedType )
{
  if constexpr( std::is_arithmetic_v<T> )
  {
    // Handle unsigned expectation but JSON is signed
    if( expectedType == nlohmann::json::value_t::number_unsigned && x.type() == nlohmann::json::value_t::number_integer )
    {
      if( x.get<long long>() < 0 ) throw exception( invalid_params, "invalid parameter: must be " + std::string( type_name( expectedType ) ) + ", but is " + std::string( type_name( x.type() ) ), std::to_string( index ) );
    }
    // Handle signed expectation but JSON is unsigned
    else if( expectedType == nlohmann::json::value_t::number_integer && x.type() == nlohmann::json::value_t::number_unsigned )
    {
      if( x.get<unsigned long long>() > static_cast<unsigned long long>( ( std::numeric_limits<T>::max )() ) ) throw exception( invalid_params, "invalid parameter: exceeds value range of " + std::string( type_name( expectedType ) ), std::to_string( index ) );
    }
    // Handle float expectation but JSON is integer or unsigned
    else if( expectedType == nlohmann::json::value_t::number_float && ( x.type() == nlohmann::json::value_t::number_integer || x.type() == nlohmann::json::value_t::number_unsigned ) )
    {
      double val = x.get<double>();
      if( val > ( std::numeric_limits<T>::max )() || val < std::numeric_limits<T>::lowest() ) throw exception( invalid_params, "invalid parameter: exceeds value range of " + std::string( type_name( expectedType ) ), std::to_string( index ) );
    }
    // Type mismatch
    else if( x.type() != expectedType )
      throw exception( invalid_params, "invalid parameter: must be " + std::string( type_name( expectedType ) ) + ", but is " + std::string( type_name( x.type() ) ), std::to_string( index ) );
  }
  else
  {
    // Non-arithmetic types: just check type
    if( x.type() != expectedType ) throw exception( invalid_params, "invalid parameter: must be " + std::string( type_name( expectedType ) ) + ", but is " + std::string( type_name( x.type() ) ), std::to_string( index ) );
  }
}

// ================= Free function Methods =================
template<typename ReturnType, typename... ParamTypes, std::size_t... I> CXX_JSONRPC_API Method createMethod( std::function<ReturnType( ParamTypes... )> method, std::index_sequence<I...>, std::optional<std::string_view> m_description )
{
  std::vector<param_t> params = { param_t::from_type<ParamTypes>()... };
  return Method(
    [method]( const nlohmann::json& params ) -> nlohmann::json
    {
      // Check parameter types
      ( check_param_type<std::decay_t<ParamTypes>>( I, params[I], type_mapper<std::decay_t<ParamTypes>>::value ), ... );
      // Call function
      return method( params[I].get<std::decay_t<ParamTypes>>()... );
    },
    std::move( params ), m_description );
}

template<typename ReturnType, typename... ParamTypes> CXX_JSONRPC_API Method GetHandle( std::function<ReturnType( ParamTypes... )> f, std::optional<std::string_view> m_description = std::nullopt )
{ return createMethod( f, std::index_sequence_for<ParamTypes...>{}, m_description ); }

template<typename ReturnType, typename... ParamTypes> CXX_JSONRPC_API Method GetHandle( ReturnType ( *f )( ParamTypes... ), std::optional<std::string_view> m_description = std::nullopt )
{ return createMethod( std::function<ReturnType( ParamTypes... )>( f ), std::index_sequence_for<ParamTypes...>{}, m_description ); }

// ================= Member function Methods (non-const) =================
template<typename T, typename ReturnType, typename... ParamTypes, std::size_t... I> CXX_JSONRPC_API Method createMethod( ReturnType ( T::*method )( ParamTypes... ), T& instance, std::index_sequence<I...>, std::optional<std::string_view> m_description )
{
  std::vector<param_t> params = { param_t::from_type<ParamTypes>()... };
  return Method(
    [&instance, method]( const nlohmann::json& params ) -> nlohmann::json
    {
      ( check_param_type<std::decay_t<ParamTypes>>( I, params[I], type_mapper<std::decay_t<ParamTypes>>::value ), ... );
      return ( instance.*method )( params[I].get<std::decay_t<ParamTypes>>()... );
    },
    std::move( params ), m_description );
}

template<typename T, typename ReturnType, typename... ParamTypes> CXX_JSONRPC_API Method GetHandle( ReturnType ( T::*method )( ParamTypes... ), T& instance, std::optional<std::string_view> m_description = std::nullopt )
{ return createMethod( method, instance, std::index_sequence_for<ParamTypes...>{}, m_description ); }

// ================= Member function Methods (const) =================
template<typename T, typename ReturnType, typename... ParamTypes, std::size_t... I>
CXX_JSONRPC_API Method createMethod( ReturnType ( T::*method )( ParamTypes... ) const, const T& instance, std::index_sequence<I...>, std::optional<std::string_view> m_description )
{
  std::vector<param_t> params = { param_t::from_type<ParamTypes>()... };
  return Method(
    [&instance, method]( const nlohmann::json& params ) -> nlohmann::json
    {
      ( check_param_type<std::decay_t<ParamTypes>>( I, params[I], type_mapper<std::decay_t<ParamTypes>>::value ), ... );
      return ( instance.*method )( params[I].get<std::decay_t<ParamTypes>>()... );
    },
    std::move( params ), m_description );
}

template<typename T, typename ReturnType, typename... ParamTypes> CXX_JSONRPC_API Method GetHandle( ReturnType ( T::*method )( ParamTypes... ) const, const T& instance, std::optional<std::string> m_description = std::nullopt )
{ return createMethod( method, instance, std::index_sequence_for<ParamTypes...>{}, m_description ); }

// ================= Free function Notifications =================
template<typename... ParamTypes, std::size_t... I> CXX_JSONRPC_API Notification createNotification( std::function<void( ParamTypes... )> method, std::index_sequence<I...>, std::optional<std::string_view> m_description )
{
  std::vector<param_t> params = { param_t::from_type<ParamTypes>()... };
  return Notification(
    [method]( const nlohmann::json& params )
    {
      ( check_param_type<std::decay_t<ParamTypes>>( I, params[I], type_mapper<std::decay_t<ParamTypes>>::value ), ... );
      method( params[I].get<std::decay_t<ParamTypes>>()... );
    },
    std::move( params ), m_description );
}

template<typename... ParamTypes> CXX_JSONRPC_API Notification GetHandle( std::function<void( ParamTypes... )> f, std::optional<std::string_view> m_description = std::nullopt )
{ return createNotification( f, std::index_sequence_for<ParamTypes...>{}, m_description ); }

template<typename... ParamTypes> CXX_JSONRPC_API Notification GetHandle( void ( *f )( ParamTypes... ), std::optional<std::string_view> m_description = std::nullopt )
{ return createNotification( std::function<void( ParamTypes... )>( f ), std::index_sequence_for<ParamTypes...>{}, m_description ); }

// ================= Member function Notifications (non-const) =================
template<typename T, typename... ParamTypes, std::size_t... I> CXX_JSONRPC_API Notification createNotification( void ( T::*method )( ParamTypes... ), T& instance, std::index_sequence<I...>, std::optional<std::string_view> m_description )
{
  std::vector<param_t> params = { param_t::from_type<ParamTypes>()... };
  return Notification(
    [&instance, method]( const nlohmann::json& params )
    {
      ( check_param_type<std::decay_t<ParamTypes>>( I, params[I], type_mapper<std::decay_t<ParamTypes>>::value ), ... );
      ( instance.*method )( params[I].get<std::decay_t<ParamTypes>>()... );
    },
    std::move( params ), m_description );
}

template<typename T, typename... ParamTypes> CXX_JSONRPC_API Notification GetHandle( void ( T::*method )( ParamTypes... ), T& instance, std::optional<std::string_view> m_description = std::nullopt )
{ return createNotification( method, instance, std::index_sequence_for<ParamTypes...>{}, m_description ); }

// ================= Member function Notifications (const) =================
template<typename T, typename... ParamTypes, std::size_t... I> CXX_JSONRPC_API Notification createNotification( void ( T::*method )( ParamTypes... ) const, const T& instance, std::index_sequence<I...>, std::optional<std::string_view> m_description )
{
  std::vector<param_t> params = { param_t::from_type<ParamTypes>()... };
  return Notification(
    [&instance, method]( const nlohmann::json& params )
    {
      ( check_param_type<std::decay_t<ParamTypes>>( I, params[I], type_mapper<std::decay_t<ParamTypes>>::value ), ... );
      ( instance.*method )( params[I].get<std::decay_t<ParamTypes>>()... );
    },
    std::move( params ), m_description );
}

template<typename T, typename... ParamTypes> CXX_JSONRPC_API Notification GetHandle( void ( T::*method )( ParamTypes... ) const, const T& instance, std::optional<std::string_view> m_description = std::nullopt )
{ return createNotification( method, instance, std::index_sequence_for<ParamTypes...>{}, m_description ); }

}  // namespace jsonrpc
