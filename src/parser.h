#pragma once

#include "lexer.h"
#include "node.h"
#include "err.h"

#include <vector>
#include <map>
#include <cstdio>
#include <string>
#include <memory>  
using czh::lexer::TokenType;
using czh::lexer::Token;
using czh::node::Node;
using czh::error::Error;
namespace czh::parser
{
  class Parser
  {
  private:
    lexer::Lexer *lex;
    std::shared_ptr<Node> node;
    Node *curr_node;
    int note;
  public:
    explicit Parser(lexer::Lexer *lex_)
        : lex(lex_), note(0), node(std::make_shared<Node>(Node())), curr_node(node.get())
    {}
    
    std::shared_ptr<Node> parse()
    {
      while (check())
      {
        switch (view().type)
        {
          case TokenType::ID:
            parse_id();
            break;
          case TokenType::SCEND:
            parse_end();
            break;
          case TokenType::NOTE:
            curr_node->add(utils::to_str(note), view().what);
            note++;
            next();
            break;
          case TokenType::SEND:
            next();
            break;
          case TokenType::FEND:
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
      if (check())
        next();
    }
    
    void parse_id()
    {
      if (!check()) return;
      auto id_name = view().what.get<std::string>();
      next();//eat name
      // id:
      if (view().type == TokenType::COLON)//scope
      {
        next();
        curr_node = curr_node->add_node(id_name);
        return;
      }
      //id = xxx
      next();//eat '='
      if (view().type == TokenType::BPATH)//ref id = -x:x
      {
        curr_node->add(id_name, parse_ref());
        return;
      }
      if (view().type == TokenType::ARR_LP)// array id = [1,2,3]
      {
        if (view(1).type == TokenType::ARR_RP)
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
          if (view(1).type == TokenType::INT)
            curr_node->add(id_name, *parse_array<int>());
          else if (view(1).type == TokenType::LONGLONG)
            curr_node->add(id_name, *parse_array<long long>());
          else if (view(1).type == TokenType::DOUBLE)
            curr_node->add(id_name, *parse_array<double>());
          else if (view(1).type == TokenType::STRING)
            curr_node->add(id_name, *parse_array<std::string>());
          else if (view(1).type == TokenType::BOOL)
            curr_node->add(id_name, *parse_array<bool>());
          return;
        }
      }
      curr_node->add(id_name, view().what);
      next();//eat value
    }
    
    Node *parse_ref()
    {
      if (!check()) return nullptr;
      Node *call = curr_node;
      Node *val = nullptr;
      for (; view().type != TokenType::COLON; next())
      {
        if (view().type == TokenType::BPATH) continue;
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
    
    Node *to_scope(const std::string &id, Node *ptr)
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
      for (; view().type != TokenType::ARR_RP; next())
      {
        if (view().type == TokenType::COMMA) continue;
        ret->emplace_back(view().what.get<T>());
      }
      next();//eat ]
      return ret;
    }
    
    std::unique_ptr<value::Value::AnyArray> parse_any_array()
    {
      std::unique_ptr<value::Value::AnyArray> ret = std::make_unique<value::Value::AnyArray>();
      next();//eat [
      for (; view().type != TokenType::ARR_RP; next())
      {
        if (view().type == TokenType::COMMA) continue;
        
        if (view().type == TokenType::INT)
          ret->emplace_back(view().what.get<int>());
        else if (view().type == TokenType::LONGLONG)
          ret->emplace_back(view().what.get<long long>());
        else if (view().type == TokenType::DOUBLE)
          ret->emplace_back(view().what.get<double>());
        else if (view().type == TokenType::STRING)
          ret->emplace_back(view().what.get<std::string>());
        else if (view().type == TokenType::BOOL)
          ret->emplace_back(view().what.get<bool>());
      }
      next();//eat ]
      return ret;
    }
    
    bool is_any_array()
    {
      int v = 1;
      TokenType arrtype(TokenType::UNEXPECTED);
      while (view(v).type != TokenType::ARR_RP)
      {
        if (view(v).type == TokenType::COMMA)
        {
          ++v;
          continue;
        }
        else if (arrtype == TokenType::UNEXPECTED)
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
    
    Token view(std::size_t s = 0)
    {
      return lex->view(s);
    }
    
    void next()
    {
      lex->next();
    }
  };
}