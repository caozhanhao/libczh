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
#ifndef LIBCZH_WRITER_HPP
#define LIBCZH_WRITER_HPP
#pragma once

#include <string>

namespace czh::node { class Node; }
namespace czh::writer
{
  template<typename T>
  concept Writer =
  requires(T w)
  {
    { w.node_begin(std::string()) };
    { w.node_end() };
    { w.value_begin(std::string()) };
    { w.value(value::Value{}) };
    { w.value_ref_path_set_global() };
    { w.value_ref_path(std::string()) };
    { w.value_ref_id(std::string()) };
    { w.value_array_begin() };
    { w.value_array_value(value::Array::value_type{}) };
    { w.value_array_end(value::Array::value_type{}) };
  };
  
  template<typename Os>
  void write_array_value(Os *&os, const value::Array::value_type &v, utils::Color c = utils::Color::no_color)
  {
    std::visit(
        [&os, &c](auto &&i) { *os << utils::to_czhstr(i, c); },
        v);
  }
  
  template<typename Os>
  void write_value(Os *&os, const value::Value &v, utils::Color c = utils::Color::no_color)
  {
    std::visit(
        utils::overloaded{
            [&os, &c](auto &&i) { *os << utils::to_czhstr(i, c); },
            [](node::Node *i) { error::czh_unreachable(); },
        },
        v.get_variant());
  }
  
  template<typename Os>
  class BasicWriter
  {
  private:
    Os *os;
  public:
    BasicWriter(Os &os_) : os(&os_) {}
    
    void node_begin(const std::string &name)
    {
      *os << name << ":";
    }
    
    void node_end()
    {
      *os << "end;";
    }
    
    void value_begin(const std::string &name)
    {
      *os << name << "=";
    }
    
    void value(const value::Value &v)
    {
      write_value(os, v);
      *os << ";";
    }
    
    void value_ref_path_set_global()
    {
      *os << "::";
    }
    
    void value_ref_path(const std::string &name)
    {
      *os << name << "::";
    }
    
    void value_ref_id(const std::string &name)
    {
      *os << name << ";";
    }
    
    void value_array_begin()
    {
      *os << "{";
    }
    
    void value_array_value(const value::Array::value_type &v)
    {
      write_array_value(os, v);
      *os << ",";
    }
    
    void value_array_end(const value::Array::value_type &v)
    {
      write_array_value(os, v);
      *os << "};";
    }
  };
  
  template<typename Os>
  class PrettyWriter
  {
  private:
    Os *os;
    size_t indentation;
    size_t pos;
  public:
    PrettyWriter(Os &os_, size_t indentation_ = 2) : os(&os_), indentation(indentation_), pos(0) {}
    
    void node_begin(const std::string &name)
    {
      *os << std::string(indentation * pos++, ' ') << name << ":\n";
    }
    
    void node_end()
    {
      *os << std::string(indentation * --pos, ' ') << "end\n";
    }
    
    void value_begin(const std::string &name)
    {
      *os << std::string(indentation * pos, ' ') << name << " = ";
    }
    
    void value(const value::Value &v)
    {
      std::visit(
          utils::overloaded{
              [this](auto &&i) { *os << utils::to_czhstr(i); },
              [](node::Node *i) { error::czh_unreachable(); },
          },
          v.get_variant());
      *os << "\n";
    }
    
    void value_ref_path_set_global()
    {
      *os << "::";
    }
    
    void value_ref_path(const std::string &name)
    {
      *os << name << "::";
    }
    
    void value_ref_id(const std::string &name)
    {
      *os << name << "\n";
    }
    
    void value_array_begin()
    {
      *os << "{";
    }
    
    void value_array_value(const value::Array::value_type &v)
    {
      write_array_value(os, v);
      *os << ", ";
    }
    
    void value_array_end(const value::Array::value_type &v)
    {
      write_array_value(os, v);
      *os << "}\n";
    }
  };
  
  template<typename Os>
  class ColorWriter
  {
  private:
    Os *os;
    size_t indentation;
    size_t pos;
  public:
    ColorWriter(Os &os_, size_t indentation_ = 2) : os(&os_), indentation(indentation_), pos(0) {}
    
    void node_begin(const std::string &name)
    {
      *os << std::string(indentation * pos++, ' ')
          << utils::colorify(name, utils::Color::with_color, utils::ColorType::BLOCK_BEG)
          << ":\n";
    }
    
    void node_end()
    {
      *os << std::string(indentation * --pos, ' ')
          << utils::colorify("end", utils::Color::with_color, utils::ColorType::BLOCK_END)
          << "\n";
    }
    
    void value_begin(const std::string &name)
    {
      *os << std::string(indentation * pos, ' ')
          << utils::colorify(name, utils::Color::with_color, utils::ColorType::ID)
          << " = ";
    }
    
    void value(const value::Value &v)
    {
      std::visit(
          utils::overloaded{
              [this](auto &&i) { *os << utils::to_czhstr(i, utils::Color::with_color); },
              [](node::Node *i) { error::czh_unreachable(); },
          },
          v.get_variant());
      *os << "\n";
    }
    
    void value_ref_path_set_global()
    {
      *os << "::";
    }
    
    void value_ref_path(const std::string &name)
    {
      *os << name << "::";
    }
    
    void value_ref_id(const std::string &name)
    {
      *os
          << utils::colorify(name, utils::Color::with_color, utils::ColorType::REF_ID)
          << "\n";
    }
    
    void value_array_begin()
    {
      *os << "{";
    }
    
    void value_array_value(const value::Array::value_type &v)
    {
      write_array_value(os, v, utils::Color::with_color);
      *os << ", ";
    }
    
    void value_array_end(const value::Array::value_type &v)
    {
      write_array_value(os, v, utils::Color::with_color);
      *os << "}\n";
    }
  };
}
#endif