#pragma once

#include "lexer.h"
#include "node.h"
#include "err.h"

#include <vector>
#include <map>
#include <cstdio>
#include <string>
#include <memory>  
using czh::lexer::Type;
using czh::lexer::Token;
using czh::node::Node;
using czh::error::Error;
namespace czh
{
	namespace parser
	{

		class Parser
		{
		private:
			std::shared_ptr<std::vector<Token>> tokens;
			std::size_t pos;
			std::shared_ptr<Node> node;
			Node* curr_node;
      int note;
		public:
			Parser(const std::shared_ptr<std::vector<Token>>& _tokens = {})
				: tokens(_tokens), pos(0), note(0),node(std::make_shared<Node>(Node())), curr_node(node.get()) {}
			std::shared_ptr<Node> parse()
			{
				while (check())
				{
					switch (get().type)
					{
					case Type::ID_TOK:        parse_id();  break;
					case Type::SCOPE_END_TOK: parse_end(); break;
          case Type::NOTE_TOK:
            curr_node->add(std::to_string(note), get().what);
            note++;
            next();
          break;
					default:
						get().error("unexpected token");
						break;
					}
				}
				return node;
			}
			void set_tokens(const std::shared_ptr<std::vector<Token>>& _tokens)
			{
				tokens = _tokens;
			}
			void reset()
			{
				tokens = nullptr;
				curr_node = nullptr;
				pos = 0;
			}
		private:
			void parse_end()
			{
				curr_node = curr_node->to_last_node();
				if (check())
					next();
			}
			void parse_id()
			{
				if (!check()) return;
				auto id_name = get().what.get<std::string>();
				next();//eat name
				// id:
				if (get().type == Type::COLON_TOK)//scope
				{
          next();
          curr_node = curr_node->add_node(id_name);
					return;
				}
				//id = xxx
				next();//eat '='
				if (get().type == Type::BPATH_TOK)//ref id = -x:x
				{
					curr_node->add(id_name, parse_ref() );
					return;
				}
				if (get().type == Type::ARR_LPAREN_TOK)// array id = [1,2,3]
				{
					if (get(1).type == Type::INT_TOK)
						curr_node->add(id_name, *parse_array<int>()) ;
					else if (get(1).type == Type::DOUBLE_TOK)
						curr_node->add(id_name, *parse_array<double>());
					else if (get(1).type == Type::STRING_TOK)
						curr_node->add(id_name, *parse_array<std::string>());
          else if (get(1).type == Type::BOOL_TOK)
            curr_node->add(id_name, *parse_array<bool>());
					return;
				}
				curr_node->add(id_name, get().what);
				next();//eat value
				return;
			}

			Node* parse_ref()
			{
				if (!check()) return nullptr;
				Node* call = curr_node;
				Node* val = nullptr;
				for (; get().type != Type::COLON_TOK; next())
				{
					if (get().type == Type::BPATH_TOK) continue;
					call = to_scope(get().what.get<std::string>(), call);
				}
				next(); //eat :
				try
				{
					val = &(*call)[get().what.get<std::string>()];
				}
				catch (Error& err)
				{
					get().error(err.get_detail());
				}
				next();//eat id
				return val;
			}
			Node* to_scope(const std::string& id, Node* ptr)
			{
				if (ptr == nullptr) ptr = node.get();

				if (id == ".") return ptr;
				else if (id == "..") return ptr->to_last_node();
				else
				{
					if (ptr->has_node(id))
						return &(*ptr)[id];
					else
						ptr = node.get();
					try
					{
						return &(*ptr)[id];
					}
					catch (Error& err)
					{
						get().error(err.get_detail());
					}
				}
				return nullptr;
			}
			template<typename T>
			std::unique_ptr<std::vector<T>> parse_array()
			{
				std::unique_ptr<std::vector<T>> ret = std::make_unique<std::vector<T>>(std::vector<T>());
				next();//eat [
				for (; get().type != Type::ARR_RPAREN_TOK; next())
				{
					if (get().type == Type::COMMA_TOK) continue;
					ret->push_back(get().what.get<T>());
				}
				next();//eat ]
				return ret;
			}

			bool check(const int& s = 0)
			{
				return (pos + s) < tokens->size();
			}
			Token& get(const int& s = 0)
			{
				if (!check(s))
					get(s - 1).error("Unexpected end of tokens.");
				return (*tokens)[pos + s];
			}
			void next(const int& s = 1)
			{
				pos += s;
			}
		};
	}
}