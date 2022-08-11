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