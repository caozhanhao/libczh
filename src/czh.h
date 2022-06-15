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

	class Czh
	{
  public:
    static const bool file = false;
	private:
		Lexer lexer;
		Parser parser;
	public:
    explicit Czh(std::string code)
    : lexer(std::move(code))
    {}
	  explicit Czh(const std::string& czh_, bool is_file = file)
    : lexer(czh_, czh_) {}
		std::shared_ptr<Node> parse()
		{
			parser.set_tokens(lexer.get_all_token());
			return  parser.parse();
		}
	};
}