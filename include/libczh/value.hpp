//   Copyright 2021-2022 libczh - caozhanhao
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
#ifndef LIBCZH_VALUE_HPP
#define LIBCZH_VALUE_HPP

#include "err.hpp"
#include <variant>
#include <string>
#include <vector>
#include <typeinfo>
#include <type_traits>
#include <ostream>
#include <deque>
#include <set>
#include <queue>
#include <list>

namespace czh
{
  namespace node
  {
    class Node;
  }
  namespace value
  {
    class Null {};
    
    template<typename... List>
    struct TypeList {};
    
    template<typename... List>
    std::variant<List...> as_variant(TypeList<List...>);
    
    template<typename List1, typename List2>
    struct Link;
    template<typename ... Args1, typename ... Args2>
    struct Link<TypeList<Args1...>, TypeList<Args2...>>
    {
      using type = TypeList<Args1..., Args2...>;
    };
    
    template<typename T, typename List>
    struct Contains : std::true_type {};
    template<typename T, typename First, typename... Rest>
    struct Contains<T, TypeList<First, Rest...>>
        : std::conditional<std::is_same<T, First>::value, std::true_type,
            Contains<T, TypeList<Rest...>>>::type
    {
    };
    template<typename T>
    struct Contains<T, TypeList<>> : std::false_type {};
    
    template<typename T, typename List>
    struct IndexOf;
    template<typename First, typename ... Rest>
    struct IndexOf<First, TypeList<First, Rest...>>
    {
      const static int value = 0;
    };
    template<typename T>
    struct IndexOf<T, TypeList<>>
    {
      const static int value = -1;
    };
    template<typename T, typename First, typename ...Rest>
    struct IndexOf<T, TypeList<First, Rest...>>
    {
      const static int temp = IndexOf<T, TypeList<Rest...>>::value;
      const static int value = temp == -1 ? -1 : temp + 1;
    };
    
    std::string VTstr(std::size_t index)
    {
      static std::vector<std::string>
          types{"Null", "int", "long long", "double", "bool", "const char*", "std::string", "czh::node::Node*",
                "czh::value::Array"};
      return types[index];
    }
    
    using BasicVTList = TypeList<Null, int, long long, double, bool, const char *, std::string>;
    using BasicVT = decltype(as_variant(BasicVTList{}));
    
    using Array = std::vector<BasicVT>;//insert() begin() end()
    
    using HighVTList = TypeList<node::Node *, Array>;
    using VTList = Link<BasicVTList, HighVTList>::type;
    using VT = decltype(as_variant(VTList{}));

#define LIBCZH_HasMMaker(MEMBER, ...)\
template <class T, class = std::void_t<>>\
    struct HasM##MEMBER : std::false_type {};\
    template <class T>\
    struct HasM##MEMBER<T, std::void_t<decltype(std::declval<T>().MEMBER(__VA_ARGS__))>> : std::true_type {};\

    LIBCZH_HasMMaker(insert, std::declval<T>().end(), 0)
    LIBCZH_HasMMaker(begin)
    LIBCZH_HasMMaker(cbegin)
    LIBCZH_HasMMaker(end)
    LIBCZH_HasMMaker(cend)
#undef LIBCZH_HasMMaker
    
    template<typename T, typename U = void>
    struct IsNormalArray : public std::false_type {};
    template<typename T>
    struct IsNormalArray<T, std::enable_if_t<!std::is_same_v<T, std::string>
                                             && Contains<typename T::value_type, BasicVTList>::value>>
        : public std::true_type
    {
    };
    
    
    struct ValueTag {};
    struct NormalArrayTag {};
    struct AnyArrayTag {};
    template<typename T>
    struct TagDispatch
    {
      using tag = std::conditional_t<
          std::is_same_v<T, Array>, AnyArrayTag,
          std::conditional_t<IsNormalArray<T>::value, NormalArrayTag, ValueTag>>;
    };
    
    class Value
    {
    private:
      VT value;
    public:
      template<typename T, typename = std::enable_if_t<!std::is_base_of_v<Value, std::decay_t<T>>>>
      explicit Value(T &&data)
      {
        *this = std::forward<T>(data);
      }
      
      Value(Value &&) = default;
      
      Value(const Value &) = default;
      
      Value() : value(0) {}
      
      template<typename T>
      [[nodiscard]]T get() const
      {
        static_assert(Contains<T, VTList>::value || IsNormalArray<T>::value,
                      "T must be in VTList(value.hpp, BasicVTList + HighVTList),"
                      " or a container that stores Type in BasicVTList(value.h).");
        if constexpr(IsNormalArray<T>::value)
        {
          static_assert((HasMend<T>::value || HasMcend<T>::value)
                        && HasMinsert<T>::value && std::is_default_constructible_v<T>,
                        "The container must have end() (or cend()) and insert() and is default constructible.");
        }
        return internal_get<T>(typename TagDispatch<T>::tag{});
      }
      
      template<typename T>
      Value &operator=(T &&v)
      {
        static_assert(Contains<T, VTList>::value || IsNormalArray<T>::value,
                      "T must be in VTList(value.hpp, BasicVTList + HighVTList),"
                      " or a container that stores Type in BasicVTList(value.h).");
        if constexpr(IsNormalArray<T>::value)
        {
          static_assert((HasMbegin<T>::value || HasMcbegin<T>::value) &&
                        (HasMend<T>::value || HasMcend<T>::value) && HasMend<T>::value,
                        "The container must have begin() (or cbegin()) and end() (or cend()).");
        }
        internal_equal<T>(std::forward<T>(v), typename TagDispatch<T>::tag{});
        return *this;
      }
      
      Value &operator=(Value &&v) = default;
      
      Value &operator=(const char *v)
      {
        value = std::string(v);
        return *this;
      }
      
      template<typename T>
      [[nodiscard]]bool is() const
      {
        return value.index() == IndexOf<T, VTList>::value;
      }
      
      [[nodiscard]] auto get_variant() const
      {
        return value;
      }
    
    private:
      template<typename T>
      void internal_equal(T &&v, ValueTag) { value = std::forward<T>(v); }
      
      template<typename T>
      void internal_equal(T &&v, AnyArrayTag) { value = std::forward<T>(v); }
      
      template<typename T>
      void internal_equal(T &&v, NormalArrayTag)
      {
        Array tmp;
        auto arr = std::forward<T>(v);
        for (auto r: arr)
        {
          tmp.insert(tmp.end(), r);
        }
        value = std::move(tmp);
      }
      
      template<typename T>
      [[nodiscard]]T internal_get(ValueTag) const
      {
        if (!is<T>())
        {
          get_error_index<T>();
        }
        return std::get<T>(value);
      }
      
      template<typename T>
      [[nodiscard]]T internal_get(NormalArrayTag) const
      {
        if (!is<Array>())
        {
          get_error_index<T>();
        }
        auto &varr = std::get<Array>(value);
        T ret;
        for (auto &r: varr)
        {
          if (auto pval = std::get_if<typename T::value_type>(&r))
          {
            ret.insert(std::end(ret), *pval);
          }
          else
          {
            throw error::Error(LIBCZH_ERROR_LOCATION, __func__,
                               "This array contains different types.");
          }
        }
        return std::move(ret);
      }
      
      template<typename T>
      [[nodiscard]]Array internal_get(AnyArrayTag) const
      {
        if (!is<Array>())
        {
          get_error_index<T>();
        }
        return std::get<Array>(value);
      }
      
      template<typename T>
      void get_error_index() const
      {
        throw error::Error(LIBCZH_ERROR_LOCATION, __func__,
                           "Get wrong type.[wrong T = '" + VTstr(IndexOf<T, VTList>::value)
                           + "', correct T = '" + VTstr(value.index())
                           + "']");
      }
    };
    
  }
}
#endif