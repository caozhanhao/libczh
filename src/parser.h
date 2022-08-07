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
    std::shared_ptr<std::vector<Token>> tokens;
    std::size_t pos;
    std::shared_ptr<Node> node;
    Node *curr_node;
    int note;
  public:
    explicit Parser(std::shared_ptr<std::vector<Token>> tokens_ = {})
        : tokens(std::move(tokens_)), pos(0), note(0), node(std::make_shared<Node>(Node())), curr_node(node.get()) {}
    
    std::shared_ptr<Node> parse()
    {
      while (check())
      {
        switch (get().type)
        {
          case TokenType::ID:
            parse_id();
            break;
          case TokenType::SCEND:
            parse_end();
            break;
          case TokenType::NOTE:
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
    
    void set_tokens(const std::shared_ptr<std::vector<Token>> &_tokens)
    {
      tokens = _tokens;
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
      if (get().type == TokenType::COLON)//scope
      {
        next();
        curr_node = curr_node->add_node(id_name);
        return;
      }
      //id = xxx
      next();//eat '='
      if (get().type == TokenType::BPATH)//ref id = -x:x
      {
        curr_node->add(id_name, parse_ref());
        return;
      }
      if (get().type == TokenType::ARR_LP)// array id = [1,2,3]
      {
        if (get(1).type == TokenType::INT)
          curr_node->add(id_name, *parse_array<int>());
        else if (get(1).type == TokenType::DOUBLE)
          curr_node->add(id_name, *parse_array<double>());
        else if (get(1).type == TokenType::STRING)
          curr_node->add(id_name, *parse_array<std::string>());
        else if (get(1).type == TokenType::BOOL)
          curr_node->add(id_name, *parse_array<bool>());
        return;
      }
      curr_node->add(id_name, get().what);
      next();//eat value
    }
    
    Node *parse_ref()
    {
      if (!check()) return nullptr;
      Node *call = curr_node;
      Node *val = nullptr;
      for (; get().type != TokenType::COLON; next())
      {
        if (get().type == TokenType::BPATH) continue;
        call = to_scope(get().what.get<std::string>(), call);
      }
      next(); //eat :
      try
      {
        val = &(*call)[get().what.get<std::string>()];
      }
      catch (Error &err)
      {
        get().error(err.get_detail());
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
      for (; get().type != TokenType::ARR_RP; next())
      {
        if (get().type == TokenType::COMMA) continue;
        ret->push_back(get().what.get<T>());
      }
      next();//eat ]
      return ret;
    }
    
    bool check(const int &s = 0)
    {
      return (pos + s) < tokens->size();
    }
    
    Token &get(const int &s = 0)
    {
      if (!check(s))
        get(s - 1).error("Unexpected end of tokens.");
      return (*tokens)[pos + s];
    }
    
    void next(const int &s = 1)
    {
      pos += s;
    }
  };
}