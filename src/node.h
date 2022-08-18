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

#include "value.h"
#include "err.h"
#include "utils.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <variant>

using czh::value::Value;
using czh::error::Error;
namespace czh::node
{
  enum class Color
  {
    with_color,
    no_color
  };
  enum class Type
  {
    ID, REF_ID, NUM, STR, BOOL, BLOCK_BEG, BLOCK_END
  };
  enum class CzhColor
  {
    BLUE, LIGHT_BLUE, GREEN, PURPLE, YELLOW, WHITE, RED
  };
  
  static CzhColor get_color(Type a)
  {
    static const std::map<Type, CzhColor> colors =
        {
            {Type::ID,        CzhColor::PURPLE},
            {Type::REF_ID,    CzhColor::LIGHT_BLUE},
            {Type::NUM,       CzhColor::BLUE},
            {Type::STR,       CzhColor::GREEN},
            {Type::BOOL,      CzhColor::BLUE},
            {Type::BLOCK_BEG, CzhColor::LIGHT_BLUE},
            {Type::BLOCK_END, CzhColor::LIGHT_BLUE},
        };
    return colors.at(a);
  }
  
  std::string colorify(const std::string &str, Color with_color, Type type)
  {
    if (with_color == Color::no_color)
      return str;
    switch (get_color(type))
    {
      case CzhColor::PURPLE:
        return "\033[35m" + str + "\033[0m";
      case CzhColor::LIGHT_BLUE:
        return "\033[36m" + str + "\033[0m";
      case CzhColor::BLUE:
        return "\033[34m" + str + "\033[0m";
      case CzhColor::GREEN:
        return "\033[32m" + str + "\033[0m";
      case CzhColor::YELLOW:
        return "\033[33m" + str + "\033[0m";
      case CzhColor::WHITE:
        return "\033[37m" + str + "\033[0m";
      case CzhColor::RED:
        return "\033[31m" + str + "\033[0m";
      default:
        throw error::Error(LIBCZH_ERROR_LOCATION, __func__, "Unexpected color");
    }
  }
  
  template<typename T>
  std::string to_czhstr(const T &val, Color color)
  {
    return colorify(utils::value_to_str(val), color, Type::NUM);
  }
  
  template<>
  std::string to_czhstr(const bool &val, Color color)
  {
    return colorify((val ? "true" : "false"), color, Type::BOOL);
  }
  
  template<>
  std::string to_czhstr(const std::string &val, Color color)
  {
    return colorify(("\"" + val + "\""), color, Type::STR);
  }
  
  template<typename Ty>
  std::string to_czhstr(const std::vector<Ty> &v, Color color)
  {
    std::string result = "{";
    for (auto it = v.cbegin(); it != (v.cend() - 1); ++it)
    {
      result += to_czhstr(*it, color);
      result += ", ";
    }
    result += to_czhstr(*(v.cend() - 1), color);
    result += "}";
    return result;
  }
  
  template<>
  std::string to_czhstr(const value::Value::AnyArray &v, Color color)
  {
    auto visitor = [&color](auto &&v) -> std::string
    { return to_czhstr(v, color); };
    std::string result = "{";
    for (auto it = v.cbegin(); it != (v.cend() - 1); ++it)
    {
      result += std::visit(visitor, *it);
      result += ", ";
    }
    result += std::visit(visitor, *(v.cend() - 1));
    result += "}";
    return result;
  }
  
  class Node
  {
    friend std::ostream &operator<<(std::ostream &, const Node &);
  
  private:
    class NodeData
    {
    public:
      using IndexType = std::map<std::string, std::list<Node>::iterator>;
      using NodeType = std::list<Node>;
      IndexType index;
      std::list<Node> nodes;
    public:
      NodeData() = default;
      
      [[nodiscard]]const auto &get_nodes() const
      { return nodes; }
      
      auto &get_nodes()
      { return nodes; }
      
      Node &add(Node node)
      {
        nodes.emplace_back(std::move(node));
        auto a = nodes.rbegin();
        // rbegin() -> end()
        index[nodes.rbegin()->name] = std::next(nodes.rbegin()).base();
        return *nodes.rbegin();
      }
      
      void erase(const std::string &tag)
      {
        auto it = index.find(tag);
        nodes.erase(it->second);
        index.erase(it);
      }
      
      void clear()
      {
        nodes.clear();
        index.clear();
      }
      
      void rename(const std::string &oldname, const std::string &newname)
      {
        auto n = index.extract(oldname);
        n.key() = newname;
        index.insert(std::move(n));
        index[newname]->name = newname;
      }
      
      [[nodiscard]]NodeData::IndexType::iterator find(const std::string &str)
      {
        return index.find(str);
      }
      
      [[nodiscard]]auto end()
      {
        return index.end();
      }
      
      [[nodiscard]]NodeData::IndexType::const_iterator find(const std::string &str) const
      {
        return index.find(str);
      }
  
      [[nodiscard]]auto end() const
      {
        return index.end();
      }
    };

  public:
    using iterator = NodeData::NodeType::iterator;
    using const_iterator = NodeData::NodeType::const_iterator;
    using reverse_iterator = NodeData::NodeType::reverse_iterator;
    using const_reverse_iterator = NodeData::NodeType::const_reverse_iterator;
  private:
    std::string name;
    Node *last_node;
    std::variant<NodeData, Value> data;
  public:
    Node(Node *node_ptr, std::string node_name)
        : name(std::move(node_name)), last_node(node_ptr)
    { data.emplace<NodeData>(); }
  
    Node(Node *node_ptr, std::string node_name, Value val)
        : name(std::move(node_name)), last_node(node_ptr), data(val)
    {}
    
    Node() : name("/"), last_node(nullptr)
    { data.emplace<NodeData>(); }
  
    Node(const Node &) = delete;
  
    Node(Node &&) = default;
  
    [[nodiscard]]bool is_node() const
    {
      return data.index() == 0;
    }
  
    [[nodiscard]]iterator begin()
    {
      if (!is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Value has no iterator.");
      }
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.begin();
    }
  
    [[nodiscard]]iterator end()
    {
      if (!is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Value has no iterator.");
      }
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.end();
    }
  
    [[nodiscard]]reverse_iterator rbegin()
    {
      if (!is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Value has no reverse_iterator.");
      }
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.rbegin();
    }
  
    [[nodiscard]]reverse_iterator rend()
    {
      if (!is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Value has no reverse_iterator.");
      }
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.rend();
    }
  
    [[nodiscard]]const_iterator cbegin() const
    {
      if (!is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Value has no const_iterator.");
      }
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.cbegin();
    }
  
    [[nodiscard]]const_iterator cend() const
    {
      if (!is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Value has no const_iterator.");
      }
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.cend();
    }
  
    [[nodiscard]]const_reverse_iterator crbegin() const
    {
      if (!is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Value has no const_reverse_iterator.");
      }
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.crbegin();
    }
  
    [[nodiscard]]const_reverse_iterator crend() const
    {
      if (!is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Value has no const_reverse_iterator.");
      }
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.crend();
    }
  
    std::string get_name() const
    {
      return name;
    }
  
    Node &remove()
    {
      if (last_node == nullptr)
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Can not remove root.");
      }
      auto &nd = std::get<NodeData>(last_node->data);
      nd.erase(name);
      return *this;
    }
  
    Node &clear()
    {
      if (!is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Value can not clear.");
      }
      auto &nd = std::get<NodeData>(data);
      nd.clear();
      return *this;
    }
    
    Node &rename(const std::string &newname)
    {
      if (last_node == nullptr)
      {
        name = newname;
        return *this;
      }
      auto &nd = std::get<NodeData>(last_node->data);
      nd.rename(name, newname);
      return *this;
    }
    
    Value make_ref()
    {
      if (is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Can not make a reference to a Node.");
      }
      return value::Value(this);
    }
    
    template<typename T>
    Node &add(const std::string &add_name, const T &_value, const std::string &before = "")
    {
      if (!is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Can not add Value to Value");
      }
      auto &nd = std::get<NodeData>(data);
      return nd.add(Node(this, add_name, Value(_value)));
    }
    
    Node &add_node(const std::string &add_name, const std::string &before = "")
    {
      if (!is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Can not add a Node to Value");
      }
      auto &nd = std::get<NodeData>(data);
      return nd.add(Node(this, add_name));
    }
    
    [[nodiscard]] auto type() const
    {
      if (is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Node not get type.");
      }
      return std::get<Value>(data).type();
    }
    
    template<typename T>
    std::unique_ptr<std::map<std::string, T>> value_map()
    {
      if (!is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Can not get a map from a Value.");
      }
      std::unique_ptr<std::map<std::string, T>> result =
          std::make_unique<std::map<std::string, T>>();
      
      auto &nd = std::get<NodeData>(data);
      std::type_index value_type(typeid(T));
      std::type_index node_type(typeid(Node));
      for (auto &r: nd.get_nodes())
      {
        if (r.type() != value_type)
        {
          throw Error(LIBCZH_ERROR_LOCATION, __func__, "TokenType is not same.");
        }
        else if (r.type() == node_type)
        {
          throw Error(LIBCZH_ERROR_LOCATION, __func__, "TokenType is Node.");
        }
        else
          (*result)[r.name] = r.get<T>();
      }
      return result;
    }
    
    [[nodiscard]] Node *to_last_node() const
    {
      return last_node;
    }
    
    Node &operator[](const std::string &s)
    {
      if (!is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Value can not []");
      }
      auto &nd = std::get<NodeData>(data);
      NodeData::IndexType::iterator it = nd.find(s);
      if (it == nd.end())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__,
                    "There is no node named '" + s + "'.Do you mean '" + error_correct(s) + "'?");
      }
      return *it->second;
    }
    
    const Node &operator[](const std::string &s) const
    {
      if (!is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Value can not []");
      }
      auto &nd = std::get<NodeData>(data);
      NodeData::IndexType::const_iterator it = nd.find(s);
      if (it == nd.end())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__,
                    "There is no node named '" + s + "'.Do you mean '" + error_correct(s) + "'?");
      }
      return *it->second;
    }
    
    template<typename T>
    Node &operator=(T &&v)
    {
      if (is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "This Node does not contain not Value.");
      }
      auto &value = std::get<Value>(data);
      if (type() == typeid(Node *))
      {
        *value.get<Node *>() = std::forward<T>(v);
      }
      else
        value = std::forward<T>(v);
      return *this;
    }
    
    template<typename T>
    const T &get() const
    {
      if (is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Can not get value from a Node.");
      }
      auto &value = std::get<Value>(data);
      if (type() == typeid(Node *))
      {
        return value.get<Node *>()->get<T>();
      }
      return value.get<T>();
    }
    
    [[nodiscard]] std::unique_ptr<std::vector<std::string>> get_path() const
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
    
    [[nodiscard]] bool has_node(const std::string &tag) const
    {
      if (!is_node())
      {
        throw Error(LIBCZH_ERROR_LOCATION, __func__, "Value has no node.");
      }
      auto &nd = std::get<NodeData>(data);
      return (nd.find(tag) != nd.end());
    }
    
    [[nodiscard]] std::string
    to_string(Color with_color = Color::no_color, std::size_t indentation = 2, int n = -1) const
    {
      if (is_node())
      {
        if (name == "/")
          return node_to_string(with_color, indentation, n);
        else
          return node_to_string(with_color, indentation, n + 1);
      }
      return value_to_string(with_color, indentation, n + 1);
    }
  
  private:
    [[nodiscard]]std::string node_to_string(Color with_color, std::size_t indentation, int n) const
    {
      auto &nd = std::get<NodeData>(data);
      std::string ret;
      if (name != "/")
      {
        ret += std::string(indentation * n, ' ')
               + colorify(name, with_color, Type::BLOCK_BEG) //node name
               + ":" + "\n";
      }
      for (auto &node: nd.get_nodes())
        ret += node.to_string(with_color, indentation, n);
      if (name != "/")
      {
        ret += std::string(indentation * n, ' ')
               + colorify("end", with_color, Type::BLOCK_END)
               + "\n";
      }
      return ret;
    }
    
    [[nodiscard]]std::string value_to_string(Color with_color, std::size_t indentation, int n) const
    {
      auto &value = std::get<Value>(data);
      std::string valuestr = std::visit(
          utils::overloaded{
              [&with_color](auto &&i) -> std::string
              { return czh::node::to_czhstr(i, with_color); },
              [&with_color, this](Node *n) -> std::string
              {
                std::string res;
                auto path = *n->get_path();
                auto this_path = *to_last_node()->get_path();
                auto[itpath, itthis] = std::mismatch(path.rbegin(), path.rend(),
                                                     this_path.rbegin(), this_path.rend());
                if (itpath == path.rend() && itthis == this_path.rend()) res += "::";
                for (; itpath < path.rend() - 1; ++itpath)
                  res += colorify(*itpath, with_color, Type::REF_ID) + "::";
                res += colorify(path[0], with_color, Type::REF_ID);
                return res;
              }
          },
          value.get_variant());
      std::string ret = std::string(indentation * n, ' ') +
                        colorify(name, with_color, Type::ID)// id name
                        + " = " + valuestr + "\n";
      return ret;
    }
    
    [[nodiscard]] std::string error_correct(const std::string &str) const
    {
      auto &nd = std::get<NodeData>(data);
      if (nd.get_nodes().empty()) return "";
      auto it = std::min_element(nd.get_nodes().cbegin(), nd.get_nodes().cend(),
                                 [&str](auto &&n1, auto &&n2) -> bool
                                 {
                                   return czh::utils::get_string_edit_distance(n1.name, str)
                                          < czh::utils::get_string_edit_distance(n2.name, str);
                                 });
      return it->name;
    }
    
  };
  
  std::ostream &operator<<(std::ostream &os, const Node &node)
  {
    os << node.to_string();
    return os;
  }
}