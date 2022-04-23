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
using czh::error::Err;
namespace czh
{

	class Czh
	{
	private:
		Lexer lexer;
		Parser parser;
	public:
		Czh(const std::string& czh_path)
			:lexer(czh_path, czh_path) {}
		std::shared_ptr<Node> parse()
		{
			parser.set_tokens(lexer.get_all_token());
			return  parser.parse();
		}
	};
}