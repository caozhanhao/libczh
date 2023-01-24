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
#pragma once

#include "value.hpp"
#include "writer.hpp"
#include "error.hpp"
#include "utils.hpp"
#include "token.hpp"
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <memory>
#include <variant>
#include <typeindex>
#include <typeinfo>

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
      
      template<typename T>
      NodeData(const T &il)
      {
        for (auto &r: il)
        {
          int e;
          add(Node(r), "", e);
        }
      }
      
      [[nodiscard]]const auto &get_nodes() const { return nodes; }
      
      auto &get_nodes() { return nodes; }
      
      bool operator==(const NodeData &nd) const
      {
        return (nodes == nd.nodes);
      }
      
      Node *add(Node node, const std::string &before, int &err)
      {
        NodeType::iterator inserted;
        if (!before.empty())
        {
          auto it = std::find_if(nodes.begin(), nodes.end(), [&before](auto &&n) { return n.get_name() == before; });
          if (it == nodes.end())
          {
            err = -1;
            return nullptr;
          }
          inserted = nodes.insert(it, std::move(node));
        }
        else
        {
          inserted = nodes.insert(nodes.end(), std::move(node));
        }
        // rbegin() -> end()
        index[inserted->name] = inserted;
        err = 0;
        return &*inserted;
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
    token::Token czh_token;
  public:
    Node(Node *node_ptr, std::string node_name, token::Token token)
        : name(std::move(node_name)), last_node(node_ptr), czh_token(std::move(token)) { data.emplace<NodeData>(); }
  
    Node(Node *node_ptr, std::string node_name, Value val, token::Token token)
        : name(std::move(node_name)), last_node(node_ptr), data(std::move(val)), czh_token(std::move(token)) {}
  
    Node() : name(""), last_node(nullptr) { data.emplace<NodeData>(); }
  
    explicit Node(const Node &node) : name(node.name), last_node(node.last_node), czh_token(node.czh_token),
                                      data(node.data)
    {
      if (is_node())
      {
        auto &nd = std::get<NodeData>(data);
        for (auto &r: nd.nodes)
        {
          r.last_node = this;
        }
      }
    }
  
    Node &operator=(const Node &v)
    {
      name = v.name;
      last_node = v.last_node;
      czh_token = token::Token(v.czh_token);
      if (v.is_node())
      {
        data = NodeData(std::get<NodeData>(v.data));
        auto &nd = std::get<NodeData>(data);
        for (auto &r: nd.nodes)
        {
          r.last_node = this;
        }
      }
      else
      {
        data = Value(std::get<Value>(v.data));
      }
      return *this;
    }
  
    Node(Node &&node)
        : name(std::move(node.name)), last_node(std::move(node.last_node)), czh_token(std::move(node.czh_token)),
          data(std::move(node.data))
    {
      if (is_node())
      {
        auto &nd = std::get<NodeData>(data);
        for (auto &r: nd.nodes)
        {
          r.last_node = this;
        }
      }
    }
  
    template<typename T>
    requires (!std::is_base_of_v<Node, std::decay_t<T>>)
    Node(std::string name_, T &&v): name(std::move(name_)), last_node(nullptr)
    {
      data.emplace<Value>(v);
    }
  
    Node(std::string name_, std::initializer_list<Node> v) : name(std::move(name_)), last_node(nullptr)
    {
      data.emplace<NodeData>(v);
      auto &nd = std::get<NodeData>(data);
      for (auto &r: nd.nodes)
      {
        r.last_node = this;
      }
    }
  
    Node(std::initializer_list<Node> v) : last_node(nullptr)
    {
      data.emplace<NodeData>(v);
      auto &nd = std::get<NodeData>(data);
      for (auto &r: nd.nodes)
      {
        r.last_node = this;
      }
    }
  
    // Node and Value
    Node &reset()
    {
      data.emplace<NodeData>();
      last_node = nullptr;
      name = "";
      return *this;
    }
  
    [[nodiscard]]bool is_node() const
    {
      return data.index() == 0;
    }
  
    bool operator==(const Node &n) const
    {
      if (name != n.name) return false;
      if (!is_node())
      {
        auto &v = std::get<Value>(data);
        if (v.is<value::Reference>())
        {
          if (!n.is<value::Reference>()) return false;
          return get_ref(v.get<value::Reference>())->get_path() == n.get_last_node()->get_ref(n.get<value::Reference>())->get_path();
        }
      }
      return data == n.data;
    }
  
    [[nodiscard]]std::string get_name() const
    {
      return name;
    }
  
    Node &remove(const std::experimental::source_location &l =
    std::experimental::source_location::current())
    {
      assert_true(last_node, "Can not remove root.", czh_token, l);
      auto &nd = std::get<NodeData>(last_node->data);
      nd.erase(name);
      return *this;
    }
  
    Node &rename(const std::string &newname, const std::experimental::source_location &l =
    std::experimental::source_location::current())
    {
      if (last_node == nullptr)
      {
        name = newname;
        return *this;
      }
      auto &nd = std::get<NodeData>(last_node->data);
      auto it = nd.index.find(newname);
      assert_true(it == nd.index.end(), "Duplicate node name.", it->second->czh_token, l);
      nd.rename(name, newname);
      return *this;
    }
  
    [[nodiscard]] Node *get_last_node() const
    {
      return last_node;
    }
    
    [[nodiscard]] std::vector<std::string> get_path() const
    {
      std::vector<std::string> res;
      res.push_back(name);
      for (auto n_ptr = get_last_node(); n_ptr != nullptr; n_ptr = n_ptr->get_last_node())
      {
        auto n = n_ptr->name;
        if (n.empty()) break;// root
        res.emplace_back(n);
      }
      return res;
    }
  
    template<writer::Writer W>
    const Node &accept(W &writer) const
    {
      if (is_node())
      {
        if (!name.empty())
        {
          writer.node_begin(name);
        }
        auto &nd = std::get<NodeData>(data);
        for (auto &r: nd.nodes)
        {
          r.accept(writer);
        }
        if (!name.empty())
        {
          writer.node_end();
        }
      }
      else
      {
        auto &value = std::get<Value>(data);
        writer.value_begin(name);
        if (value.is<value::Reference>())
        {
          auto path = get_ref(value.get<value::Reference>())->get_path();
          auto this_path = get_last_node()->get_path();
          auto[itpath, itthis] = std::mismatch(path.rbegin(), path.rend(),
                                               this_path.rbegin(), this_path.rend());
          if (itpath == path.rend() && itthis == this_path.rend())
          {
            writer.value_ref_path_set_global();
          }
          for (; itpath < path.rend() - 1; ++itpath)
          {
            writer.value_ref_path(*itpath);
          }
          writer.value_ref_id(path[0]);
        }
        else if (value.is<value::Array>())
        {
          auto arr = value.get<value::Array>();
          writer.value_array_begin();
          for (auto it = arr.cbegin(); it < arr.cend() - 1; ++it)
          {
            writer.value_array_value(*it);
          }
          if (!arr.empty())
          {
            writer.value_array_end(*arr.crbegin());
          }
        }
        else
        {
          writer.value(value);
        }
      }
      return *this;
    }
  
    template<typename T>
    requires (!std::is_base_of_v<Node, std::decay_t<T>>)
    Node &operator=(T &&v)
    {
      if (is_node()) data.template emplace<Value>();
      auto &value = std::get<Value>(data);
      if (value.is<value::Reference>())
      {
	auto ptr = get_end_of_list_of_ref(value.get<value::Reference>());
	assert_true(ptr != nullptr, "Can not get a circular reference.", czh_token);
        *ptr = std::forward<T>(v);
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
      if (is_node()) data.template emplace<Value>();
      auto &value = std::get<Value>(data);
      value = std::forward<std::initializer_list<T>>(il);
      return *this;
    }
  
    Node &operator=(const value::Array &v)
    {
      if (is_node()) data.template emplace<Value>();
      auto &value = std::get<Value>(data);
      value = v;
      return *this;
    }
  
  
    // Node only
    [[nodiscard]] bool has_node(const std::string &tag, const std::experimental::source_location &l =
    std::experimental::source_location::current()) const
    {
      assert_node(l);
      auto &nd = std::get<NodeData>(data);
      return (nd.find(tag) != nd.end());
    }
  
    [[nodiscard]]iterator begin(const std::experimental::source_location &l =
    std::experimental::source_location::current())
    {
      assert_node(l);
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.begin();
    }
  
    [[nodiscard]]iterator end(const std::experimental::source_location &l =
    std::experimental::source_location::current())
    {
      assert_node(l);
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.end();
    }
  
    [[nodiscard]]reverse_iterator rbegin(const std::experimental::source_location &l =
    std::experimental::source_location::current())
    {
      assert_node(l);
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.rbegin();
    }
  
    [[nodiscard]]reverse_iterator rend(const std::experimental::source_location &l =
    std::experimental::source_location::current())
    {
      assert_node(l);
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.rend();
    }
  
    [[nodiscard]]const_iterator cbegin(const std::experimental::source_location &l =
    std::experimental::source_location::current()) const
    {
      assert_node(l);
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.cbegin();
    }
  
    [[nodiscard]]const_iterator cend(const std::experimental::source_location &l =
    std::experimental::source_location::current()) const
    {
      assert_node(l);
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.cend();
    }
  
    [[nodiscard]]const_reverse_iterator crbegin(const std::experimental::source_location &l =
    std::experimental::source_location::current()) const
    {
      assert_node(l);
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.crbegin();
    }
  
    [[nodiscard]]const_reverse_iterator crend(const std::experimental::source_location &l =
    std::experimental::source_location::current()) const
    {
      assert_node(l);
      auto &nd = std::get<NodeData>(data);
      return nd.nodes.crend();
    }
  
    Node &clear(const std::experimental::source_location &l =
    std::experimental::source_location::current())
    {
      assert_node(l);
      auto &nd = std::get<NodeData>(data);
      nd.clear();
      return *this;
    }
  
    template<typename T, typename = std::enable_if_t<!std::is_base_of_v<Node, std::decay_t<T>>>>
    Node &add(std::string add_name, T &&_value, const std::string &before = "", token::Token token = token::Token(),
              const std::experimental::source_location &l =
              std::experimental::source_location::current())
    {
      assert_node(l);
      auto &nd = std::get<NodeData>(data);
      int err = 0;
      auto ret = nd.add(Node(this, std::move(add_name), Value(std::forward<T>(_value)), std::move(token)), before,
                        err);
      if (err != 0) report_no_node(before, l);
      return *ret;
    }
    
    Node &add(std::string add_name, const Node &node_, const std::string &before = "")
    {
      return add(std::move(add_name), value::Reference(node_.get_path()), before, token::Token(node_.czh_token));
    }
  
    Node &add_node(std::string add_name, const std::string &before = "", token::Token token = token::Token(),
                   const std::experimental::source_location &l =
                   std::experimental::source_location::current())
    {
      assert_node(l);
      auto &nd = std::get<NodeData>(data);
      int err = 0;
      auto ret = nd.add(Node(this, std::move(add_name), std::move(token)), before, err);
      if (err != 0) report_no_node(before, l);
      return *ret;
    }
  
    template<typename T>
    std::map<std::string, T> value_map(const std::experimental::source_location &l =
    std::experimental::source_location::current())
    {
      assert_node(l);
      std::map<std::string, T> result;
      auto &nd = std::get<NodeData>(data);
      for (auto &r: nd.get_nodes())
      {
        auto pval = std::get_if<value::Value>(&r.data);
        assert_true(pval, "This Node must only contain value.", czh_token, l);
        result[r.name] = pval->get<T>();
      }
      return result;
    }
  
    const Node &operator()(const std::string &s, const std::experimental::source_location &l =
    std::experimental::source_location::current()) const
    {
      assert_node(l);
      auto &nd = std::get<NodeData>(data);
      auto it = nd.find(s);
      if (it == nd.end()) report_no_node(s, l);
      return *it->second;
    }
  
    Node &operator()(const std::string &s, const std::experimental::source_location &l =
    std::experimental::source_location::current())
    {
      return const_cast<Node &>(const_cast<const Node &>(*this)(s));
    }
  
    Node &operator[](const std::string &s)
    {
      return operator()(s);
    }
  
    const Node &operator[](const std::string &s) const
    {
      return operator()(s);
    }
  
    //Value only
    template<typename T>
    [[nodiscard]]bool is(const std::experimental::source_location &l =
    std::experimental::source_location::current()) const
    {
      assert_value(l);
      auto &val = std::get<Value>(data);
      return val.is<T>();
    }
  
    Value &get_value(const std::experimental::source_location &l =
    std::experimental::source_location::current())
    {
      assert_value(l);
      return std::get<Value>(data);
    }
  
    template<typename T>
    T get(const std::experimental::source_location &l =
    std::experimental::source_location::current()) const
    {
      value::check_type<T>();
      assert_value(l);
      auto &value = std::get<Value>(data);
      if (value.is<value::Reference>() && typeid(T) != typeid(value::Reference))
      {
        auto ptr = get_end_of_list_of_ref(value.get<value::Reference>(), l);
	assert_true(ptr != nullptr, "Can not get a circular reference.", czh_token, l);
	return ptr->get<T>();
      }
  
      if (!value.can_get<T>())
      {
        report_error("The value is not '" + std::string(value::details::nameof<T>()) + "'.[Actual T = '"
                     + value.get_typename() + "'].", czh_token, l);
      }
      return value.get<T>();
    }


  private:
    // If there is no cycle, it returns the result of a (list of) Reference.
    Node *get_end_of_list_of_ref(const value::Reference &ref, const std::experimental::source_location &l = std::experimental::source_location::current()) const
    { 
      Node* fast = get_ref(ref, l);
      Node* slow = fast;
      while (fast->is<value::Reference>())
      {
        fast = get_ref(fast->get<value::Reference>(), l);
      	if(!fast->is<value::Reference>()) break;
        fast = get_ref(fast->get<value::Reference>(), l);
      	if(!fast->is<value::Reference>()) break;
        slow = get_ref(slow->get<value::Reference>(), l);
	if(fast == slow) return nullptr;
      }
      return fast;
    }
    Node *get_ref(const value::Reference &ref, const std::experimental::source_location &l =
    std::experimental::source_location::current()) const
    {
      Node *nptr = last_node;
      auto rit = ref.path.crbegin();
      if (is_node())
      {
        nptr = const_cast<Node *>(this);
      }
      if (rit->empty())
      {
        while (nptr->last_node != nullptr)
        {
          nptr = nptr->last_node;
        }
        rit++;
      }
    
      for (; rit < ref.path.crend(); ++rit)
      {
        if (!nptr->has_node(*rit))
        {
          assert_true(nptr->last_node != nullptr, "Unknown reference.", czh_token, l);
          return nptr->last_node->get_ref(ref, l);
        }
        else
        {
          nptr = &(*nptr)[*rit];
        }
      }
      return nptr;
    }
  
    void assert_node(const std::experimental::source_location &l) const
    {
      if (!is_node())
      {
        czh_token.report_error("This Node is not a node. Required from " + error::location_to_str(l) + ".");
      }
    }
  
    void assert_value(const std::experimental::source_location &l) const
    {
      if (is_node())
      {
        czh_token.report_error("This Node is not a value. Required from " + error::location_to_str(l) + ".");
      }
    }
  
    static void report_error(const std::string &str, const token::Token &token,
                             const std::experimental::source_location &l)
    {
      token.report_error(str + " Required from " + error::location_to_str(l) + ".");
    }
  
    static void assert_true(bool a, const std::string &str, const token::Token &token,
                            const std::experimental::source_location &l = std::experimental::source_location::current())
    {
      if (!a) report_error(str, token, l);
    }
  
    void report_no_node(const std::string &str,
                        const std::experimental::source_location &l) const
    {
      auto &nd = std::get<NodeData>(data);
      if (nd.get_nodes().empty())
      {
        report_error("There is no node named '" + str + "' in a empty node.", czh_token, l);
        return;
      }
      auto it = std::min_element(nd.get_nodes().cbegin(), nd.get_nodes().cend(),
                                 [&str](auto &&n1, auto &&n2) -> bool
                                 {
                                   return czh::utils::get_string_edit_distance(n1.name, str)
                                          < czh::utils::get_string_edit_distance(n2.name, str);
                                 });
    
      report_error("There is no node named '" + str + "'.Do you mean '" + it->name + "'?", it->czh_token, l);
    }
  
  };
  
  std::ostream &operator<<(std::ostream &os, const Node &node)
  {
    writer::BasicWriter<std::ostream> w{os};
    node.accept(w);
    return os;
  }
}
#endif
