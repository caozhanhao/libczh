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
  namespace node{class Node;}
	namespace value
	{
    class Note
    {
    public:
      std::string note;
      explicit Note(std::string note_) : note(std::move(note_)){}
    };
		class Value
		{
		private:
			std::variant<int, long long, double, std::string, Note, bool,
				std::vector<int>, std::vector<long long>, std::vector<double>, std::vector<std::string>,std::vector<bool>,
				node::Node*> value;
			std::type_index value_type;
		public:
			template<typename T>
		 	explicit Value(const T& data)
				: value(data), value_type(typeid(T)) {  }
			explicit Value(const char* v)
				: value(std::string(v)), value_type(typeid(std::string)) {}
			Value() :value(0), value_type(typeid(void)) {  }
			template <typename T>
			T get() const
			{
				return std::get<T>(value);
			}
      
      [[nodiscard]] std::type_index type() const
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