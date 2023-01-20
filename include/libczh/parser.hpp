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
    std::unique_ptr<node::Node> node;
    node::Node *curr_node;
    token::Token curr_tok;
  public:
    explicit Parser(lexer::Lexer *lex_)
        : lex(lex_), node(std::make_unique<node::Node>()), curr_node(node.get()),
          curr_tok(token::TokenType::UNEXPECTED, 0, token::Pos(nullptr)) {}
  
    std::unique_ptr<node::Node> parse()
    {
      curr_tok = get();
      std::unique_ptr<node::Node> ret;
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
            ret.reset(node.release());
            node = nullptr;
            curr_node = nullptr;
            return std::move(ret);
          default:
            error::czh_unreachable("Unexpected token");
            break;
        }
      }
      ret.reset(node.release());
      node = nullptr;
      curr_node = nullptr;
      return std::move(ret);
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
      auto bak = curr_tok;
      curr_tok = get();//eat name
      // id:
      if (curr_tok.type == token::TokenType::COLON)//scope
      {
        curr_tok = get();//eat ':'
        curr_node = &curr_node->add_node(id_name, "", bak);
        return;
      }
      //id = xxx
      curr_tok = get();//eat '='
      if (curr_tok.type == token::TokenType::ID || curr_tok.type == token::TokenType::REF)//ref id = -x:x
      {
        curr_node->add(id_name, parse_ref(), "", bak);
        return;
      }
      else if (curr_tok.type == token::TokenType::ARR_LP)// array id = [1,2,3]
      {
        curr_node->add(id_name, parse_array(), "", bak);
        return;
      }
      curr_node->add(id_name, curr_tok.what, "", bak);
      curr_tok = get();//eat value
    }
    
    node::Node *parse_ref()
    {
      if (!check()) return nullptr;
      node::Node *call = curr_node;
      if (curr_tok.type == token::TokenType::REF)
      {
        call = node.get();
      }
      else
      {
        auto name = curr_tok.what.get<std::string>();
        while (call != nullptr)
        {
          if (call->has_node(name))
          {
            call = &(*call)[name];
            curr_tok = get();
            if (curr_tok.type != token::TokenType::REF)
            {
              return call;
            }
            else
            {
              break;
            }
          }
          else
          {
            call = call->get_last_node();
          }
        }
        if (call == nullptr)
        {
          curr_tok.report_error("There is no node named '" + name + "'.");
        }
      }
      
      while (true)
      {
        if (curr_tok.type == token::TokenType::REF)
        {
          curr_tok = get();
        }
        try
        {
          error::czh_assert(call != nullptr);
          call = &(*call)[curr_tok.what.get<std::string>()];
        }
        catch (error::Error &err)
        {
          curr_tok.report_error(err.get_detail());
        }
        curr_tok = get();
        if (curr_tok.type != token::TokenType::REF)
        {
          return call;
        }
      }
      error::czh_unreachable();
      return nullptr;
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
            [](node::Node *) { error::czh_unreachable(); },
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