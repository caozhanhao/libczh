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

#include "error.hpp"
#include "node.hpp"
#include "parser.hpp"
#include <fstream>
#include <iostream>
#include <memory>

using czh::parser::Parser;
using czh::node::Node;
using czh::lexer::Lexer;
using czh::error::Error;
using czh::error::CzhError;
namespace czh
{
  enum class InputMode
  {
    stream, nonstream, string
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
      if (mode == InputMode::nonstream)
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
  
    std::unique_ptr<Node> parse()
    {
      std::unique_ptr<Node> ret;
      try
      {
        ret = parser.parse();
      }
      catch (czh::error::Error &err)
      {
        std::cout << "Internal:\n" << err.get_content() << std::endl;
      }
      catch (czh::error::CzhError &err)
      {
        std::cout << err.get_content() << std::endl;
      }
      error::czh_assert(ret != nullptr, "Unexpected nullptr from Parser::parse");
      return ret;
    }
  };
}
#endif