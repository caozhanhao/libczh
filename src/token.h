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
#include "file.h"
#include "utils.h"
#include "err.h"
#include <string>
#include <variant>
#include <memory>
namespace czh::token
{
  enum class TokenType
  {
    ID,
    INT, LONGLONG, DOUBLE, STRING, BOOL,
    EQUAL,//=
    ARR_LP, ARR_RP,//{}
    COMMA, COLON,
    REF,//::
    FEND, SEND, SCEND,
    NOTE,
    UNEXPECTED
  };
  
  class Pos
  {
  private:
    static constexpr std::size_t last = 3;
    static constexpr std::size_t next = 3;
  public:
    std::size_t pos;
    std::size_t size;
    std::shared_ptr<file::File> code;
  public:
    explicit Pos(std::shared_ptr<file::File> code_)
        : pos(0), size(0), code(std::move(code_))
    {}
    
    Pos &operator+=(const std::size_t &p)
    {
      pos += p;
      return *this;
    }
    
    Pos &operator-=(const std::size_t &p)
    {
      pos -= p;
      return *this;
    }
    
    [[nodiscard]] std::string location() const
    {
      return (code->get_name() + ":line " + utils::to_str(code->get_lineno(pos)));
    }
    
    [[nodiscard]] std::size_t get() const
    {
      return pos;
    }
  
  public:
    Pos &set_size(std::size_t s)
    {
      size = s;
      return *this;
    }
    
    [[nodiscard]] std::unique_ptr<std::string> get_details_from_code() const
    {
      std::size_t lineno = code->get_lineno(pos);
      std::size_t linenosize = utils::to_str(lineno + next).size();
      std::size_t actual_last = last;
      std::size_t actual_next = next;
      std::size_t total_line = code->get_lineno(code->size() - 1);
      
      while (static_cast<int>(lineno) - static_cast<int>(actual_last) <= 0 && actual_last > 0)
      {
        --actual_last;
      }
      while (lineno + actual_next >= total_line && actual_next > 0)
      {
        --actual_next;
      }
  
      std::string temp1, temp2;
      //lineno - actual_last is impossible to be equal lineno + 1
      temp1 = code->get_spec_line(lineno - actual_last, lineno + 1, linenosize);//[beg, end)
      if (actual_next != 0)
      {//lineno + 1 might be equal to lineno + actual_next + 1
        temp2 = code->get_spec_line(lineno + 1, lineno + actual_next + 1, linenosize);
      }
      std::string arrow("\n");
      std::size_t arrowpos = code->get_arrowpos(pos) - size + linenosize + 1;
      arrow += std::string(arrowpos, ' ');
      arrow += "\033[0;32;32m";
      arrow.insert(arrow.end(), size, '^');
      arrow += "\033[m\n";
  
      std::string errorstring = temp1 + arrow + temp2;
      return std::make_unique<std::string>(errorstring);
    }
  };
  
  template<typename T>
  std::string to_token_str(const T &v)
  { return utils::to_str(v); }
  
  template<>
  std::string to_token_str(const std::string &v)
  { return v; }
  
  //not use
  template<>
  std::string to_token_str(node::Node *const &v)
  { return ""; }
  
  std::string to_token_str(const value::Array &v)
  { return ""; }
  
  //end
  class Token
  {
  public:
    TokenType type;
    Value what;
    Pos pos;
  public:
    template<typename T>
    Token(TokenType type_, T what_, Pos pos_)
        :type(type_), what(std::move(what_)), pos(std::move(pos_))
    {}
  
    Token(const Token &) = delete;
  
    Token(Token &&) = default;
  
    Token &operator=(Token &&) = default;
  
    void error(const std::string &details) const
    {
      throw error::CzhError(pos.location(), details + ": \n"
                                            + *(pos.get_details_from_code()));
    }
  
    [[nodiscard]] std::string get_string() const
    {
      return std::visit(
          utils::overloaded{
              [](auto &&i) -> auto
              { return czh::token::to_token_str(i); },
              [](char i) -> auto
              { return std::string(1, i); }
          }, what.get_variant());
    }
  };
}