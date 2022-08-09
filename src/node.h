#pragma once

#include "value.h"
#include "err.h"
#include "utils.h"
#include <iostream>
#include <algorithm>
#include <charconv>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>

using czh::value::Value;
using czh::error::Error;
namespace czh::node
{
  enum class Type
  {
    ID, REF_ID, NUM, STR, BOOL, BLOCK_BEG, BLOCK_END, REF_BLOCK_ID, NOTE
  };
  enum class Color
  {
    BLUE, LIGHT_BLUE, GREEN, PURPLE, YELLOW, WHITE, RED
  };
  const std::map<Type, Color> colors =
      {
          {Type::ID,           Color::PURPLE},
          {Type::REF_ID,       Color::PURPLE},
          {Type::NUM,          Color::BLUE},
          {Type::STR,          Color::GREEN},
          {Type::BOOL,         Color::BLUE},
          {Type::BLOCK_BEG,    Color::LIGHT_BLUE},
          {Type::BLOCK_END,    Color::LIGHT_BLUE},
          {Type::REF_BLOCK_ID, Color::LIGHT_BLUE},
          {Type::NOTE,         Color::YELLOW}
      };
  std::string colorify(const std::string& str, bool with_color, Type type)
  {
    if(!with_color)
      return str;
    if(colors.find(type) == colors.end())
      throw error::Error(CZH_ERROR_LOCATION, __func__, "Unexpected type");
    switch (colors.at(type))
    {
      case Color::PURPLE:
        return "\033[35m" + str + "\033[0m";
      case Color::LIGHT_BLUE:
        return "\033[36m" + str + "\033[0m";
      case Color::BLUE:
        return "\033[34m" + str + "\033[0m";
      case  Color::GREEN:
        return "\033[32m" + str + "\033[0m";
      case Color::YELLOW:
        return  "\033[33m" + str + "\033[0m";
      case Color::WHITE:
        return  "\033[37m" + str + "\033[0m";
      case Color::RED:
        return  "\033[31m" + str + "\033[0m";
      default:
        throw error::Error(CZH_ERROR_LOCATION, __func__, "Unexpected color");
    }
  }
  template <typename T>
  std::string to_czhstr(const T& val, bool color)
  {
    return colorify(utils::value_to_str(val), color, Type::NUM);
  }
  template <>
  std::string to_czhstr(const bool& val, bool color)
  {
    return colorify((val ? "true" : "false"), color, Type::BOOL);
  }
  template <>
  std::string to_czhstr(const std::string& val, bool color)
  {
    return colorify(("\"" + val + "\""), color, Type::STR);
  }
  template <>
  std::string to_czhstr(const value::Note& val, bool color)
  {
    return colorify(("/b/" + val.note + "/e/"), color, Type::NOTE);
  }
  template<typename VT>
  std::string vector_to_string(const VT &v, bool color)
  {
    std::string result = "[";
    for (auto it = v.cbegin(); it != (v.cend() - 1); ++it)
    {
      result += to_czhstr(*it, color);
      result += ", ";
    }
    result += to_czhstr(*(v.cend() - 1), color);
    result += "]";
    return result;
  }
  
  class Node
  {
    friend std::ostream &operator<<(std::ostream &, const Node &);
  
  public:
    static const bool disable_output = false;
    static const bool color = true;
  private:
    std::string name;
    Node *last_node;
    std::map<std::string, Node> node;
    Value value;
    bool outputable;
    bool is_node;
    std::shared_ptr<std::vector<std::string>> output_list;
  public:
    Node(Node *node_ptr, std::string node_name, bool outputable_ = true)
        : name(std::move(node_name)), last_node(node_ptr), outputable(outputable_), is_node(true)
    {
      if (outputable)
        output_list = std::make_shared<std::vector<std::string>>();
    }
    
    Node(Node *node_ptr, std::string node_name, Value val, bool outputable_ = true)
        : name(std::move(node_name)), last_node(node_ptr), value(std::move(val)),
          outputable(outputable_), is_node(false)
    {
      if (outputable)
        output_list = std::make_shared<std::vector<std::string>>();
    }
    
    explicit Node(bool outputable_ = true)
        : name("/"), last_node(nullptr), outputable(outputable_), is_node(true)
    {
      if (outputable)
        output_list = std::make_shared<std::vector<std::string>>();
    }
    
    [[nodiscard]] std::string get_name() const { return name; }
    
    Node& remove(const std::string &item)
    {
      if (outputable)
      {
        auto it = std::find(output_list->begin(), output_list->end(), item);
        if (it == output_list->end())
        {
          throw Error(CZH_ERROR_LOCATION, __func__, "There is no Node named '"
                                                    + item + "'.");
        }
        output_list->erase(it);
      }
      node.erase(name);
      return *this;
    }
    
    Node& clear()
    {
      node.clear();
      output_list->clear();
      return *this;
    }
  
    Node& rename(const std::string& item, const std::string& newname)
    {
      if (outputable)
      {
        auto it = std::find(output_list->begin(), output_list->end(), item);
        if (it == output_list->end())
        {
          throw Error(CZH_ERROR_LOCATION, __func__, "There is no Node named '"
                                                    + item + "'.");
        }
        *it = newname;
      }
      if (node.find(item) == node.end())
      {
        throw Error(CZH_ERROR_LOCATION, __func__, "There is no Node named '"
                                                  + item + "'.");
      }
      node[item].name = newname;
      auto n = node.extract(item);
      n.key() = newname;
      node.insert(std::move(n));
      return *this;
    }
    
    Value &get_value()
    {
      if (is_node)
        throw Error(CZH_ERROR_LOCATION, __func__, "This Node is not Value.");
      return value;
    }
    
    Value get_ref()
    {
      if (is_node)
        throw Error(CZH_ERROR_LOCATION, __func__, "Can not make a reference to a Node.");
      return value::Value(this);
    }
    template<typename T>
    Node& add(const std::string &add_name, const T &_value, const std::string &before = "")
    {
      if (!is_node)
        throw Error(CZH_ERROR_LOCATION, __func__, "Can not add Value to Value");
      node[add_name] = Node(this, add_name, Value(_value));
      if (outputable)
      {
        if (before.empty())
          output_list->emplace_back(add_name);
        else
        {
          bool added = false;
          for (auto it = output_list->begin(); it < output_list->end(); it++)
          {
            if (*it == before)
            {
              output_list->insert(it, add_name);
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
      return *this;
    }
    
    Node *add_node(const std::string &add_name)
    {
      if (!is_node)
        throw Error(CZH_ERROR_LOCATION, __func__, "Can not add a Node to Value");
      node[add_name] = Node(this, add_name);
      if (outputable)
        output_list->emplace_back(add_name);
      return &node[add_name];
    }
    
    template<typename T>
    std::unique_ptr<std::map<std::string, T>> value_map()
    {
      if (!is_node)
        throw Error(CZH_ERROR_LOCATION, __func__, "Can not view_char a map from a Value.");
      std::unique_ptr<std::map<std::string, T>> result =
          std::make_unique<std::map<std::string, T>>();
      
      std::type_index value_type(typeid(T));
      std::type_index node_type(typeid(Node));
      std::type_index note_type(typeid(value::Note));
      for (auto &r:node)
      {
        if (r.second.type() == note_type)
          continue;
        else if (r.second.type() != value_type)
          throw Error(CZH_ERROR_LOCATION, __func__, "TokenType is not same.", Error::internal);
        else if (r.second.type() == node_type)
          throw Error(CZH_ERROR_LOCATION, __func__, "TokenType is Node.", Error::internal);
        else
          (*result)[r.first] = r.second.get<T>();
      }
      return result;
    }
  
    [[nodiscard]] Node *to_last_node() const
    {
      return last_node;
    }
  
    [[nodiscard]] Node *get_root() const
    {
      auto last = to_last_node();
      for (; last->name != "/"; last = last->to_last_node());
      return last;
    }
    
    Node &operator[](const std::string &s)
    {
      if (!is_node)
        throw Error(CZH_ERROR_LOCATION, __func__, "Value can not []", Error::internal);
      if (node.find(s) == node.end())
        throw Error(CZH_ERROR_LOCATION, __func__, "There is no node named '" + s + "'.");
      return node[s];
    }
    
    const Node &operator[](const std::string &s) const
    {
      if (!is_node)
        throw Error(CZH_ERROR_LOCATION, __func__, "Value can not []", Error::internal);
      if (node.find(s) == node.end())
        throw Error(CZH_ERROR_LOCATION, __func__, "There is no node named '" + s + "'.");
      return node.at(s);
    }
  
    [[nodiscard]] auto type() const
    {
      if (is_node)
        throw Error(CZH_ERROR_LOCATION, __func__, "Node not view_char type.", Error::internal);
      return value.type();
    }
    
    template<typename T>
    T get() const
    {
      if (is_node)
        throw Error(CZH_ERROR_LOCATION, __func__, "Can not view_char value from a Node.", Error::internal);
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
  
    [[nodiscard]] auto get_outputlist() const { return output_list; }
  
    [[nodiscard]] bool has_node(const std::string &tag) const
    {
      if (!is_node)
        throw Error(CZH_ERROR_LOCATION, __func__, "Value has no node.", Error::internal);
      return (node.find(tag) != node.end());
    }
  
    [[nodiscard]] std::string to_string(bool with_color = !color, std::size_t i = 0) const
    {
      if (!outputable)
      {
        throw Error(CZH_ERROR_LOCATION, __func__,
                    "Node is not outputable.", Error::internal);
      }
      std::string ret;
      if (is_node && name != "/")
        ret += std::string(i * 2, ' ')
            + colorify(name, with_color, Type::BLOCK_BEG) //node name
            + ":" + "\n";
      for (auto &r:*output_list)
      {
        if (node.at(r).is_node)
        {
          if (name != "/")
            ret += node.at(r).to_string(with_color,i + 1);
          else
            ret += node.at(r).to_string(with_color,i);
        } else
        {
          if (node.at(r).type() != typeid(value::Note))
          {
            ret += std::string((i + 1) * 2, ' ') +
                colorify(node.at(r).name, with_color, Type::ID)// id name
                + " = " + value_to_string(r, node.at(r).value, with_color)// value
                + ";" + "\n";
          } else
          {
            if (!ret.empty() && *ret.crbegin() == '\n')
              ret.pop_back();//eat '\n'
            ret += to_czhstr(node.at(r).value.get<value::Note>(), with_color) + "\n";
          }
        }
      }
      if (is_node && name != "/")
        ret += std::string(i * 2, ' ') +
            colorify("end", with_color, Type::BLOCK_END)
            + ";\n";
      return ret;
    }
  
  private:
    [[nodiscard]] std::string value_to_string(const std::string &value_name, const Value &val, bool with_color) const
    {
      auto t = val.type();
      if (t == typeid(int))
        return to_czhstr(val.get<int>(), with_color);
      if (t == typeid(long long))
        return to_czhstr(val.get<long long>(), with_color);
      else if (t == typeid(std::string))
        return to_czhstr(val.get<std::string>(), with_color);
      else if (t == typeid(double))
        return to_czhstr(val.get<double>(), with_color);
      else if (t == typeid(bool))
        return to_czhstr(val.get<bool>(), with_color);
      else if (t == typeid(std::vector<int>))
        return vector_to_string(val.get<std::vector<int>>(), with_color);
      else if (t == typeid(std::vector<long long>))
        return vector_to_string(val.get<std::vector<long long>>(), with_color);
      else if (t == typeid(std::vector<std::string>))
        return vector_to_string(val.get<std::vector<std::string>>(), with_color);
      else if (t == typeid(std::vector<double>))
        return vector_to_string(val.get<std::vector<double>>(), with_color);
      else if (t == typeid(std::vector<bool>))
        return vector_to_string(val.get<std::vector<bool>>(), with_color);
      
      //Node*
      if (t != typeid(Node *))
        throw Error(CZH_ERROR_LOCATION, __func__, "Unexpected error", Error::internal);
      
      std::string res;
      auto path = *val.get<Node *>()->get_path();
      auto this_path = *get_path();
      std::reverse(path.begin(), path.end());
      std::reverse(this_path.begin(), this_path.end());
      std::size_t samepos = 0;
      for (auto i = 0; i < std::min(path.size(), this_path.size()); i++)
      {
        if (path[i] == this_path[i])
        {
          samepos++;
          if (i == this_path.size())
          {
            res += "-.";
            break;
          }
        } else
        {
          if (i == this_path.size() - 1)
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
        res += colorify(*it, with_color, Type::REF_BLOCK_ID);
      }
      res += ":";
      res += colorify(*path.crbegin(), with_color, Type::REF_ID);
      return res;
    }
  };
  
  std::ostream &operator<<(std::ostream &os, const Node &node)
  {
    os << node.to_string();
    return os;
  }
}