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
#ifndef LIBCZH_PARSER_HPP
#define LIBCZH_PARSER_HPP
#pragma once

#include "lexer.hpp"
#include "token.hpp"
#include "node.hpp"
#include "error.hpp"

#include <vector>
#include <string>
#include <memory>

namespace czh::parser
{
  class Parser
  {
  private:
    lexer::Lexer *lex;
    node::Node node;
    node::Node *curr_node;
    token::Token curr_tok;
  public:
    explicit Parser(lexer::Lexer *lex_)
        : lex(lex_), node(node::Node()), curr_node(&node),
          curr_tok(token::TokenType::UNEXPECTED, 0, token::Pos(nullptr)) {}
  
    node::Node parse()
    {
      if (curr_node == nullptr)
      {
        node.reset();
        curr_node = &node;
        lex->reset();
      }
      curr_tok = get();
      error::czh_assert(curr_tok.type != token::TokenType::FEND, "Unexpected end of czh.");
      while (check())
      {
        switch (curr_tok.type)
        {
          case token::TokenType::ID:
            parse_id();
            break;
          case token::TokenType::SCEND:
            parse_end();
            break;
          case token::TokenType::SEND:
            curr_tok = get();
            break;
          case token::TokenType::FEND:
            curr_node = nullptr;
            return std::move(node);
          default:
            error::czh_unreachable("Unexpected token");
            break;
        }
      }
      curr_node = nullptr;
      return std::move(node);
    }
  
  private:
    void parse_end()
    {
      curr_node = curr_node->get_last_node();
      if (!curr_node)
      {
        curr_tok.report_error("Unexpected scope end.");
      }
      if (check())
      {
        curr_tok = get();
      }
    }
    
    void parse_id()
    {
      if (!check()) return;
      auto id_name = curr_tok.what.get<std::string>();
      if (curr_node->has_node(id_name)) curr_tok.report_error("Duplicate node name.");
      auto bak = token::Token(curr_tok);
      curr_tok = get();//eat name
      // id:
      if (curr_tok.type == token::TokenType::COLON)//scope
      {
        curr_tok = get();//eat ':'
        curr_node = &curr_node->add_node(id_name, "", std::move(bak));
        return;
      }
      //id = xxx
      curr_tok = get();//eat '='
      if (curr_tok.type == token::TokenType::ID || curr_tok.type == token::TokenType::REF)//ref id = -x:x
      {
        curr_node->add(id_name, parse_ref(), "", std::move(bak));
        return;
      }
      else if (curr_tok.type == token::TokenType::ARR_LP)// array id = [1,2,3]
      {
        curr_node->add(id_name, parse_array(), "", std::move(bak));
        return;
      }
      curr_node->add(id_name, curr_tok.what, "", std::move(bak));
      curr_tok = get();//eat value
    }
  
    value::Reference parse_ref()
    {
      std::vector<std::string> path;
      if (curr_tok.type == token::TokenType::REF)
      {
        path = {""};
      }
      bool id = false;
      while (curr_tok.type == token::TokenType::ID || curr_tok.type == token::TokenType::REF)
      {
        if (curr_tok.type == token::TokenType::ID)
        {
          if (id) break;// double id
          path.insert(path.begin(), curr_tok.what.get<std::string>());
          id = true;
        }
        else
        {
          id = false;
        }
        curr_tok = get();
      }
      return {path};
    }
  
    value::Array parse_array()
    {
      value::Array ret;
      curr_tok = get();//eat [
      for (; curr_tok.type != token::TokenType::ARR_RP; curr_tok = get())
      {
        if (curr_tok.type == token::TokenType::COMMA) continue;
        std::visit(utils::overloaded{
            [&ret](auto &&a) { ret.insert(ret.end(), a); },
            [](value::Reference) { error::czh_unreachable(); },
            [](value::Array) { error::czh_unreachable(); }
        }, curr_tok.what.get_variant());
      }
      curr_tok = get();//eat ]
      return ret;
    }
    
    bool check()
    {
      return !lex->eof();
    }
    
    token::Token get()
    {
      return lex->get();
    }
  };
}
#endif