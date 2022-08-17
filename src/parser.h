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

#include "lexer.h"
#include "token.h"
#include "node.h"
#include "err.h"

#include <vector>
#include <map>
#include <cstdio>
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
    int note;
  public:
    explicit Parser(lexer::Lexer *lex_)
        : lex(lex_), note(0), node(std::make_shared<node::Node>()), curr_node(node.get())
    {}
    
    std::shared_ptr<node::Node> parse()
    {
      while (check())
      {
        switch (view().type)
        {
          case token::TokenType::ID:
            parse_id();
            break;
          case token::TokenType::SCEND:
            parse_end();
            break;
          case token::TokenType::NOTE:
            curr_node->add(utils::to_str(note), view().what);
            note++;
            next();
            break;
          case token::TokenType::SEND:
            next();
            break;
          case token::TokenType::FEND:
            return node;
          default:
            auto t = view();
            t.error("unexpected token");
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
        view().error("Unexpected scope end.");
      if (check())
        next();
    }
    
    void parse_id()
    {
      if (!check()) return;
      auto id_name = view().what.get<std::string>();
      next();//eat name
      // id:
      if (view().type == token::TokenType::COLON)//scope
      {
        next();
        curr_node = &curr_node->add_node(id_name);
        return;
      }
      //id = xxx
      next();//eat '='
      if (view().type == token::TokenType::BPATH)//ref id = -x:x
      {
        curr_node->add(id_name, parse_ref());
        return;
      }
      if (view().type == token::TokenType::ARR_LP)// array id = [1,2,3]
      {
        if (view(1).type == token::TokenType::ARR_RP)
        {
          curr_node->add(id_name, std::vector<int>());
        }
        else if (is_any_array())
        {
          curr_node->add(id_name,
                         *parse_any_array());
          return;
        }
        else
        {
          if (view(1).type == token::TokenType::INT)
            curr_node->add(id_name, *parse_array<int>());
          else if (view(1).type == token::TokenType::LONGLONG)
            curr_node->add(id_name, *parse_array<long long>());
          else if (view(1).type == token::TokenType::DOUBLE)
            curr_node->add(id_name, *parse_array<double>());
          else if (view(1).type == token::TokenType::STRING)
            curr_node->add(id_name, *parse_array<std::string>());
          else if (view(1).type == token::TokenType::BOOL)
            curr_node->add(id_name, *parse_array<bool>());
          return;
        }
      }
      curr_node->add(id_name, view().what);
      next();//eat value
    }
    
    node::Node *parse_ref()
    {
      if (!check()) return nullptr;
      node::Node *call = curr_node;
      node::Node *val = nullptr;
      for (; view().type != token::TokenType::COLON; next())
      {
        if (view().type == token::TokenType::BPATH) continue;
        call = to_scope(view().what.get<std::string>(), call);
      }
      next(); //eat :
      try
      {
        val = &(*call)[view().what.get<std::string>()];
      }
      catch (Error &err)
      {
        view().error(err.get_detail());
      }
      next();//eat id
      return val;
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
          view().error(err.get_detail());
        }
      }
      return nullptr;
    }
    
    template<typename T>
    std::unique_ptr<std::vector<T>> parse_array()
    {
      std::unique_ptr<std::vector<T>> ret = std::make_unique<std::vector<T>>();
      next();//eat [
      for (; view().type != token::TokenType::ARR_RP; next())
      {
        if (view().type == token::TokenType::COMMA) continue;
        ret->emplace_back(view().what.get<T>());
      }
      next();//eat ]
      return ret;
    }
    
    std::unique_ptr<value::Value::AnyArray> parse_any_array()
    {
      std::unique_ptr<value::Value::AnyArray> ret = std::make_unique<value::Value::AnyArray>();
      next();//eat [
      for (; view().type != token::TokenType::ARR_RP; next())
      {
        if (view().type == token::TokenType::COMMA) continue;
        
        if (view().type == token::TokenType::INT)
          ret->emplace_back(view().what.get<int>());
        else if (view().type == token::TokenType::LONGLONG)
          ret->emplace_back(view().what.get<long long>());
        else if (view().type == token::TokenType::DOUBLE)
          ret->emplace_back(view().what.get<double>());
        else if (view().type == token::TokenType::STRING)
          ret->emplace_back(view().what.get<std::string>());
        else if (view().type == token::TokenType::BOOL)
          ret->emplace_back(view().what.get<bool>());
      }
      next();//eat ]
      return ret;
    }
    
    bool is_any_array()
    {
      int v = 1;
      token::TokenType arrtype(token::TokenType::UNEXPECTED);
      while (view(v).type != token::TokenType::ARR_RP)
      {
        if (view(v).type == token::TokenType::COMMA)
        {
          ++v;
          continue;
        }
        else if (arrtype == token::TokenType::UNEXPECTED)
          arrtype = view(v).type;
        else if (view(v).type != arrtype)
          return true;
        ++v;
      }
      return false;
    }
    
    bool check()
    {
      return !lex->eof();
    }
    
    token::Token view(std::size_t s = 0)
    {
      return lex->view(s);
    }
    
    void next()
    {
      lex->next();
    }
  };
}