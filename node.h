#pragma once 

#include "value.h"
#include "err.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>


using czh::value::Value;
using czh::error::Err;
namespace czh
{
	namespace node
	{
		class Node
		{
		private:
			std::string name;
			Node* last_node;
			std::map<std::string, Value> val;
			std::map<std::string, Node> node;
		public:
			Node(Node* node_ptr, const std::string& node_name)
				:name(node_name), last_node(node_ptr) {}

			Node() :name("/"), last_node(nullptr) {  }

			template <typename T>
			void add(const std::string& name, const T& _value)
			{
				val[name] = Value(_value);
			}

			Node* add_node(const std::string& name)
			{
				node[name] = Node(this, name);
				return &node[name];
			}

			template <typename T>
			std::unique_ptr<std::map<std::string, T>> value_map()
			{
				std::unique_ptr<std::map<std::string, T>> result =
					std::make_unique<std::map<std::string, T>>(std::map<std::string, T>());

				std::type_index value_type(typeid(T));
				std::type_index node_type(typeid(Node));
				for (auto& r : val)
				{
					if (r.second.type() != value_type)
						throw Err(CZH_ERROR_LOCATION, __func__, "Type is not same.", Err::internal);
					else if (r.second.type() == node_type)
						throw Err(CZH_ERROR_LOCATION, __func__, "Type is Node.", Err::internal);
					else
						(*result)[r.first] = r.second.get<T>();
				}
				return result;
			}

			Node* to_last_node() const
			{
				return last_node;
			}
			Node* get_root() const
			{
				auto last = to_last_node();
				for (; last->name != "/"; last = last->to_last_node());
				return last;
			}
			Value& operator[](const std::string& s)
			{
				if (val.find(s) == val.end())
					error("There is no node named '" + s + "'.");
				return val[s];
			}
			const Value& operator[](const std::string& s) const
			{
				if (val.find(s) == val.end())
					error("There is no node named '" + s + "'.");
				return val.at(s);
			}

			template<typename Arg>
			Node& operator()(const Arg& arg)
			{
				return in(arg);
			}
			template<typename Arg, typename ...Args>
			Node& operator()(const Arg& arg, const Args&... args)
			{
				auto& temp = in(arg);
				return temp.operator()(args...);
			}

			std::unique_ptr<std::vector<std::string>> get_path() const
			{
				auto n_ptr = to_last_node();
				std::vector<std::string> res;
				res.push_back(name);
				while (n_ptr != nullptr)
				{
					res.push_back(n_ptr->name);
					n_ptr = n_ptr->to_last_node();
				}
				return std::move(std::make_unique<decltype(res)>(res));
			}

			bool has_node(const std::string& name) const
			{
				return (node.find(name) != node.end());
			}
		private:
			Node& in(const std::string& s)
			{
				if (node.find(s) == node.end())
					error("There is no node named '" + s + "'.");
				return node.at(s);
			}
			void error(const std::string& str) const
			{
				throw Err(CZH_ERROR_LOCATION, __func__, str, Err::internal);
			}
		};
	}
}