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

#include "value.h"
#include "token.h"
#include "file.h"
#include "err.h"
#include "utils.h"

#include <memory>
#include <limits>
#include <vector>
#include <queue>
#include <string>
#include <map>
#include <algorithm>
using czh::error::Error;
using czh::error::CzhError;
using czh::value::Value;
namespace czh::lexer
{
  enum class State
  {
    INIT,
    ID, VALUE,
    ARR_VALUE, EQUAL, ARR_LP, ARR_RP,
    COMMA, SC_COLON, PATH_COLON, BPATH, PATH_ID_TARGET, PATH_ID,
    UNEXPECTED, END
  };
  
  class Match
  {
  private:
    State state;
    State last_state;
  public:
    Match() : state(State::INIT), last_state(State::UNEXPECTED)
    {}
    
    std::string error_correct()
    {
      switch (last_state)
      {
        case State::INIT:
        case State::PATH_COLON:
        case State::BPATH:
          return "identifier";
        case State::ID:
          return "'=' or ':'";
        case State::EQUAL:
          return "value or '['";
        case State::ARR_LP:
          return "value or ']'";
        case State::ARR_VALUE:
          return "']' or ','";
        case State::COMMA:
          return "value";
        case State::PATH_ID:
          return "'-' or ':'";
        default:
          throw error::Error(CZH_ERROR_LOCATION, __func__, "Unexpected state.");
      }
      return "";
    }
    
    [[nodiscard]]State get_state() const
    {
      return state;
    }
    
    void match(const token::TokenType &token)
    {
      if (token == token::TokenType::NOTE) return;
      switch (state)
      {
        case State::INIT:
          switch (token)
          {
            case token::TokenType::ID:
              state = State::ID;
              break;
            case token::TokenType::SCEND:
              state = State::END;
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::ID:
          switch (token)
          {
            case token::TokenType::EQUAL:
              state = State::EQUAL;
              break;
            case token::TokenType::COLON:
              state = State::END;
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::EQUAL:
          switch (token)
          {
            case token::TokenType::INT:
            case token::TokenType::LONGLONG:
            case token::TokenType::DOUBLE:
            case token::TokenType::STRING:
            case token::TokenType::BOOL:
              state = State::END;
              break;
            case token::TokenType::ARR_LP:
              state = State::ARR_LP;
              break;
            case token::TokenType::BPATH:
              state = State::BPATH;
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::ARR_LP:
          switch (token)
          {
            case token::TokenType::INT:
            case token::TokenType::LONGLONG:
            case token::TokenType::DOUBLE:
            case token::TokenType::STRING:
            case token::TokenType::BOOL:
              state = State::ARR_VALUE;
              break;
            case token::TokenType::ARR_RP:
              state = State::END;
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::ARR_VALUE:
          switch (token)
          {
            case token::TokenType::COMMA:
              state = State::COMMA;
              break;
            case token::TokenType::ARR_RP:
              state = State::END;
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::COMMA:
          switch (token)
          {
            case token::TokenType::INT:
            case token::TokenType::LONGLONG:
            case token::TokenType::DOUBLE:
            case token::TokenType::STRING:
            case token::TokenType::BOOL:
              state = State::ARR_VALUE;
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::BPATH:
          switch (token)
          {
            case token::TokenType::ID:
              state = State::PATH_ID;
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::PATH_ID:
          switch (token)
          {
            case token::TokenType::BPATH:
              state = State::BPATH;
              break;
            case token::TokenType::COLON:
              state = State::PATH_COLON;
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::PATH_COLON:
          switch (token)
          {
            case token::TokenType::ID:
              state = State::END;
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::VALUE:
        case State::ARR_RP:
        case State::SC_COLON:
        case State::PATH_ID_TARGET:
          break;
        case State::UNEXPECTED:
          throw error::Error(CZH_ERROR_LOCATION, __func__,
                             "Unexpected state can not match.");
        case State::END:
          if (token != token::TokenType::SEND && token != token::TokenType::FEND)
            throw error::Error(CZH_ERROR_LOCATION, __func__, "Unexpected end.");
          else
            reset();
          break;
        default:
          throw error::Error(CZH_ERROR_LOCATION, __func__, "Unexpected state.");
      }
    }
    
    bool good()
    {
      return state != State::UNEXPECTED;
    }
    
    bool end()
    {
      return state == State::END;
    }
    
    void reset()
    {
      state = State::INIT;
      last_state = State::UNEXPECTED;
    }
  };
  
  std::string get_string_from_file(const std::string &path)
  {
    std::ifstream file{path, std::ios::binary};
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
  }
  
  class NumberMatch
  {
  private:
    enum class State
    {
      INIT, INT, INT_DOT, SIGN, DOT, DOT_NO_INT, EXP, EXP_SIGN, EXP_INT, END, UNEXPECTED
    };
    enum class Token
    {
      INT, DOT, SIGN, EXP, UNEXPECTED, END
    };
    State state;
    bool _is_double;
  public:
    NumberMatch() : state(State::INIT), _is_double(false)
    {}
    
    bool match(const std::string &s)
    {
      for (auto &ch: s)
      {
        auto token = get_token(ch);
        if (token == Token::UNEXPECTED) return false;
        next(token);
        if (state == State::UNEXPECTED) return false;
      }
      next(get_token());
      if (state != State::END) return false;
      return true;
    }
    
    [[nodiscard]]bool has_dot() const
    {
      return _is_double;
    }
    
    void reset()
    {
      state = State::INIT;
      _is_double = false;
    }
  
  private:
    Token get_token(int ch = -1)
    {
      if (std::isdigit(ch))
        return Token::INT;
      else if (ch == '.')
      {
        _is_double = true;
        return Token::DOT;
      }
      else if (ch == 'e' || ch == 'E')
        return Token::EXP;
      else if (ch == '+' || ch == '-')
        return Token::SIGN;
      else if (ch == -1)
        return Token::END;
      return Token::UNEXPECTED;
    }
    
    void next(const Token &token)
    {
      switch (state)
      {
        case State::INIT:
          switch (token)
          {
            case Token::INT:
              state = State::INT;
              break;
            case Token::DOT:
              state = State::DOT_NO_INT;
              break;
            case Token::SIGN:
              state = State::SIGN;
              break;
            default:
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::SIGN:
          switch (token)
          {
            case Token::INT:
              state = State::INT;
              break;
            case Token::DOT:
              state = State::DOT_NO_INT;
              break;
            default:
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::INT:
          switch (token)
          {
            case Token::INT:
              break;
            case Token::DOT:
              state = State::DOT;
              break;
            case Token::EXP:
              state = State::EXP;
              break;
            case Token::END:
              state = State::END;
              break;
            default:
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::INT_DOT:
          switch (token)
          {
            case Token::INT:
              break;
            case Token::EXP:
              state = State::EXP;
              break;
            case Token::END:
              state = State::END;
              break;
            default:
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::DOT_NO_INT:
          switch (token)
          {
            case Token::INT:
              state = State::INT_DOT;
              break;
            default:
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::DOT:
          switch (token)
          {
            case Token::INT:
              state = State::INT_DOT;
              break;
            case Token::EXP:
              state = State::EXP;
              break;
            case Token::END:
              state = State::END;
              break;
            default:
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::EXP:
          switch (token)
          {
            case Token::INT:
              state = State::EXP_INT;
              break;
            case Token::SIGN:
              state = State::EXP_SIGN;
              break;
            default:
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::EXP_SIGN:
          switch (token)
          {
            case Token::INT:
              state = State::EXP_INT;
              break;
            default:
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::EXP_INT:
          switch (token)
          {
            case Token::INT:
              break;
            case Token::END:
              state = State::END;
              break;
            default:
              state = State::UNEXPECTED;
              break;
          }
          break;
        default:
          state = State::UNEXPECTED;
          break;
      }
    }
  };
  
  inline bool isnumber(const char &ch)
  {
    return std::isdigit(ch) || ch == '.' || ch == 'e' || ch == 'E' || ch == '+' || ch == '-';
  }
  
  class Lexer
  {
  private:
    std::shared_ptr<file::File> code;
    std::deque<token::Token> tokenstream;
    Match match;
    NumberMatch nmatch;
    token::Pos codepos;
    bool parsing_path;
    bool is_eof;
  public:
    Lexer()
        : code(nullptr),
          codepos(nullptr),
          parsing_path(false),
          is_eof(false)
    {}
    
    void set_czh(std::string filename, std::unique_ptr<std::ifstream> fs)
    {
      code = std::make_shared<file::StreamFile>(std::move(filename), std::move(fs));
      codepos = token::Pos(code);
    }
    
    void set_czh(const std::string &path, const std::string &filename)
    {
      code = std::make_shared<file::NonStreamFile>(filename, get_string_from_file(path));
      codepos = token::Pos(code);
    }
    
    void set_czh(std::string str)
    {
      code = std::make_shared<file::NonStreamFile>("czh from std::string", std::move(str));
      codepos = token::Pos(code);
    }
    
    token::Token view(std::size_t s)
    {
      if (tokenstream.size() <= s)
      {
        while (tokenstream.size() < 1024)
        {
          auto t = get_tok();
          check_token(t);
          tokenstream.emplace_back(t);
          if (t.type == token::TokenType::FEND)
          {
            is_eof = true;
            break;
          }
        }
      }
      return tokenstream[s];
    }
    
    void next(const std::size_t &s = 1)
    {
      tokenstream.pop_front();
    }
    
    [[nodiscard]]bool eof() const
    {
      return is_eof && tokenstream.empty();
    }
  
  private:
    void check_token(const token::Token &token)
    {
      if (token.type == token::TokenType::FEND)
      {
        if (match.get_state() == State::END || match.get_state() == State::INIT)
          return;
        else
          token.error("Unexpected end of file.");
      }
      if (match.end() && token.type != token::TokenType::SEND)
        match.match(token::TokenType::SEND);
      match.match(token.type);
      if (!match.good())
      {
        token.error("Unexpected token '" + token.get_string() + "'.Do you mean '"
                    + match.error_correct() + "'?");
      }
    }
    
    token::Pos get_pos()
    {
      return codepos;
    }
    
    token::Token get_tok()
    {
      static const std::map<char, token::TokenType> marks =
          {
              {'=', token::TokenType::EQUAL},
              {'[', token::TokenType::ARR_LP},
              {']', token::TokenType::ARR_RP},
              {':', token::TokenType::COLON},
              {'-', token::TokenType::BPATH},
              {';', token::TokenType::SEND},
              {',', token::TokenType::COMMA}
          };
      
      while (check_char() && isspace(view_char()))
        next_char();
      
      bool is_num = false;
      if (!parsing_path && check_char() && (std::isdigit(view_char()) || view_char() == '.' || view_char() == '+' ||
                                            view_char() == '-'))
      {
        is_num = true;
        if (view_char() == '-')
        {
          if (check_char(1) && !(std::isdigit(view_char(1)) || view_char(1) == '.'))
            is_num = false; //-id
          if (check_char(2) && view_char(1) == '.' && view_char(2) == '.')
            is_num = false; //-..
          if (check_char(2) && view_char(1) == '.' && view_char(2) == '-')
            is_num = false;//-.-
          if (check_char(2) && view_char(1) == '.' && view_char(2) == ':')
            is_num = false;//-.:
        }
      }
      
      if (is_num)
      {
        std::string temp;
        do
        {
          temp += view_char();
          next_char();
        } while (check_char() && isnumber(view_char()));
        if (nmatch.match(temp))
        {
          if (nmatch.has_dot())
          {
            nmatch.reset();
            return {token::TokenType::DOUBLE, utils::str_to_num(temp),
                    get_pos().set_size(temp.size())};
          }
          else
          {
            nmatch.reset();
            auto t = utils::str_to_num(temp);
            if (static_cast<double>(static_cast<long long>(t)) != t)//like -6e-2
            {
              return {token::TokenType::DOUBLE, t,
                      get_pos().set_size(temp.size())};
            }
            else
            {
              if (t < std::numeric_limits<int>::max())
                return {token::TokenType::INT, (int) t, get_pos().set_size(temp.size())};
              else
                return {token::TokenType::LONGLONG, (long long) t,
                        get_pos().set_size(temp.size())};
            }
          }
        }
        else
        {
          token::Token tmp(token::TokenType::UNEXPECTED, 0, get_pos().set_size(temp.size()));
          tmp.error("Unexpected token '" + temp + "'.Is this a number?");
        }
      }
      else if (check_char() && view_char() == '"')//str
      {
        std::string temp;
        next_char();//eat '"'.
        while (check_char() && view_char() != '"')
        {
          temp += view_char();
          next_char();
        }
        next_char();//eat '"'
        return {token::TokenType::STRING, temp, get_pos().set_size(temp.size())};
      }
      else if ((check_char() && (isalpha(view_char()) || view_char() == '_'))
               || (check_char() && parsing_path && view_char() == '.'))//id
      {
        std::string temp;
        if (parsing_path && view_char() == '.')
        {
          temp = ".";
          next_char();
          if (check_char() && view_char() == '.')
          {
            temp += ".";
            next_char();
          }
        }
        while (check_char() && (isalnum(view_char()) || view_char() == '_') && view_char() != '-')
        {
          temp += view_char();
          next_char();
        }
        
        if (temp == "end")
          return {token::TokenType::SCEND, temp, get_pos().set_size(3)};
        else if (temp == "true")
          return {token::TokenType::BOOL, true, get_pos().set_size(4)};
        else if (temp == "false")
          return {token::TokenType::BOOL, false, get_pos().set_size(5)};
        else
          return {token::TokenType::ID, temp, get_pos().set_size(temp.size())};
      }
      else if (check_char() && marks.find(view_char()) != marks.end())//mark
      {
        next_char();
        if (marks.at(view_char(-1)) == token::TokenType::BPATH) parsing_path = true;
        if (parsing_path && marks.at(view_char(-1)) == token::TokenType::COLON) parsing_path = false;
        return {marks.at(view_char(-1)), view_char(-1), get_pos().set_size(1)};
      }
      else if (check_char(2) && view_char() == '/' && view_char(1) == 'b' && view_char(2) == '/')//note
      {
        std::string temp;
        next_char(3);//eat '/b/'
        while (!(check_char(2) && view_char() == '/' && view_char(1) == 'e' && view_char(2) == '/'))
        {
          temp += view_char();
          next_char();
        }
        next_char(3);//eat '/e/'
        return {token::TokenType::NOTE, value::Note(temp),
                get_pos().set_size(temp.size())};
      }
      else if (!check_char()) return {token::TokenType::FEND, 0, get_pos().set_size(0)};
      else
      {
        token::Token(token::TokenType::UNEXPECTED, 0, get_pos().set_size(0))
            .error(std::string("Unexpected token '" + std::string(1, view_char()) + "'."));
      }
      return {token::TokenType::UNEXPECTED, 0, get_pos().set_size(0)};
    }
    
    bool check_char(std::size_t s = 0)
    {
      return code->check(s);
    }
    
    char view_char(int s = 0)
    {
      return code->view(s);
    }
    
    void next_char(std::size_t s = 1)
    {
      code->ignore(s);
      codepos.pos += s;
    }
  };
}