#pragma once 

#include "value.h"
#include "err.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>


using czh::value::Value;
using czh::error::Error;
namespace czh
{
	namespace node
	{
		template <typename VT>
		std::string vector_to_string(const VT& v)
		{
			std::string result = "[";
			for (auto it = v.cbegin(); it != (v.cend() - 1); ++it)
			{
				result += std::to_string(*it);
				result += ", ";
			}
			result += std::to_string(*(v.cend() - 1));
			result += "]";
			return result;
		}
    template <>
    std::string vector_to_string(const std::vector<bool>& v)
    {
      std::string result = "[";
      for (auto it = v.cbegin(); it != (v.cend() - 1); ++it)
      {
        result += (*it ? "true" : "false");
        result += ", ";
      }
      result += (*(v.cend() - 1) ? "true" : "false");
      result += "]";
      return result;
    }
		template <>
		std::string vector_to_string(const std::vector<std::string>& v)
		{
			std::string result = "";
			result += "[";
			for (auto it = v.cbegin(); it != (v.cend() - 1); ++it)
			{
				result += "\"";
				result += *it;
				result += "\"";
				result += ", ";
			}
			result += "\"";
			result += *(v.cend() - 1);
			result += "\"";
			result += "]";
			return result;
		}

		class Node
		{
			friend std::ostream& operator<<(std::ostream&, const Node&);
		public:
			static const bool disable_output = false;
		private:
			std::string name;
			Node* last_node;
			std::map<std::string, Node> node;
			Value value;
			bool outputable;
			bool is_node;
			std::shared_ptr<std::vector<std::string>> output_list;
		public:
			Node(Node* node_ptr, const std::string& node_name, bool _outputable = true)
				:name(node_name), last_node(node_ptr), outputable(_outputable), is_node(true)
			{
				if (outputable)
					output_list = std::make_shared<std::vector<std::string>>();
			}
			Node(Node* node_ptr, const std::string& node_name, const Value& val, bool _outputable = true)
				:name(node_name), last_node(node_ptr), value(val),
				outputable(_outputable), is_node(false)
			{
				if (outputable)
					output_list = std::make_shared<std::vector<std::string>>();
			}
			Node(bool _outputable = true)
				:name("/"), last_node(nullptr), outputable(_outputable), is_node(true)
			{
				if (outputable)
					output_list = std::make_shared<std::vector<std::string>>();
			}
      std::string get_name()const{return name;}
			void remove()
			{
				if (last_node->outputable)
				{
					auto& l = *last_node->output_list;
					for (auto it = l.begin(); it < l.end(); it++)
					{
						if (*it == name)
						{
							l.erase(it);
							break;
						}
					}
				}
				last_node->node.erase(name);
			}
			Value& get_value()
			{
				if (is_node)
					throw Error(CZH_ERROR_LOCATION, __func__, "Node is not Value.");
				return value;
			}
			template <typename T>
			void add(const std::string& name, const T& _value, const std::string& before = "")
			{
				if (!is_node)
					throw Error(CZH_ERROR_LOCATION, __func__, "Can not add Value to Value");
				node[name] = Node(this, name, Value(_value));
				if (outputable)
				{
					if (before == "")
						output_list->emplace_back(name);
					else
					{
						bool added = false;
						for (auto it = output_list->begin(); it < output_list->end(); it++)
						{
							if (*it == before)
							{
								output_list->insert(it, name);
								added = true;
								break;
							}
						}
						if (!added)
						{
							throw Error(CZH_ERROR_LOCATION, __func__, "There is no Node named '"
								+ before + "'.");
						}
					}
				}
			}

			Node* add_node(const std::string& name)
			{
				if (!is_node)
					throw Error(CZH_ERROR_LOCATION, __func__, "Can not add a Node to Value");
				node[name] = Node(this, name);
				if (outputable)
					output_list->emplace_back(name);
				return &node[name];
			}

			template <typename T>
			std::unique_ptr<std::map<std::string, T>> value_map()
			{
				if (!is_node)
					throw Error(CZH_ERROR_LOCATION, __func__, "Can not get a map from a Value.");
				std::unique_ptr<std::map<std::string, T>> result =
					std::make_unique<std::map<std::string, T>>();

				std::type_index value_type(typeid(T));
				std::type_index node_type(typeid(Node));
				std::type_index note_type(typeid(value::Note));
				for (auto& r : node)
				{
          if(r.second.type() == note_type)
            continue;
					else if (r.second.type() != value_type)
						throw Error(CZH_ERROR_LOCATION, __func__, "Type is not same.", Error::internal);
					else if (r.second.type() == node_type)
						throw Error(CZH_ERROR_LOCATION, __func__, "Type is Node.", Error::internal);
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
			Node& operator[](const std::string& s)
			{
				if (!is_node)
					throw Error(CZH_ERROR_LOCATION, __func__, "Value can not []", Error::internal);
				if (node.find(s) == node.end())
					throw Error(CZH_ERROR_LOCATION, __func__, "There is no node named '" + s + "'.");
				return node[s];
			}
			const Node& operator[](const std::string& s) const
			{
				if (!is_node)
					throw Error(CZH_ERROR_LOCATION, __func__, "Value can not []", Error::internal);
				if (node.find(s) == node.end())
					throw Error(CZH_ERROR_LOCATION, __func__, "There is no node named '" + s + "'.");
				return node.at(s);
			}
      auto type() const
      {
        if (is_node)
          throw Error(CZH_ERROR_LOCATION, __func__, "Node not get type.", Error::internal);
        return value.type();
      }
			template <typename T>
			T get() const
			{
				if (is_node)
					throw Error(CZH_ERROR_LOCATION, __func__, "Can not get value from a Node.", Error::internal);
        if(type() == typeid(Node*))
        {
          return value.get<Node*>()->get<T>();
        }
        return value.get<T>();
			}
			std::unique_ptr<std::vector<std::string>> get_path() const
			{
				auto n_ptr = to_last_node();
				std::vector<std::string> res;
				res.push_back(name);
				while (n_ptr != nullptr)
				{
					if (n_ptr->name != "/")
						res.push_back(n_ptr->name);
					n_ptr = n_ptr->to_last_node();
				}
				return std::move(std::make_unique<decltype(res)>(res));
			}

			bool has_node(const std::string& name) const
			{
				if (!is_node)
          throw Error(CZH_ERROR_LOCATION, __func__, "Value has no node.", Error::internal);
				return (node.find(name) != node.end());
			}
			std::string to_string(std::size_t i = 0) const
			{
				if (!outputable)
				{
					throw Error(CZH_ERROR_LOCATION, __func__,
						"Node is not outputable.", Error::internal);
				}
				std::string ret;
				if (is_node && name != "/")
					ret += std::string(i * 2, ' ') + name + ":" + "\n";
				for (auto& r : *output_list)
				{
					if (node.at(r).is_node)
					{
						if (name != "/")
							ret += node.at(r).to_string(i + 1);
						else
							ret += node.at(r).to_string(i);
					}
					else
					{
            if(node.at(r).type() != typeid(value::Note))
            {
              ret += std::string((i + 1) * 2, ' ') + node.at(r).name + " = "
							+ value_to_string(r, node.at(r).value) + ";" + "\n";
            }
            else
            {
              if(!ret.empty() && *ret.crbegin() == '\n')
                ret.pop_back();//eat '\n'
              ret += "/b/" + node.at(r).value.get<value::Note>().note + "/e/\n";
            }
					}
				}
				if (is_node && name != "/")
					ret += std::string(i * 2, ' ') + "end;\n";
				return ret;
			}
		private:
			std::string value_to_string(const std::string& name, const Value& value) const
			{
				auto t = value.type();
				if (t == typeid(int))
					return std::to_string(value.get<int>());
				else if (t == typeid(std::string))
					return ("\"" + value.get<std::string>() + "\"");
				else if (t == typeid(double))
					return std::to_string(value.get<double>());
        else if (t == typeid(bool))
          return std::to_string(value.get<bool>());
				else if (t == typeid(std::vector<int>))
					return vector_to_string(value.get<std::vector<int>>());
				else if (t == typeid(std::vector<std::string>))
					return vector_to_string(value.get<std::vector<std::string>>());
				else if (t == typeid(std::vector<double>))
					return vector_to_string(value.get<std::vector<double>>());
        else if (t == typeid(std::vector<bool>))
          return vector_to_string(value.get<std::vector<bool>>());

				//Node*
				if (t != typeid(Node*))
					throw Error(CZH_ERROR_LOCATION, __func__, "Unexpected error", Error::internal);

				std::string res;
				auto path = *value.get<Node*>()->get_path();
				auto this_path = *get_path();
        std::reverse(path.begin(), path.end());
        std::reverse(this_path.begin(), this_path.end());
        std::size_t samepos = 0;
        for (auto i = 0;i<std::min(path.size(),this_path.size()); i++)
        {
          if(path[i] == this_path[i])
          {
            samepos++;
            if(i == this_path.size())
            {
              res += "-.";
              break;
            }
          }
          else
          {
            if(i == this_path.size() - 1)
            {
              res += "-..";
              break;
            }
            samepos = 0;
            break;
          }
        }
        
        for (auto it = path.cbegin() + samepos; it < path.cend() - 1; it++)
				{
					res += "-";
          res += *it;
				}
				res += ":";
				res += *path.crbegin();
				return res;
			}
		};
    std::ostream& operator<<(std::ostream& os, const Node& node)
		{
			os << node.to_string();
			return os;
		}
	}
}