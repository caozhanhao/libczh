//   Copyright 2021-2022 libczh - caozhanhao
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
#ifndef LIBCZH_FILE_HPP
#define LIBCZH_FILE_HPP

#include <memory>
#include <string>
#include <fstream>
#include <limits>

namespace czh::file
{
  class File
  {
  public:
    std::string filename;
    
    explicit File(std::string name)
        : filename(std::move(name)) {}
    
    [[nodiscard]] virtual std::string
    get_spec_line(std::size_t beg, std::size_t end, std::size_t linenosize) const = 0;
    
    [[nodiscard]] virtual std::size_t get_lineno(std::size_t pos) const = 0;
    
    [[nodiscard]] virtual std::size_t get_arrowpos(std::size_t pos) const = 0;
    
    [[nodiscard]] virtual std::string get_name() const = 0;
    
    [[nodiscard]] virtual std::size_t size() const = 0;
    
    [[nodiscard]] virtual char get() = 0;
    
    [[nodiscard]] virtual char peek() = 0;
    
    [[nodiscard]] virtual bool check() = 0;
  };
  
  class StreamFile : public File
  {
  public:
    std::unique_ptr<std::ifstream> file;
    std::size_t file_size;
  public:
    StreamFile(std::string name_, std::unique_ptr<std::ifstream> fs_)
        : File(std::move(name_)), file(std::move(fs_))
    {
      if (!file->good())
      {
        throw error::Error(LIBCZH_ERROR_LOCATION, __func__, "Error File.");
      }
      file->ignore(std::numeric_limits<std::streamsize>::max());
      file_size = file->gcount();
      file->clear();
      file->seekg(std::ios_base::beg);
    }
    
    [[nodiscard]] std::string get_spec_line(std::size_t beg, std::size_t end, std::size_t linenosize) const override
    {
      std::string ret;
      file->clear();
      file->seekg(std::ios::beg);
      std::string tmp;
      for (int a = 1; std::getline(*file, tmp); a++)
      {
        if (beg <= a && a < end)
        {
          std::string addition = utils::to_str(a);
          if (addition.size() < linenosize)
            ret += std::string(linenosize - addition.size(), '0');
          ret += addition + "| ";
          ret += tmp;
          ret += "\n";
        }
      }
      ret.pop_back();
      return ret;
    }
  
    [[nodiscard]] std::size_t get_lineno(std::size_t pos) const override
    {
      std::size_t lineno = 1;
      std::size_t postmp = 0;
      std::string tmp;
      file->clear();
      file->seekg(std::ios::beg);
      for (; std::getline(*file, tmp); lineno++)
      {
        postmp += tmp.size() + 1;
        if (postmp >= pos) break;
      }
      return lineno;
    }
    
    [[nodiscard]] std::size_t get_arrowpos(std::size_t pos) const override
    {
      std::size_t postmp = 0;
      std::string tmp;
      file->clear();
      file->seekg(std::ios::beg);
      while (std::getline(*file, tmp))
      {
        if (postmp + tmp.size() >= pos) return pos - postmp + 1;
        postmp += tmp.size() + 1;
      }
      return 0;
    }
    
    [[nodiscard]] std::string get_name() const override
    {
      return filename;
    }
  
    [[nodiscard]] std::size_t size() const override
    {
      return file_size;
    }
  
    [[nodiscard]] char get() override
    {
      return static_cast<char>(file->get());
    }
  
    [[nodiscard]] char peek() override
    {
      return static_cast<char>(file->peek());
    }
  
    [[nodiscard]] bool check() override
    {
      return !file->eof();
    }
  };
  
  class NonStreamFile : public File
  {
  public:
    std::string code;
    std::size_t codepos;
  public:
    NonStreamFile(std::string name, std::string code_)
        : File(std::move(name)), code(std::move(code_)), codepos(0) {}
    
    [[nodiscard]] std::string get_spec_line(std::size_t beg, std::size_t end, std::size_t linenosize) const override
    {
      std::size_t lineno = 1;
      bool first_line_flag = true;
      bool first_line_no = true;
      std::string ret;
      for (std::size_t i = 0; i < code.size() && lineno < end; ++i)
      {
        if (code[i] == '\n')
        {
          ++lineno;
          first_line_flag = true;
          continue;
        }
        if (lineno >= beg)
        {
          if (first_line_flag)
          {
            std::string addition = utils::to_str(lineno);
            if (addition.size() < linenosize)
              ret += std::string(linenosize - addition.size(), '0');
            if (!first_line_no)
              ret += '\n';
            else
              first_line_no = false;
            ret += addition + "| ";
            first_line_flag = false;
          }
          if (code[i] != '\r')
            ret += code[i];
        }
      }
      while (ret.back() == '\r' || ret.back() == '\n')
      {
        ret.pop_back();
      }
      return ret;
    }
    
    [[nodiscard]] std::size_t get_lineno(std::size_t pos) const override
    {
      std::size_t lineno = 1;
      for (std::size_t i = 0; i < pos; ++i)
      {
        if (code[i] == '\n')
          lineno++;
      }
      return lineno;
    }
    
    [[nodiscard]] std::size_t get_arrowpos(std::size_t pos) const override
    {
      int i = static_cast<int>(pos);
      if (pos != 1)
        --i;
      while (code[i] != '\n' && i >= 0)
        --i;
      return pos - i;
    }
    
    [[nodiscard]] std::string get_name() const override
    {
      return filename;
    }
    
    [[nodiscard]] std::size_t size() const override
    {
      return code.size();
    }
  
    [[nodiscard]] char get() override
    {
      return code[codepos++];
    }
  
    [[nodiscard]] char peek() override
    {
      return code[codepos + 1];
    }
  
    [[nodiscard]] bool check() override
    {
      return codepos < code.size();
    }
  };
}
#endif