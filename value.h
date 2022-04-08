#pragma once
#include "err.h"
#include <variant>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <typeinfo>
#include <typeindex>

namespace czh
{
  namespace value
  {
	template <typename VT>
	std::string vector_string(const VT& v)
	{
	  std::string result = "[";
	  for (auto it = v.cbegin(); it != (v.cend() - 1); ++it)
	  {
		result += std::to_string(*it);
		result += ",";
	  }
	  result += std::to_string(*(v.cend() - 1));
	  result += "]";
	  return result;
	}
	template <>
	std::string vector_string(const std::vector<std::string>& v)
	{
	  std::string result = "";
	  result += "[";
	  for (auto it = v.cbegin(); it != (v.cend() - 1); ++it)
	  {
		result += "\"";
		result += *it;
		result += "\"";
		result += ",";
	  }
	  result += "\"";
	  result += *(v.cend() - 1);
	  result += "\"";
	  result += "]";
	  return result;
	}

	class Value
	{
	private:
	  std::variant<int,double,std::string,
			std::vector<int>, std::vector<double>,std::vector<std::string>,
		  Value*> value;
		std::type_index value_type;
	public:
	  template<typename T>
	  Value(const T & data)
		: value(data), value_type(typeid(T)) {  }
		Value() :value(0), value_type(typeid(void)) {  }
	  template <typename T>
	  T get() const
	  {
			if (value_type == std::type_index(typeid(Value*)))
				return std::get<Value*>(value)->get<T>();
			return std::get<T>(value);
	  }

	  std::type_index type() const
		{
			return value_type; 
		}

	 // std::string get_string() const
	 // {
		//auto t = type();
		//if (t == typeid(int))
		//  return std::to_string(get<int>());
		//else if (t == typeid(std::string))
		//  return ("\"" + get<std::string>() + "\"");
		//else if (t == typeid(double))
		//  return std::to_string(get<double>());
		//else if (t == typeid(std::vector<int>))
		//  return vector_string(get<std::vector<int>>());
		//else if (t == typeid(std::vector<std::string>))
		//  return vector_string(get<std::vector<std::string>>());
		//else if (t == typeid(std::vector<double>))
		//  return vector_string(get<std::vector<double>>());

		////Value*
		//if (t != typeid(Value*)) 
		//  throw Err(CZH_ERROR_LOCATION, __func__, "unexpected error", Err::is_ICE);
		//std::string res;
		//auto path = last_node.lock()->get_path();
		//for (auto it = path->crbegin(); it < path->crend(); it++)
		//{
		//  res += "-";
		//  res += *it;
		//}
		//res += ":";
		//res += get<Value*>()->name;
		//return res;
	 // }
	  template <typename T>
	  Value& operator=(const T& v)
	  {
		value = v;
		return *this;
	  }
	};
	template <>
	Value& Value::operator=(const Value& v)
	{
		value = v.value;
		return *this;
	}
  }
}