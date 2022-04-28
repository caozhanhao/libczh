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
		class Value
		{
		private:
			std::variant<int, double, std::string,
				std::vector<int>, std::vector<double>, std::vector<std::string>,
				Value*> value;
			std::type_index value_type;
		public:
			template<typename T>
			Value(const T& data)
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

			template <typename T>
			Value& operator=(const T& v)
			{
				value = v;
				value_type = typeid(T);
				return *this;
			}
			Value& operator=(const char* v)
			{
				value = std::string(v);
				value_type = typeid(std::string);
				return *this;
			}
		};

		template <>
		Value& Value::operator=(const Value& v)
		{
			value = v.value;
			value_type = typeid(Value*);
			return *this;
		}
	}
}