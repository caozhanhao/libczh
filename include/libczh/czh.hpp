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
#ifndef LIBCZH_CZH_HPP
#define LIBCZH_CZH_HPP
#pragma once

#include "dtoa.hpp"
#include "error.hpp"
#include "file.hpp"
#include "lexer.hpp"
#include "node.hpp"
#include "parser.hpp"
#include "token.hpp"
#include "utils.hpp"
#include "value.hpp"
#include "writer.hpp"
#include <fstream>
#include <iostream>
#include <memory>

namespace czh
{
  using czh::parser::Parser;
  using czh::node::Node;
  using czh::lexer::Lexer;
  using czh::error::Error;
  using czh::error::CzhError;
  using czh::writer::BasicWriter;
  using czh::writer::PrettyWriter;
  using czh::writer::ColorWriter;
  enum class InputMode
  {
    stream, file, string
  };
  
  class Czh
  {
  private:
    Lexer lexer;
    Parser parser;
  public:
    explicit Czh(const std::string &path, InputMode mode)
        : parser(&lexer)
    {
      if (mode == InputMode::file)
      {
        lexer.set_czh(path, path);
      }
      else if (mode == InputMode::stream)
      {
        lexer.set_czh(path, std::make_unique<std::ifstream>(path));
      }
      else
      {
        lexer.set_czh(path);
      }
    }
  
    Node parse()
    {
      return std::move(parser.parse());
    }
  };
  
  inline namespace literals
  {
    inline node::Node operator "" _czh(const char *c, size_t n)
    {
      Czh czh(std::string(c, n), InputMode::string);
      return std::move(czh.parse());
    }
  }
}
#endif