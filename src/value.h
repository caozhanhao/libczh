//   Copyright 2022 caozhanhao
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
#pragma once
#include "err.h"
#include <variant>
#include <string>
#include <vector>
#include <typeinfo>
#include <type_traits>

namespace czh
{
  namespace node
  {
    class Node;
  }
  namespace value
  {
    template<typename... List>
    struct TypeList
    {
    };
    
    template<typename... List>
    std::variant<List...> as_variant(TypeList<List...>);
    
    template<typename List1, typename List2>
    struct Link;
    template<typename ... Args1, typename ... Args2>
    struct Link<TypeList<Args1...>, TypeList<Args2...>>
    {
      using type = TypeList<Args1..., Args2...>;
    };
    
    template<typename T, typename... List>
    struct Contains : std::true_type
    {
    };
    
    template<typename T, typename First, typename... Rest>
    struct Contains<T, TypeList<First, Rest...>>
        : std::conditional<std::is_same<T, First>::value, std::true_type, Contains<T, Rest...>>::type
    {
    };
    template<typename T>
    struct Contains<T> : std::false_type
    {
    };
    
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
          types{"int", "long long", "double", "std::string", "bool", "czh::node::Node*", "czh::value::Array"};
      return types[index];
    }
    
    using BasicVTList = TypeList<int, long long, double, std::string, bool>;
    using BasicVT = decltype(as_variant(BasicVTList{}));
    
    using Array = std::vector<BasicVT>;//insert() begin() end()
    
    using HighVTList = TypeList<node::Node *, Array>;
    using VTList = Link<BasicVTList, HighVTList>::type;
    using VT = decltype(as_variant(VTList{}));
    
    struct ValueTag
    {
    };
    struct NormalArrayTag
    {
    };
    struct AnyArrayTag
    {
    };
    struct ErrorTag
    {
    };
    
    template<typename T, typename = void>
    struct is_normal_array
    {
      static constexpr bool value = !std::is_same_v<T, std::string>
                                    && Contains<typename T::value_type, BasicVTList>::value;
    };
    
    template<typename T>
    struct is_normal_array<T, std::enable_if_t<(std::is_fundamental_v<T> || std::is_same_v<T, node::Node *>)>>
    {
      static constexpr bool value = false;
    };
    
    template<typename T>
    struct TagDispatch
    {
      using tag = std::conditional_t<
          std::is_same_v<T, Array>, AnyArrayTag,
          std::conditional_t<is_normal_array<T>::value, NormalArrayTag,
              std::conditional_t<(Contains<T, BasicVTList>::value || std::is_same_v<T, node::Node *>), ValueTag,
                  ErrorTag>>>;
    };
    
    class Value
    {
    private:
      VT value;
    public:
      template<typename T, typename = typename std::enable_if_t<
          Contains<T, BasicVTList>::value || std::is_same_v<T, node::Node *>>>
      explicit Value(T &&data)
          : value(std::forward<T>(data))
      {}
      
      template<typename T, typename = typename std::enable_if_t<
          Contains<typename T::value_type, BasicVTList>::value && !std::is_same_v<T, std::string>>>
      explicit Value(const T &data)
      {
        value.emplace<Array>();
        auto &arr = std::get<Array>(value);
        for (auto &r: arr)
        {
          arr.insert(arr.end(), r);
        }
      }
      
      explicit Value(const char *v)
          : value(std::string(v))
      {}
      
      Value(Value &&) = default;
      
      Value(const Value &) = default;
      
      Value() : value(0)
      {}
      
      template<typename T>
      T get() const
      {
        return internal_get<T>(typename TagDispatch<T>::tag{});
      }
      
      template<typename T>
      bool is() const
      {
        return value.index() == IndexOf<T, VTList>::value;
      }
      
      [[nodiscard]] const auto &get_variant() const
      {
        return value;
      }
      
      template<typename T>
      std::enable_if_t<Contains<T, BasicVTList>::value, Value &> operator=(const T &v)
      {
        value = v;
        return *this;
      }
      
      template<typename T>
      std::enable_if_t<std::is_same_v<T, Array>
                       || Contains<typename T::value_type, BasicVTList>::value, Value &>
      operator=(T &&v)
      {
        if constexpr(std::is_same_v<Array, T>)
        {
          value = v;
        }
        else
        {
          Array tmp;
          for (auto &r: v)
          {
            tmp.insert(tmp.end(), r);
          }
        }
        return *this;
      }
      
      template<typename T, typename =  std::enable_if_t<
          Contains<T, BasicVTList>::value>>
      Value &operator=(const std::initializer_list<T> &data)
      {
        Array tmp;
        for (auto &r: data)
        {
          tmp.insert(tmp.end(), r);
        }
        value = tmp;
        return *this;
      }
      
      template<typename T>
      Value &operator=(Array &&data)
      {
        value = std::forward<T>(data);
        return *this;
      }
      
      Value &operator=(const Value &v)
      {
        value = v.value;
        return *this;
      }
      
      Value &operator=(const char *v)
      {
        value = std::string(v);
        return *this;
      }
    
    private:
      template<typename T>
      T internal_get(ValueTag) const
      {
        if (!is<T>())
        {
          get_error_index<T>();
        }
        return std::get<T>(value);
      }
      
      template<typename T>
      T internal_get(NormalArrayTag) const
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
      T internal_get(ErrorTag) const
      {
        get_error_index<T>();
        return T();
      }
      
      template<typename T>
      Array internal_get(AnyArrayTag) const
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