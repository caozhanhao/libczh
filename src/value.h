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
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <typeinfo>
#include <typeindex>
#include <any>
#include <cxxabi.h>

namespace czh
{
  namespace node
  {
    class Node;
  }
  namespace value
  {
    std::string get_type_str(const std::type_index &type)
    {
      return abi::__cxa_demangle(type.name(), 0, 0, 0);
    }
    
    class Value
    {
    public:
      using BasicVT = std::variant<int, long long, double, std::string, bool>;
      using AnyArray = std::vector<BasicVT>;
      using AnyArrayValueType = BasicVT;
    private:
      std::variant<int, long long, double, std::string, bool, char,
          std::vector<int>, std::vector<long long>, std::vector<double>,
          std::vector<std::string>, std::vector<bool>,
          AnyArray,
          node::Node *> value;
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
      const T &get() const
      {
        if (std::type_index(typeid(T)) != value_type)
          throw error::Error(CZH_ERROR_LOCATION, __func__,
                             "The value is '" + get_type_str(type()) + "', not '"
                             + get_type_str(typeid(T)) + "'.");
        return std::get<T>(value);
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