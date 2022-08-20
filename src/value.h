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
#include <typeindex>

namespace czh
{
  namespace node
  {
    class Node;
  }
  namespace value
  {
    using BasicVT = std::variant<int, long long, double, std::string, bool>;
    using Array = std::vector<BasicVT>;
    using ArrayValueType = BasicVT;
    
    std::string get_type_str(const std::type_index &type)
    {
      return type.name();
    }
    
    class Value
    {
    private:
      std::variant<int, long long, double, std::string, bool, char, Array, node::Node *> value;
      std::type_index value_type;
    public:
      template<typename T>
      explicit Value(const T &data)
          : value(data), value_type(typeid(T))
      {}
      
      explicit Value(const char *v)
          : value(std::string(v)), value_type(typeid(std::string))
      {}
      
      Value() : value(0), value_type(typeid(void))
      {}
      
      template<typename T>
      T get() const
      {
        if constexpr(std::is_same_v<std::vector<int>, T>
                     || std::is_same_v<std::vector<long long>, T>
                     || std::is_same_v<std::vector<double>, T>
                     || std::is_same_v<std::vector<std::string>, T>
                     || std::is_same_v<std::vector<bool>, T>)
        {
          return get_array<T>();
        }
        else if constexpr(std::is_same_v<Array, T>)
        {
          return get_any_array();
        }
        else
        {
          if (std::type_index(typeid(T)) != value_type)
          {
            throw error::Error(LIBCZH_ERROR_LOCATION, __func__,
                               "The value is '" + get_type_str(type()) + "', not '"
                               + get_type_str(typeid(T)) + "'.");
          }
          return std::get<T>(value);
        }
      }
      
      [[nodiscard]] const auto &get_variant() const
      {
        return value;
      }
      
      [[nodiscard]] std::type_index type() const
      {
        return value_type;
      }
      
      template<typename T>
      Value &operator=(const T &v)
      {
        value = v;
        value_type = typeid(T);
        return *this;
      }
  
      Value &operator=(const char *v)
      {
        value = std::string(v);
        value_type = typeid(std::string);
        return *this;
      }

    private:
      template<typename T>
      T get_array() const
      {
        if (std::type_index(typeid(Array)) != value_type)
        {
          throw error::Error(LIBCZH_ERROR_LOCATION, __func__,
                             "The value is '" + get_type_str(type()) + "', not '"
                             + get_type_str(typeid(Array)) + "'.");
        }
        auto &varr = std::get<Array>(value);
        T ret;
        for (auto &r: varr)
        {
          if (auto pval = std::get_if<typename T::value_type>(&r))
          {
            ret.emplace_back(*pval);
          }
          else
          {
            throw error::Error(LIBCZH_ERROR_LOCATION, __func__,
                               "This array contains different types.");
          }
        }
        return std::move(ret);
      }
  
      Array get_any_array() const
      {
        if (std::type_index(typeid(Array)) != value_type)
        {
          throw error::Error(LIBCZH_ERROR_LOCATION, __func__,
                             "The value is '" + get_type_str(type()) + "', not '"
                             + get_type_str(typeid(Array)) + "'.");
        }
        return std::get<Array>(value);
      }
    };
    
    template<>
    Value &Value::operator=(const Value &v)
    {
      value = v.value;
      value_type = typeid(Value *);
      return *this;
    }
  }
}