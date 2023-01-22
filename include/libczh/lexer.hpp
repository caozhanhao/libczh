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
#ifndef LIBCZH_LEXER_HPP
#define LIBCZH_LEXER_HPP
#pragma once

#include "value.hpp"
#include "token.hpp"
#include "file.hpp"
#include "error.hpp"
#include "utils.hpp"

#include <memory>
#include <limits>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

namespace czh::lexer
{
  enum class State
  {
    INIT,
    ID, VALUE,
    ARR_VALUE, EQUAL, ARR_LP, ARR_RP,
    COMMA, SC_COLON, REF, REF_ID,
    UNEXPECTED, END
  };
  
  class Match
  {
  private:
    State state;
    State last_state;
  public:
    Match() : state(State::INIT), last_state(State::UNEXPECTED) {}
    
    std::string error_correct()
    {
      switch (last_state)
      {
        case State::REF:
        case State::INIT:
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
        case State::REF_ID:
          return "'::'";
        default:
          error::czh_unreachable("Unexpected token");
      }
      error::czh_unreachable("Unexpected token");
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
            case token::TokenType::VALUE:
              state = State::END;
              break;
            case token::TokenType::ARR_LP:
              state = State::ARR_LP;
              break;
            case token::TokenType::REF:
              state = State::REF;
              break;
            case token::TokenType::ID:
              state = State::REF_ID;
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
            case token::TokenType::VALUE:
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
            case token::TokenType::VALUE:
              state = State::ARR_VALUE;
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::REF:
          switch (token)
          {
            case token::TokenType::ID:
              state = State::REF_ID;
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::REF_ID:
          switch (token)
          {
            case token::TokenType::REF:
              state = State::REF;
              break;
            case token::TokenType::SEND:
            case token::TokenType::FEND:
              reset();
              break;
            default:
              reset();
              match(token);
              break;
          }
          break;
        case State::VALUE:
        case State::ARR_RP:
        case State::SC_COLON:
        case State::UNEXPECTED:
          error::czh_unreachable();
        case State::END:
          error::czh_assert(token == token::TokenType::SEND || token == token::TokenType::FEND,
                            "Unexpected end of file.");
          reset();
          break;
        default:
          error::czh_unreachable();
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
    error::czh_assert(file.good(), error::czh_invalid_file);
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
    NumberMatch() : state(State::INIT), _is_double(false) {}
    
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
  
  class Lexer
  {
  private:
    std::shared_ptr<file::File> code;
    Match match;
    NumberMatch nmatch;
    token::Pos codepos;
    token::Token buffer;
    bool is_eof;
    char ch;
  public:
    Lexer()
        : code(nullptr),
          codepos(nullptr),
          is_eof(false),
          ch(0),
          buffer(token::TokenType::UNEXPECTED, 0, codepos) {}
    
    void set_czh(std::string filename, std::unique_ptr<std::ifstream> fs)
    {
      error::czh_assert(fs->good(), error::czh_invalid_file);
      code = std::make_shared<file::StreamFile>(std::move(filename), std::move(fs));
      codepos = token::Pos(code);
      ch = get_char();
    }
  
    void set_czh(const std::string &path, const std::string &filename)
    {
      code = std::make_shared<file::NonStreamFile>(filename, get_string_from_file(path));
      codepos = token::Pos(code);
      ch = get_char();
    }
    
    void set_czh(std::string str)
    {
      code = std::make_shared<file::NonStreamFile>("czh from std::string", std::move(str));
      codepos = token::Pos(code);
      ch = get_char();
    }
  
    token::Token get()
    {
      token::Token t = std::move(buffer);
      if (t.type == token::TokenType::UNEXPECTED)
      {
        t = std::move(get_tok());
        check_token(t);
        is_eof = (t.type == token::TokenType::FEND);
      }
      buffer = std::move(get_tok());
      check_token(buffer);
      is_eof = (buffer.type == token::TokenType::FEND);
      return std::move(t);
    }
  
    token::Token &peek()
    {
      return buffer;
    }
    
    [[nodiscard]]bool eof() const
    {
      return is_eof;
    }
  
  private:
    void check_token(const token::Token &token)
    {
      if (token.type == token::TokenType::FEND)
      {
        if (match.get_state() == State::END || match.get_state() == State::INIT)
          return;
        else
          token.report_error("Unexpected end of file.");
      }
      if (match.end() && token.type != token::TokenType::SEND)
        match.match(token::TokenType::SEND);
      match.match(token.type);
      if (!match.good())
      {
        token.report_error("Unexpected token '" + token.to_string() + "'.Do you mean '"
                           + match.error_correct() + "'?");
      }
    }
  
    token::Pos get_pos()
    {
      return codepos;
    }
  
    [[nodiscard]]std::size_t get_char_num() const
    {
      if ((ch & 0x80) == 0x00) return 1;
      if ((ch & 0xE0) == 0xC0) return 2;
      if ((ch & 0xF0) == 0xE0) return 3;
      if ((ch & 0xF8) == 0xF0) return 4;
      return 0;
    }
  
    void skip()
    {
      while (check_char() && get_char_num() == 1 && isspace(ch))
      {
        ch = get_char();
      }
      while (check_char() && ch == '<')
      {
        int notes = 0;
        auto bak = get_pos().set_size(1);
        ch = get_char();
        while (check_char() && !(ch == '>' && notes == 0))
        {
          if (ch == '<') ++notes;
          if (ch == '>') --notes;
          ch = get_char();
        }
        if (notes != 0 && !check_char())
        {
          token::Token tmp(token::TokenType::UNEXPECTED, static_cast<int>('<'), bak);
          tmp.report_error("Unexpected note begin.");
        }
        if (ch == '>')
        {
          ch = get_char();
        }
      }
    }
  
    token::Token get_tok()
    {
      static std::map<int, token::TokenType> marks =
          {
              {'=', token::TokenType::EQUAL},
              {'{', token::TokenType::ARR_LP},
              {'}', token::TokenType::ARR_RP},
              {':', token::TokenType::COLON},
              {';', token::TokenType::SEND},
              {',', token::TokenType::COMMA}
          };
      //space and note
      while (check_char() && ((get_char_num() == 1 && isspace(ch)) || ch == '<'))
      {
        skip();
      }
      //num
      if (get_char_num() == 1 && (std::isdigit(ch) || ch == '.'
                                  || ch == '+' || ch == '-'))
      {
        std::string temp;
        do
        {
          temp += ch;
          ch = get_char();
        } while (check_char() && (std::isdigit(ch)
                                  || ch == '.' || ch == 'e' || ch == 'E'
                                  || ch == '+' || ch == '-'));
        if (nmatch.match(temp))
        {
          if (nmatch.has_dot())
          {
            nmatch.reset();
            return {token::TokenType::VALUE, utils::str_to_num(temp),
                    get_pos().set_size(temp.size())};
          }
          else
          {
            nmatch.reset();
            auto t = utils::str_to_num(temp);
            if (static_cast<double>(static_cast<long long>(t)) != t)//like -6e-2
            {
              return {token::TokenType::VALUE, t,
                      get_pos().set_size(temp.size())};
            }
            else
            {
              if (t < std::numeric_limits<int>::max())
              {
                return {token::TokenType::VALUE, static_cast<int>(t), get_pos().set_size(temp.size())};
              }
              else
              {
                return {token::TokenType::VALUE, static_cast<long long>(t),
                        get_pos().set_size(temp.size())};
              }
            }
          }
        }
        else
        {
          token::Token tmp(token::TokenType::UNEXPECTED, 0, get_pos().set_size(temp.size()));
          tmp.report_error("Unexpected token '" + temp + "'. Is this a number?");
        }
      }
        //string
      else if (ch == '"')
      {
        std::string temp;
        ch = get_char();
        while (check_char() && ch != '"')
        {
          temp += ch;
          ch = get_char();
        }
        ch = get_char();
        return {token::TokenType::VALUE, temp, get_pos().set_size(temp.size())};
      }
        //id = ...
      else if (get_char_num() > 1 || isalpha(ch) || ch == '_')
      {
        std::string temp;
        while (check_char() && (get_char_num() > 1 || isalnum(ch) || ch == '_'))
        {
          auto nchar = get_char_num();
          if (nchar == 1)
          {
            if (!std::isalnum(ch) && ch != '_') continue;
            temp += ch;
            ch = get_char();
          }
          else
          {
            temp += ch;
            for (int i = 0; i < nchar - 1; ++i)
            {
              temp += ch = get_char();
            }
            ch = get_char();
          }
        }
  
        if (temp == "end")
          return {token::TokenType::SCEND, temp, get_pos().set_size(3)};
        else if (temp == "true")
          return {token::TokenType::VALUE, true, get_pos().set_size(4)};
        else if (temp == "false")
        {
          return {token::TokenType::VALUE, false, get_pos().set_size(5)};
        }
        else if (temp == "null")
        {
          return {token::TokenType::VALUE, value::Null(), get_pos().set_size(4)};
        }
        else
        {
          return {token::TokenType::ID, temp, get_pos().set_size(temp.size())};
        }
      }
        //marks
      else if (marks.find(ch) != marks.end())
      {
        char bak = ch;
        ch = get_char();
        if (bak == ':' && ch == ':')
        {
          ch = get_char();
          return {token::TokenType::REF, "::", get_pos().set_size(2)};
        }
        return {marks[bak], static_cast<int>(bak), get_pos().set_size(1)};
      }
        //end
      else if (!check_char()) return {token::TokenType::FEND, 0, get_pos().set_size(1)};
      else
      {
        char bak = get_char();
        token::Token(token::TokenType::UNEXPECTED, 0, get_pos().set_size(1))
            .report_error(std::string("Unexpected token '" + std::string(1, bak) + "'."));
      }
      return {token::TokenType::UNEXPECTED, 0, get_pos().set_size(1)};
    }
  
    bool check_char()
    {
      return code->check();
    }
  
    char get_char()
    {
      ++codepos.pos;
      return code->get();
    }
  };
}
#endif