#pragma once

#include "value.h"
#include "err.h"

#include <memory>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <cstdio>
using czh::error::Err;
using czh::value::Value;
namespace czh
{
  namespace lexer
  {
    enum class Type
    {
      ID_TOK,
      INT_TOK, DOUBLE_TOK, STRING_TOK,
      EQUAL_TOK,//=
      LPAREN_TOK, RPAREN_TOK, //()
      ARR_LPAREN_TOK, ARR_RPAREN_TOK,//[]
      COMMA_TOK, COLON_TOK,
      BPATH_TOK,//-

      FILE_END_TOK, SENTENCE_END_TOK, SCOPE_END_TOK,

      NOTE_TOK,
      //The following is not token
      UNEXPECTED, ME, MB//many begin
    };

    std::map<char, Type> marks =
    {
      {'=', Type::EQUAL_TOK},
      {'(', Type::LPAREN_TOK},
      {')', Type::RPAREN_TOK},
      {'[', Type::ARR_LPAREN_TOK},
      {']', Type::ARR_RPAREN_TOK},
      {':', Type::COLON_TOK},
      {'-', Type::BPATH_TOK},
      {';', Type::SENTENCE_END_TOK},
      {',', Type::COMMA_TOK}
    };

    std::map<Type, std::string> means =
    {
      {Type::ID_TOK, "identifier"},
      {Type::INT_TOK, "int"},
      {Type::DOUBLE_TOK, "double"},
      {Type::STRING_TOK, "string"},
      {Type::EQUAL_TOK, "="},
      {Type::LPAREN_TOK, "("},
      {Type::RPAREN_TOK, ")"},
      {Type::ARR_LPAREN_TOK, "["},
      {Type::ARR_RPAREN_TOK, "]"},
      {Type::FILE_END_TOK, "end of file"},
      {Type::SENTENCE_END_TOK, ";"}, 
      {Type::COMMA_TOK, ","}, 
      {Type::COLON_TOK, ":"}, 
      {Type::BPATH_TOK, "-"}, 

      {Type::UNEXPECTED, "unexpected token"},
      {Type::MB, "many begin"},
      {Type::ME, "many end"},
      {Type::SENTENCE_END_TOK, "sentence end"},
      {Type::SCOPE_END_TOK, "scope end"},
      {Type::NOTE_TOK, "note"}
    };

    inline std::string get_mean(const Type& t)
    {
      if (means.find(t) == means.end())
        throw Err(CZH_ERROR_LOCATION, __func__, "unexpected error mean",  error::internal);
      return means[t];
    }
    inline bool  is_end(const Type& t)
    {
      return (t == Type::COLON_TOK || t == Type::SENTENCE_END_TOK || t == Type::FILE_END_TOK);
    }

    inline bool is_newline_and_next(const std::string& str, std::size_t& pos, int delta = 1)
    {
      if (str[pos] == '\r')
      {
        if (pos + 1 < str.size() && str[pos + 1] == '\n')
        {
          pos += 2 * delta;
          return true;
        }
        else
          pos += 1 * delta;
        return true;
      }
      else if (str[pos] == '\n')
      {
        pos += 1 * delta;
        return true;
      }
      return false;
    }
    class File
    {
    private:
      std::string filename;
      std::string code;
    public:
      File(const std::string& name, const std::string& code_)
        :filename(name), code(code_) {}
      std::string getline(std::size_t beg, std::size_t end, std::size_t linenosize = 0) const
      {
        std::string ret;
        if(linenosize == 0)
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
            if(code[i] != '\r' && code[i] != '\n')
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
          }
          else
            i++;
        }
        return ret;
      }
      std::size_t get_lineno(std::size_t pos) const
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
      std::size_t get_error_size(std::size_t pos) const
      {
        std::size_t i = pos;
        while (i > 0)
        {
          if (is_newline_and_next(code, i, -1))
            break;
          else i--;
        }
        auto a = code[i];
        if (code[i] == '\r' && i + 1 < code.size() && code[i + 1] == '\n')
          return pos - i - 2;
        return pos - i - 1;
      }
      std::string get_name() const
      {
        return filename;
      }
      void set_file(const std::string& name, const std::string& code_)
      {
        filename = name; 
        code = code_;
      }
      void reset()
      {
        filename = "";
        code = "";
      }
      std::size_t size() const
      {
        return code.size();
      }
      char& operator[](std::size_t pos)
      {
        return code[pos];
      }
    };
    class Pos
    {
    private:
      std::size_t pos;
      std::size_t size;
      std::shared_ptr<File> code;
    public:
      Pos(const std::shared_ptr<File>& _code)
        :pos(0), code(_code), size(0) 
      {  }
      operator std::size_t() { return pos; }
      Pos& operator+=(const std::size_t& p)
      {
        pos += p;
        return *this;
      }
      Pos& operator-=(const std::size_t& p)
      {
        pos -= p;
        return *this;
      }
      std::string location() const
      {
        return (code->get_name() + ":line " + std::to_string(code->get_lineno(pos)));
      }
    public:
      Pos& set_size(std::size_t s)
      {
        size = s;
        return *this;
      }

      std::unique_ptr<std::string> get_details_from_code() const
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
        std::string arrow = "\n" + std::string(code->get_error_size(pos) - size + linenosize + 1, ' ') +"\033[0;32;32m";
        for (auto i = 0; i < size; i++)
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
      Type type;
      Value what;
      Pos pos;
    public:
      template <typename T>
      Token(Type _type, const T& _what, const Pos& _pos)
        :type(_type), what(_what), pos(_pos) {  }

      void error(const std::string& details) const
      {
        throw Err(pos.location(), __func__, details + ": \n" + *(pos.get_details_from_code()));
      }
    };

    using Rule = std::vector<Type>;

    template<typename T>
    class MatchesHelper
    {
    private:
      std::vector<std::shared_ptr<T>> vec;
    public:
      T* find(const T& i) const
      {
        auto r = std::find_if(vec.cbegin(), vec.cend(),
          [&](const std::shared_ptr<T>& p1)
          {
            return *p1 == i;
          });
        if (r != vec.cend())
          return r->get();
        return nullptr;
      }
      bool empty() const
      {
        return vec.empty();
      }
      T* insert(const T& i)
      {
        auto p = find(i.get_type());
        if (p != nullptr)
          return p;
        vec.emplace_back(std::make_shared<T>(i));
        return vec[vec.size() - 1].get();
      }
      auto begin() const { return vec.begin(); }
      auto end() const { return vec.end(); }

      Type guess_if_forget(Type t) const
      {
        for (auto& r : vec)
        {
          if (r->match(t) != nullptr)
            return r->get_type();
        }
        return Type::UNEXPECTED;
      }
      Type guess_one() const
      {
        return vec[0]->get_type();
      }
    };
    class Match 
    {
      //friend void debug_check_match(const Match& m);
      friend Match make_match(const std::vector<Rule>& rule_vec);
      friend bool operator==(const Match& m1, const Match& m2);
      friend class MatchesHelper<Match>;
    private:
      MatchesHelper<Match> matches;
      Match* last_match;
      Match* many;
      Type type;
    public:
      Match(const Type& t, Match* last = nullptr)
        :type(t), many(nullptr), last_match(last){}

      Match(const Match& t) 
        : matches(t.matches), many(t.many), type(t.type), last_match(t.last_match){}
      void set_many(Match* p)
      {
        many = p;
      }

      Type get_type() const
      {
        return type;
      }
      Match* get_many() const { return many; }

      Match* match(const Type& t) 
      {
        if (many != nullptr)
          return many->match(t);
        return matches.find(t);
      }

      Match* to_last_match() const
      {
        return last_match;
      }
      bool has_completed() const
      {
        if (many != nullptr)
          return matches.empty() && many->has_completed();
        return matches.empty();
      }
      Type guess_if_forget(Type t) const
      {
        if (many != nullptr)
          return many->guess_if_forget(t);
        return matches.guess_if_forget(t);
      }
      Type guess_one() const
      {
        if (many != nullptr)
          return many->guess_one();
        return matches.guess_one();
      }
    private:
      Match* add(const Type& t)
      {
        return matches.insert(Match(t, this));
      }
    };
    bool operator==(const Match& m1, const Match& m2)
    {
      return m1.type == m2.type;
    }

    //size_t debug_count_many(const Match& m)
    //{
    //  const Match* ptr = m.to_last_match();
    //  const Match* many = m.get_many();
    //  std::size_t num = 1;
    //  while (ptr != many)
    //  {
    //    ptr = ptr->to_last_match();
    //    num++;
    //  }
    //  return num;
    //}
    //size_t debug_size_of_match(const Match& m)
    //{
    //  const Match* ptr = &m;
    //  std::size_t size = 0;
    //  while (ptr != nullptr)
    //  {
    //    ptr = ptr->to_last_match();
    //    size++;
    //  }
    //  return size;
    //}
    //void debug_check_match(const Match& m)
    //{
    //  std::string space(debug_size_of_match(m) * 5, '-');
    //  std::cout << space << "type: " << get_mean(m.get_type()) << "   ";             
    //  if (m.many) 
    //  {
    //    std::cout << "~~~many: " << debug_count_many(m);
    //  }
    //  std::cout << std::endl;
    //  for (auto& r : m.matches)
    //  {
    //    debug_check_match(*r);
    //  }
    //}
    Match make_match(const std::vector<Rule>& rule_vec)
    {
      Match ret(Type::UNEXPECTED);
      Match* curr = &ret;
      for (std::size_t i = 0; i < rule_vec.size(); i++)
      {
        Match* beg = nullptr;
        Match* beforebeg = nullptr;
        for (std::size_t j = 0; j < rule_vec[i].size(); j++)
        {
          switch (rule_vec[i][j])
          {
          case Type::MB:
            j++;
            curr = curr->add(rule_vec[i][j]);
            beg = curr;
            break;
          case Type::ME:
            if(beg)
              curr = curr->add(beg->get_type());
            curr->set_many(beg);
            j++;// skip ME
            curr = beg;
            beg = nullptr;
            break;
          default:
            curr = curr->add(rule_vec[i][j]);
            break;
          }
        }
        curr = &ret;
      }
      //debug_check_match(ret);
      return ret;
    }

    const std::vector<Rule> all_rules =
    { 
      {Type::ID_TOK, Type::EQUAL_TOK, Type::BPATH_TOK,
      Type::MB, Type::ID_TOK, Type::BPATH_TOK, Type::ME,
      Type::ID_TOK, Type::COLON_TOK, Type::ID_TOK},// 'id = -a-b-c-d:val' or 'id = -a:val'
      
      {Type::ID_TOK, Type::COLON_TOK}, // 'id:'

      {Type::ID_TOK, Type::EQUAL_TOK, Type::COLON_TOK,
      Type::ID_TOK},// 'id = :val' 

      {Type::ID_TOK, Type::EQUAL_TOK, Type::ARR_LPAREN_TOK, 
      Type::MB, Type::INT_TOK, Type::COMMA_TOK, Type::ME, 
      Type::INT_TOK, Type::ARR_RPAREN_TOK},//'id = [1,2]' or 'id = [1]'   

      {Type::ID_TOK, Type::EQUAL_TOK, Type::ARR_LPAREN_TOK, 
      Type::MB, Type::DOUBLE_TOK, Type::COMMA_TOK, Type::ME, 
      Type::DOUBLE_TOK, Type::ARR_RPAREN_TOK},//'id = [1.1,2.2]' or 'id = [1.1]'   

      {Type::ID_TOK, Type::EQUAL_TOK, Type::ARR_LPAREN_TOK, 
      Type::MB, Type::STRING_TOK, Type::COMMA_TOK, Type::ME, 
      Type::STRING_TOK, Type::ARR_RPAREN_TOK},//'id = ["1","2"]' or 'id = ["1"]'


      {Type::ID_TOK, Type::EQUAL_TOK, Type::INT_TOK},//'id = 1'
      {Type::ID_TOK, Type::EQUAL_TOK, Type::DOUBLE_TOK},//'id = 1.1'
      {Type::ID_TOK, Type::EQUAL_TOK, Type::STRING_TOK},// 'id = "11"'

      {Type::SCOPE_END_TOK} // end
    };
    std::string get_string_from_file(const std::string& path)
    {
      std::ifstream file{ path, std::ios::binary };
      std::stringstream ss;
      ss << file.rdbuf();
      return ss.str();
    }
    class Lexer
    {
    private:
      std::shared_ptr<File> code;
      std::size_t code_size;
      std::shared_ptr<std::vector<Token>> tokens;
      Match match;
      Match* match_ptr;
      Match* last_match_ptr;
      Pos codepos;
      bool parsing_path;
    public:
      Lexer(const std::string& path, const std::string& _filename)
        : code(std::make_shared<File>(_filename, get_string_from_file(path))),
        code_size(code->size()),
        tokens(std::make_shared<std::vector<Token>>(std::vector<Token>())),
        match(make_match(all_rules)),
        match_ptr(&match),
        last_match_ptr(match_ptr),
        codepos(code),
        parsing_path(false)
        {  }
      void set_file(const std::string& _code, const std::string& _filename)
      {
        code->set_file(_code, _filename);
      }
      void reset()
      {
        code->reset();
        code_size = 0;
        tokens = nullptr;
        codepos.reset();
        parsing_path = false;
      }
      std::shared_ptr<std::vector<Token>> get_all_token()
      {
        Token t(Type::UNEXPECTED, 0, get_pos().set_size(0));
        do
        {
          t = get_tok();
          check(t);
        } while (t.type != Type::FILE_END_TOK);
        return tokens;
      }
    private:
      void check(const Token& token)
      {
        if (token.type == Type::NOTE_TOK || token.type == Type::FILE_END_TOK || token.type == Type::SENTENCE_END_TOK) return;
        last_match_ptr = match_ptr;
        match_ptr = match_ptr->match(token.type);
        if (match_ptr)
          tokens->emplace_back(token);
        else
        {
          auto t = last_match_ptr->guess_if_forget(token.type);
          if (t != Type::UNEXPECTED)
            token.error("Unexpected token '" + get_mean(token.type) + "'.Did you forget '"
              + get_mean(t) + "'?");
          else
            token.error("Unexpected token '" + get_mean(token.type) + "'.Did you mean '"
              + get_mean(last_match_ptr->guess_one()) + "'?");
        }
        if (match_ptr && match_ptr->has_completed())
          match_ptr = &match;
      }


      Token get_tok()
      {
        while (check() && isspace(get())) 
          next();

        if (check() && isdigit(get()))//num
        {
          bool has_dot = false;
          std::string temp = "";
          do
          {
            if (get() == '.') has_dot = true;
            temp += get();
            next();
          } while (check() && (isdigit(get())) || get() == '.');

          if (has_dot)
            return Token(Type::DOUBLE_TOK, std::stod(temp), get_pos().set_size(temp.size()));
          return Token(Type::INT_TOK, std::stoi(temp), get_pos().set_size(temp.size()));
        }

        else if (check() && get() == '"')//str
        {
          std::string temp = "";
          next();//eat '"'.
          do
          {
            temp += get();
            next();
          } while (check() && get() != '"');
          next();//eat '"'
          return Token(Type::STRING_TOK, temp, get_pos().set_size(temp.size()));
        }

        else if ((check() && (isalpha(get()) || get() == '_')) || (parsing_path ? (get() == '.') : false))//id
          //解析path时将'.'和'..'作为id
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
            return Token(Type::SCOPE_END_TOK, temp, get_pos().set_size(temp.size()));
          else
            return Token(Type::ID_TOK, temp, get_pos().set_size(temp.size()));
        }

        else if (check() && marks.find(get()) != marks.end())//mark
        {
          next();
          if (marks[get(-1)] == Type::BPATH_TOK) parsing_path = true;
          if (parsing_path && marks[get(-1)] == Type::SENTENCE_END_TOK) parsing_path = false;
          return Token(marks[get(-1)], get(-1), get_pos().set_size(1));
        }

        else if (check(2) && get() == '/' && get(1) == 'b' && get(2) == '/')//note
        {
          std::string temp;
          next(3);//eat '/b/'
          while (!(check(2) && get() == '/' && get(1) == 'e' && get(2) == '/'))
          {
            temp += get();
            next();
          }
          next(3);//eat '/e/'
          return Token(Type::NOTE_TOK, temp, get_pos().set_size(temp.size()));
        }
        else if (codepos == code->size()) return Token(Type::FILE_END_TOK, 0, get_pos().set_size(0));
        else
          Token(Type::UNEXPECTED, 0, get_pos().set_size(0)).error(std::string("Unexpected token'"));
        return Token(Type::UNEXPECTED, 0, get_pos().set_size(0));
      }
      Pos get_pos()
      {
        return codepos;
      }
      bool check(const std::size_t& s = 0) 
      { 
        return (codepos + s) < code_size; 
      }
      char& get(const std::size_t& s = 0)
      {
        if (!check(s))
          Token(Type::UNEXPECTED, 0, get_pos().set_size(0)).error(std::string("Unexpected end of tokens.'")); 
        return (*code)[codepos + s];
      }
      void next(const std::size_t& s = 1)
      {
        codepos += s;
      }
    };
  }
}