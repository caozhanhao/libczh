//   Copyright 2021-2023 libczh - caozhanhao
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
#ifndef LIBCZH_NODE_HPP
#define LIBCZH_NODE_HPP

#include "value.hpp"
#include "error.hpp"
#include "utils.hpp"
#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <memory>
#include <variant>

using czh::value::Value;
namespace czh::node
{
  using Color = utils::Color;
  
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
      
      [[nodiscard]]const auto &get_nodes() const { return nodes; }
      
      auto &get_nodes() { return nodes; }
      
      Node &add(Node node, const std::string &before, int &err)
      {
        NodeType::iterator inserted;
        if (!before.empty())
        {
          auto it = std::find_if(nodes.begin(), nodes.end(), [&before](auto &&n) { return n.get_name() == before; });
          if (it == nodes.end())
          {
            err = -1;
            return *node.rbegin();
          }
          inserted = nodes.insert(it, std::move(node));
        }
        else
        {
          inserted = nodes.insert(nodes.end(), std::move(node));
        }
        // rbegin() -> end()
        index[inserted->name] = inserted;
        return *inserted;
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
        : name(std::move(node_name)), last_node(node_ptr) { data.emplace<NodeData>(); }
    
    Node(Node *node_ptr, std::string node_name, Value val)
        : name(std::move(node_name)), last_node(node_ptr), data(std::move(val)) {}
    
    Node() : name("/"), last_node(nullptr) { data.emplace<NodeData>(); }
  
    Node(const Node &node) : name(node.name), last_node(node.last_node), data(node.data) {}
    
    Node(Node &&) = default;
    
    // Node and Value
    [[nodiscard]]bool is_node() const
    {
      return data.index() == 0;
    }
    
    [[nodiscard]]std::string get_name() const
    {
      return name;
    }
    
    Node &remove()
    {
      error::czh_assert(last_node, "Can not remove root.");
      auto &nd = std::get<NodeData>(last_node->data);
      nd.erase(name);
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
      error::czh_assert(nd.index.find(newname) == nd.index.end(), "Node and Value names are not repeatable.");
      nd.rename(name, newname);
      return *this;
    }
    
    [[nodiscard]] Node *to_last_node() const
    {
      return last_node;
    }
    
    [[nodiscard]] std::unique_ptr<std::vector<std::string>> get_path() const
    {
      auto n_ptr = to_last_node();
      std::vector<std::string> res;
      res.push_back(name);
      while (n_ptr != nullptr)
      {
        if (n_ptr->name != "/")
        {
          res.push_back(n_ptr->name);
        }
        n_ptr = n_ptr->to_last_node();
      }
      return std::move(std::make_unique<decltype(res)>(res));
    }
    
    // Node
    [[nodiscard]] bool has_node(const std::string &tag) const
    {
      assert_node();
      auto &nd = std::get<NodeData>(data);
      return (nd.find(tag) != nd.end());
    }
    
    [[nodiscard]]iterator begin()
    {
      assert_node();
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.begin();
    }
    
    [[nodiscard]]iterator end()
    {
      assert_node();
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.end();
    }
    
    [[nodiscard]]reverse_iterator rbegin()
    {
      assert_node();
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.rbegin();
    }
    
    [[nodiscard]]reverse_iterator rend()
    {
      assert_node();
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.rend();
    }
    
    [[nodiscard]]const_iterator cbegin() const
    {
      assert_node();
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.cbegin();
    }
    
    [[nodiscard]]const_iterator cend() const
    {
      assert_node();
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.cend();
    }
    
    [[nodiscard]]const_reverse_iterator crbegin() const
    {
      assert_node();
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.crbegin();
    }
    
    [[nodiscard]]const_reverse_iterator crend() const
    {
      assert_node();
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.crend();
    }
    
    Node &clear()
    {
      assert_node();
      auto &nd = std::get<NodeData>(data);
      nd.clear();
      return *this;
    }
    
    template<typename T, typename = std::enable_if_t<!std::is_base_of_v<Node, std::decay_t<T>>>>
    Node &add(std::string add_name, T &&_value, const std::string &before = "")
    {
      assert_node();
      auto &nd = std::get<NodeData>(data);
      int err = 0;
      auto &ret = nd.add(Node(this, std::move(add_name), Value(std::forward<T>(_value))), before, err);
      error::czh_assert(err == 0,
                        "There is no node named '" + before + "'.Do you mean '" + error_correct(before) + "'?");
      return ret;
    }
    
    Node &add(std::string add_name, Node &node_, std::string before = "")
    {
      return add(std::move(add_name), &node_, std::move(before));
    }
    
    Node &add_node(std::string add_name, const std::string &before = "")
    {
      assert_node();
      auto &nd = std::get<NodeData>(data);
      int err = 0;
      auto &ret = nd.add(Node(this, std::move(add_name)), before, err);
      error::czh_assert(err == 0,
                        "There is no node named '" + before + "'.Do you mean '" + error_correct(before) + "'?");
      return ret;
    }
    
    template<typename T>
    std::map<std::string, T> value_map()
    {
      assert_node();
      std::map<std::string, T> result;
      auto &nd = std::get<NodeData>(data);
      for (auto &r: nd.get_nodes())
      {
        auto pval = std::get_if<value::Value>(&r.data);
        error::czh_assert(pval, "This Node must only contain value.");
        result[r.name] = pval->get<T>();
      }
      return result;
    }
    
    Node &operator[](const std::string &s)
    {
      assert_node();
      auto &nd = std::get<NodeData>(data);
      NodeData::IndexType::iterator it = nd.find(s);
      error::czh_assert(it != nd.end(), "There is no node named '" + s + "'.Do you mean '" + error_correct(s) + "'?");
      return *it->second;
    }
    
    const Node &operator[](const std::string &s) const
    {
      assert_node();
      auto &nd = std::get<NodeData>(data);
      NodeData::IndexType::const_iterator it = nd.find(s);
      error::czh_assert(it != nd.end(), "There is no node named '" + s + "'.Do you mean '" + error_correct(s) + "'?");
      return *it->second;
    }
    
    //Value
    
    [[nodiscard]] std::string to_string
        (Color with_color = Color::no_color, std::size_t indentation = 2, int n = -1) const
    {
      if (is_node())
      {
        if (name == "/")
        {
          return node_to_string(with_color, indentation, n);
        }
        else
        {
          return node_to_string(with_color, indentation, n + 1);
        }
      }
      return value_to_string(with_color, indentation, n + 1);
    }
    
    template<typename T>
    [[nodiscard]]bool is() const
    {
      assert_value();
      auto &val = std::get<Value>(data);
      return val.is<T>();
    }
    
    template<typename T>
    Node &operator=(T &&v)
    {
      assert_value();
      auto &value = std::get<Value>(data);
      if (value.is<Node *>())
      {
        *value.get<Node *>() = std::forward<T>(v);
      }
      else
      {
        value = std::forward<T>(v);
      }
      return *this;
    }
    
    template<typename T>
    Node &operator=(std::initializer_list<T> &&il)
    {
      assert_value();
      auto &value = std::get<Value>(data);
      value = std::forward<std::initializer_list<T>>(il);
      return *this;
    }
    
    Node &operator=(value::Array &&v)
    {
      assert_value();
      auto &value = std::get<Value>(data);
      value = std::forward<value::Array>(v);
      return *this;
    }
    
    Value &get_value()
    {
      assert_value();
      return std::get<Value>(data);
    }
    
    template<typename T>
    T get() const
    {
      assert_value();
      auto &value = std::get<Value>(data);
      if (value.is<Node *>())
      {
        return value.get<Node *>()->get<T>();
      }
      return value.get<T>();
    }


  private:
    void assert_node() const
    {
      error::czh_assert(is_node(), "This Node is not a node.");
    }
  
    void assert_value() const
    {
      error::czh_assert(!is_node(), "This Node is not a value.");
    }
  
    [[nodiscard]]std::string node_to_string(Color with_color, std::size_t indentation, int n) const
    {
      auto &nd = std::get<NodeData>(data);
      std::string ret;
      if (name != "/")
      {
        ret += std::string(indentation * n, ' ')
               + colorify(name, with_color, utils::ColorType::BLOCK_BEG) //node name
               + ":" + "\n";
      }
      for (auto &node: nd.get_nodes())
        ret += node.to_string(with_color, indentation, n);
      if (name != "/")
      {
        ret += std::string(indentation * n, ' ')
               + colorify("end", with_color, utils::ColorType::BLOCK_END)
               + "\n";
      }
      return ret;
    }
    
    [[nodiscard]]std::string value_to_string(Color with_color, std::size_t indentation, int n) const
    {
      auto &value = std::get<Value>(data);
      std::string valuestr = std::visit(
          utils::overloaded{
              [&with_color](auto &&i) -> std::string { return czh::utils::to_czhstr(i, with_color); },
              [&with_color, this](Node *n) -> std::string
              {
                std::string res;
                auto path = *n->get_path();
                auto this_path = *to_last_node()->get_path();
                auto[itpath, itthis] = std::mismatch(path.rbegin(), path.rend(),
                                                     this_path.rbegin(), this_path.rend());
                if (itpath == path.rend() && itthis == this_path.rend()) res += "::";
                for (; itpath < path.rend() - 1; ++itpath)
                {
                  res += colorify(*itpath, with_color, utils::ColorType::REF_ID) + "::";
                }
                res += colorify(path[0], with_color, utils::ColorType::REF_ID);
                return res;
              }
          },
          value.get_variant());
      std::string ret = std::string(indentation * n, ' ') +
                        colorify(name, with_color, utils::ColorType::ID)// id name
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
#endif