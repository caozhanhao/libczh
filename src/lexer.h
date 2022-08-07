#pragma once

#include "value.h"
#include "err.h"

#include <memory>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <set>
#include <map>
#include <algorithm>
#include <cstdio>
using czh::error::Error;
using czh::value::Value;
namespace czh::lexer
{
  enum class TokenType
  {
    ID,
    INT, DOUBLE, STRING, BOOL,
    EQUAL,//=
    ARR_LP, ARR_RP,//[]
    COMMA, COLON,
    BPATH,//-
    FEND, SEND, SCEND,
    NOTE,
    UNEXPECTED
  };
  
  const std::map<char, TokenType> marks =
      {
          {'=', TokenType::EQUAL},
          {'[', TokenType::ARR_LP},
          {']', TokenType::ARR_RP},
          {':', TokenType::COLON},
          {'-', TokenType::BPATH},
          {';', TokenType::SEND},
          {',', TokenType::COMMA}
      };
  
  const std::map<TokenType, std::string> means =
      {
          {TokenType::ID,               "identifier"},
          {TokenType::INT,           "int"},
          {TokenType::DOUBLE,        "double"},
          {TokenType::STRING,        "string"},
          {TokenType::BOOL,          "bool"},
          {TokenType::EQUAL,         "="},
          {TokenType::ARR_LP,        "["},
          {TokenType::ARR_RP,        "]"},
          {TokenType::ARR_RP,        "]"},
          {TokenType::FEND,       "end of file"},
          {TokenType::SEND,       ";"},
          {TokenType::COMMA,      ","},
          {TokenType::COLON,      ":"},
          {TokenType::BPATH,      "-"},
          
          {TokenType::UNEXPECTED, "unexpected token"},
          {TokenType::SEND,       "sentence end"},
          {TokenType::SCEND,      "scope end"},
          {TokenType::NOTE, "note"}
      };
  inline std::string get_mean(const TokenType &t)
  {
    if (means.find(t) == means.end())
      throw Error(CZH_ERROR_LOCATION, __func__, "unexpected error mean", Error::internal);
    return means.at(t);
  }
  inline bool is_newline_and_next(const std::string &str, std::size_t &pos, int delta = 1)
  {
    if (str[pos] == '\r')
    {
      if (pos + 1 < str.size() && str[pos + 1] == '\n')
      {
        pos += 2 * delta;
        return true;
      } else
        pos += 1 * delta;
      return true;
    } else if (str[pos] == '\n')
    {
      pos += 1 * delta;
      return true;
    }
    return false;
  }
  
  class File
  {
  public:
    std::string filename;
    std::string code;
  public:
    File(std::string name, std::string code_)
        : filename(std::move(name)), code(std::move(code_)) {}
    
    [[nodiscard]] std::string getline(std::size_t beg, std::size_t end, std::size_t linenosize = 0) const
    {
      std::string ret;
      if (linenosize == 0)
        linenosize = std::to_string(end).size();
      std::size_t lineno = 1;
      bool first_line_flag = false;
      auto add = [&]()
      {
        std::string addition = std::to_string(lineno);
        if (addition.size() < linenosize)
          ret += std::string(linenosize - addition.size(), '0');
        ret += addition + " ";
      };
      for (std::size_t i = 0; i < code.size();)
      {
        if (lineno >= beg && lineno < end)
        {
          if (!first_line_flag && lineno == 1)
          {
            add();
            first_line_flag = true;
          }
          if (code[i] != '\r' && code[i] != '\n')
            ret += code[i];
        }
        if (is_newline_and_next(code, i))
        {
          lineno++;
          if (lineno >= beg && lineno < end)
          {
            ret += '\n';
            add();
          }
        } else
          i++;
      }
      return ret;
    }
  
    [[nodiscard]] std::size_t get_lineno(std::size_t pos) const
    {
      std::size_t lineno = 1;
      for (std::size_t i = 0; i < pos;)
      {
        if (is_newline_and_next(code, i))
          lineno++;
        else
          i++;
      }
      return lineno;
    }
  
    [[nodiscard]] std::size_t get_error_size(std::size_t pos) const
    {
      std::size_t i = pos;
      while (i > 0)
      {
        if (is_newline_and_next(code, i, -1))
          break;
        else i--;
      }
      if (code[i] == '\r' && i + 1 < code.size() && code[i + 1] == '\n')
        return pos - i - 2;
      return pos - i - 1;
    }
  
    [[nodiscard]] std::string get_name() const
    {
      return filename;
    }
    
    void set_code(const std::string &name, const std::string &code_)
    {
      filename = name;
      code = code_;
    }
    
    void reset()
    {
      filename = "";
      code = "";
    }
  
    [[nodiscard]] std::size_t size() const
    {
      return code.size();
    }
    
    char &operator[](std::size_t pos)
    {
      return code[pos];
    }
  };
  
  class Pos
  {
  public:
    std::size_t pos;
    std::size_t size;
    std::shared_ptr<File> code;
  public:
    explicit Pos(std::shared_ptr<File> code_)
        : pos(0), size(0), code(std::move(code_)) {}
    
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
      return (code->get_name() + ":line " + std::to_string(code->get_lineno(pos)));
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
      const std::size_t last = 3;
      const std::size_t next = 3;
      std::size_t lineno = code->get_lineno(pos);
      std::size_t linenosize = std::to_string(lineno + next).size();
      std::size_t actual_last = last;
      std::size_t actual_next = next;
      if (lineno - 1 < last)
        actual_last = lineno - 1;
      if (code->get_lineno(code->size() - 1) < lineno + next - 1)
        actual_next = lineno - 1;
      std::string temp1 = code->getline(lineno - actual_last, lineno + 1, linenosize);//[beg, end)
      std::string arrow = "\n" + std::string(code->get_error_size(pos) - size + linenosize + 1, ' ') + "\033[0;32;32m";
      for (std::size_t i = 0; i < size; i++)
        arrow += "^";
      arrow += "\033[m";
      std::string temp2 = code->getline(lineno + 1, lineno + actual_next + 1, linenosize);
      std::string errorstring = temp1 + arrow + temp2;
      return std::move(std::make_unique<std::string>(errorstring));
    }
    
    void reset()
    {
      pos = 0;
      size = 0;
    }
  };
  
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
    
    void error(const std::string &details) const
    {
      throw Error(pos.location(), __func__, details + ": \n"
                                            + *(pos.get_details_from_code()));
    }
    
    [[nodiscard]] std::string get_string() const
    {
      return pos.code->code.substr(pos.pos - pos.size, pos.size);
    }
  };
//  enum class TokenType
//  {
//    ID,
//    INT, DOUBLE, STRING, BOOL,
//    EQUAL,//=
//    LP, RP, //()
//    ARR_LP, ARR_RP,//[]
//    COMMA, SC_COLON,
//    BPATH,//-
//    
//    FEND, SEND, SCEND,
//    
//    NOTE,
//    //The following is not token
//    UNEXPECTED, ME, MB//many begin
//  };
//  
//  enum class State
//  {
//    ID, VALUE, EQUAL, LP, RP, ARR_LP, ARR_RP,
//    COMMA, SC_COLON, BPATH, FEND, SEND, SCEND,
//    UNEXPECTED, END
//  };
  
  enum class State
  {
    INIT,
    ID, VALUE,
    ARR_VALUE,EQUAL, ARR_LP, ARR_RP,
    COMMA, SC_COLON, PATH_COLON, BPATH, PATH_ID_TARGET, PATH_ID,
    UNEXPECTED
  };
  class Match
  {
  private:
    State state;
    State last_state;
  public:
    Match(): state(State::INIT), last_state(State::UNEXPECTED) {}
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
          return "'[' or ','";
        case State::COMMA:
          return "value";
        case State::PATH_ID:
          return "'-' or ':'";
        default:
          throw error::Error(CZH_ERROR_LOCATION, __func__, "Unexpected state.");
      }
    }
    void match(const Token& token)
    {
      if(token.type == TokenType::NOTE) return;
      switch (state)
      {
        case State::INIT:
          switch (token.type)
          {
            case TokenType::ID:
              state = State::ID;
              break;
            case TokenType::SCEND:
              reset();
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::ID:
          switch (token.type)
          {
            case TokenType::EQUAL:
              state = State::EQUAL;
              break;
            case TokenType::COLON:
              reset();
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::EQUAL:
          switch (token.type)
          {
            case TokenType::INT:
            case TokenType::DOUBLE:
            case TokenType::STRING:
            case TokenType::BOOL:
              reset();
              break;
            case TokenType::ARR_LP:
              state = State::ARR_LP;
              break;
            case TokenType::BPATH:
              state = State::BPATH;
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::ARR_LP:
          switch (token.type)
          {
            case TokenType::INT:
            case TokenType::DOUBLE:
            case TokenType::STRING:
            case TokenType::BOOL:
              state = State::ARR_VALUE;
              break;
            case TokenType::ARR_RP:
              reset();
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::ARR_VALUE:
          switch (token.type)
          {
            case TokenType::COMMA:
              state = State::COMMA;
              break;
            case TokenType::ARR_RP:
              reset();
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::COMMA:
          switch (token.type)
          {
            case TokenType::INT:
            case TokenType::DOUBLE:
            case TokenType::STRING:
            case TokenType::BOOL:
              state = State::ARR_VALUE;
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::BPATH:
          switch (token.type)
          {
            case TokenType::ID:
              state = State::PATH_ID;
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::PATH_ID:
          switch (token.type)
          {
            case TokenType::BPATH:
              state = State::BPATH;
              break;
            case TokenType::COLON:
              state = State::PATH_COLON;
              break;
            default:
              last_state = state;
              state = State::UNEXPECTED;
              break;
          }
          break;
        case State::PATH_COLON:
          switch (token.type)
          {
            case TokenType::ID:
              reset();
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
        default:
          throw error::Error(CZH_ERROR_LOCATION, __func__, "Unexpected state.");
      }
    }
    bool good()
    {
      return state != State::UNEXPECTED;
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
    NumberMatch() : state(State::INIT), _is_double(false) {}
    bool match(const std::string& s)
    {
      for(auto& ch : s)
      {
        auto token = get_token(ch);
        if(token == Token::UNEXPECTED) return false;
        next(token);
        if(state == State::UNEXPECTED) return false;
      }
      next(get_token());
      if(state != State::END) return false;
      return true;
    }
    bool is_double() const
    {
      return _is_double;
    }
  void reset()
  {
    state = State::INIT;
    _is_double = false;
  }
  private:
    Token get_token(char ch = -1)
    {
      if(std::isdigit(ch))
        return Token::INT;
      else if(ch == '.')
      {
        _is_double = true;
        return Token::DOT;
      }
      else if(ch == 'e' || ch == 'E')
        return Token::EXP;
      else if(ch == '+' || ch == '-')
        return Token::SIGN;
      else if(ch == -1)
        return Token::END;
      return Token::UNEXPECTED;
    }
    void next(const Token& token)
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
        case State::END:
          state = State::UNEXPECTED;
          break;
      }
    }
  };
  inline bool isnumber(const char& ch)
  {
    return std::isdigit(ch) || ch == '.' || ch == 'e' || ch == 'E' || ch == '+' || ch == '-';
  }
  class Lexer
  {
  private:
    std::shared_ptr<File> code;
    std::size_t code_size;
    std::shared_ptr<std::vector<Token>> tokens;
    Match match;
    NumberMatch nmatch;
    Pos codepos;
    bool parsing_path;
  public:
    Lexer(const std::string &path, const std::string &_filename)
        : code(std::make_shared<File>(_filename, get_string_from_file(path))),
          code_size(code->size()),
          tokens(std::make_shared<std::vector<Token>>(std::vector<Token>())),
          codepos(code),
          parsing_path(false) {}
    
    explicit Lexer(std::string code_str)
        : code(std::make_shared<File>("temp", std::move(code_str))),
          code_size(code->size()),
          tokens(std::make_shared<std::vector<Token>>(std::vector<Token>())),
          codepos(code),
          parsing_path(false) {}
    
    std::shared_ptr<std::vector<Token>> get_all_token()
    {
      Token t(TokenType::UNEXPECTED, 0, get_pos().set_size(0));
      do
      {
        t = get_tok();
        if(t.type == TokenType::SEND || t.type == TokenType::FEND) continue;
        check(t);
      } while (t.type != TokenType::FEND);
      return tokens;
    }
  
  private:
    void check(const Token &token)
    {
      match.match(token);
      if(!match.good())
      {
        token.error("Unexpected token '" + token.get_string()+ "'.Do you mean '"
                    + match.error_correct() + "'?");
        return;
      }
      tokens->emplace_back(token);
    }
    Token get_tok()
    {
      while (check() && isspace(get()))
        next();
      
      bool is_num = false;
      if (!parsing_path && check() && (std::isdigit(get()) || get() == '.' || get() == '+' || get() == '-'))
      {
        is_num = true;
        if(get() == '-')
        {
          if (check(1) && !(std::isdigit(get(1)) || get(1) == '.'))
            is_num = false; //-id
          if (check(2) && get(1) == '.' && get(2) == '.')
            is_num = false; //-..
          if (check(2) && get(1) == '.' && get(2) == '-')
            is_num = false;//-.-
          if (check(2) && get(1) == '.' && get(2) == ':')
            is_num = false;//-.:
        }
      }
      
      if(is_num)
      {
        std::string temp;
        do
        {
          temp += get();
          next();
        } while (check() && isnumber(get()));
        if(nmatch.match(temp))
        {
          if (nmatch.is_double())
          {
            nmatch.reset();
            return {TokenType::DOUBLE, std::stod(temp), get_pos().set_size(temp.size())};
          }
          else
          {
            nmatch.reset();
            return {TokenType::INT, std::stoi(temp), get_pos().set_size(temp.size())};
          }
        }
        else
        {
          Token tmp(TokenType::UNEXPECTED, 0, get_pos().set_size(temp.size()));
          tmp.error("Unexpected token '" + temp + "'.Is this a number?");
        }
      }
      else if (check() && get() == '"')//str
      {
        std::string temp;
        next();//eat '"'.
        while (check() && get() != '"')
        {
          temp += get();
          next();
        }
        next();//eat '"'
        return {TokenType::STRING, temp, get_pos().set_size(temp.size())};
      } else if ((check() && (isalpha(get()) || get() == '_')) || (check() && parsing_path && get() == '.'))//id
        //pathʱ'.''..'Ϊid
      {
        std::string temp = "";
        if (parsing_path && get() == '.')
        {
          temp = ".";
          next();
          if (check() && get() == '.')
          {
            temp += ".";
            next();
          }
        }
        while (check() && (isalnum(get()) || get() == '_') && get() != '-')
        {
          temp += get();
          next();
        }
        
        if (temp == "end")
          return {TokenType::SCEND, temp, get_pos().set_size(3)};
        else if (temp == "true")
          return {TokenType::BOOL, true, get_pos().set_size(4)};
        else if (temp == "false")
          return {TokenType::BOOL, false, get_pos().set_size(5)};
        else
          return {TokenType::ID, temp, get_pos().set_size(temp.size())};
      } else if (check() && marks.find(get()) != marks.end())//mark
      {
        next();
        if (marks.at(get(-1)) == TokenType::BPATH) parsing_path = true;
        if (parsing_path && marks.at(get(-1)) == TokenType::COLON) parsing_path = false;
        return Token(marks.at(get(-1)), get(-1), get_pos().set_size(1));
      } else if (check(2) && get() == '/' && get(1) == 'b' && get(2) == '/')//note
      {
        std::string temp;
        next(3);//eat '/b/'
        while (!(check(2) && get() == '/' && get(1) == 'e' && get(2) == '/'))
        {
          temp += get();
          next();
        }
        next(3);//eat '/e/'
        return Token(TokenType::NOTE, value::Note(temp), get_pos().set_size(temp.size()));
      } else if (codepos.get() == code->size()) return Token(TokenType::FEND, 0, get_pos().set_size(0));
      else
      {
        Token(TokenType::UNEXPECTED, 0, get_pos().set_size(0))
            .error(std::string("Unexpected token '" + std::string(1, get()) + "'."));
      }
      return {TokenType::UNEXPECTED, 0, get_pos().set_size(0)};
    }
    
    Pos get_pos()
    {
      return codepos;
    }
    
    bool check(const std::size_t &s = 0)
    {
      return (codepos.get() + s) < code_size;
    }
    
    char &get(const std::size_t &s = 0)
    {
      if (!check(s))
        Token(TokenType::UNEXPECTED, 0, get_pos().set_size(0)).error(std::string("Unexpected end of tokens.'"));
      return (*code)[codepos.get() + s];
    }
    
    void next(const std::size_t &s = 1)
    {
      codepos += s;
    }
  };
}