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
#include "err.h"
#include "node.h"
#include "parser.h"
#include <fstream>
#include <sstream>
#include <memory>
using czh::parser::Parser;
using czh::node::Node;
using czh::lexer::Lexer;
using czh::error::Error;
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
        lexer.set_czh(path, path);
      else if (mode == InputMode::stream)
        lexer.set_czh(path, std::make_unique<std::ifstream>(path));
      else
        lexer.set_czh(path);
    }
    
    std::shared_ptr<Node> parse()
    {
      return parser.parse();
    }
  };
}