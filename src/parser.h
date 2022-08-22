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
#ifndef LIBCZH_PARSER_H
#define LIBCZH_PARSER_H

#include "lexer.h"
#include "token.h"
#include "node.h"
#include "err.h"

#include <vector>
#include <string>
#include <memory>

namespace czh::parser
{
  class Parser
  {
  private:
    lexer::Lexer *lex;
    std::shared_ptr<node::Node> node;
    node::Node *curr_node;
    token::Token curr_tok;
  public:
    explicit Parser(lexer::Lexer *lex_)
        : lex(lex_), node(std::make_unique<node::Node>()), curr_node(node.get()),
          curr_tok(token::TokenType::UNEXPECTED, 0, token::Pos(0)) {}
    
    std::shared_ptr<node::Node> parse()
    {
      curr_tok = get();
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
            return node;
          default:
            curr_tok.error("unexpected token");
            break;
        }
      }
      return node;
    }
  
  private:
    void parse_end()
    {
      curr_node = curr_node->to_last_node();
      if (!curr_node)
      {
        curr_tok.error("Unexpected scope end.");
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
      if (curr_node->has_node(id_name))
      {
        curr_tok.error("Node cannot be duplicated.");
      }
      curr_tok = get();//eat name
      // id:
      if (curr_tok.type == token::TokenType::COLON)//scope
      {
        curr_tok = get();//eat ':'
        curr_node = &curr_node->add_node(id_name);
        return;
      }
      //id = xxx
      curr_tok = get();//eat '='
      if (curr_tok.type == token::TokenType::ID || curr_tok.type == token::TokenType::REF)//ref id = -x:x
      {
        curr_node->add(id_name, parse_ref());
        return;
      }
      else if (curr_tok.type == token::TokenType::ARR_LP)// array id = [1,2,3]
      {
        curr_node->add(id_name, *parse_array());
        return;
      }
      curr_node->add(id_name, curr_tok.what);
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
            call = call->to_last_node();
        }
        if (call == nullptr)
        {
          curr_tok.error("There is no node named '" + name + "'.");
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
          call = &(*call)[curr_tok.what.get<std::string>()];
        }
        catch (Error &err)
        {
          curr_tok.error(err.get_detail());
        }
        curr_tok = get();
        if (curr_tok.type != token::TokenType::REF)
        {
          return call;
        }
      }
      return nullptr;
    }
    
    node::Node *to_scope(const std::string &id, node::Node *ptr)
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
        catch (Error &err)
        {
          curr_tok.error(err.get_detail());
        }
      }
      return nullptr;
    }
  
    std::unique_ptr<value::Array> parse_array()
    {
      std::unique_ptr<value::Array> ret = std::make_unique<value::Array>();
      curr_tok = get();//eat [
      for (; curr_tok.type != token::TokenType::ARR_RP; curr_tok = get())
      {
        if (curr_tok.type == token::TokenType::COMMA) continue;
  
        if (curr_tok.type == token::TokenType::INT)
        {
          ret->insert(ret->end(), curr_tok.what.get<int>());
        }
        else if (curr_tok.type == token::TokenType::LONGLONG)
        {
          ret->insert(ret->end(), curr_tok.what.get<long long>());
        }
        else if (curr_tok.type == token::TokenType::DOUBLE)
        {
          ret->insert(ret->end(), curr_tok.what.get<double>());
        }
        else if (curr_tok.type == token::TokenType::STRING)
        {
          ret->insert(ret->end(), curr_tok.what.get<std::string>());
        }
        else if (curr_tok.type == token::TokenType::BOOL)
        {
          ret->insert(ret->end(), curr_tok.what.get<bool>());
        }
      }
      curr_tok = get();//eat ]
      return ret;
    }
  
    bool check()
    {
      return !lex->eof();
    }
  
    token::Token &peek()
    {
      return lex->peek();
    }
  
    token::Token get()
    {
      return lex->get();
    }
  };
}
#endif