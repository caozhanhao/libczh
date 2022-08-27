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
#ifndef LIBCZH_ERR_HPP
#define LIBCZH_ERR_HPP

#include <stdexcept>
#include <string>

#define _LIBCZH_STRINGFY(x) #x
#define LIBCZH_STRINGFY(x) _LIBCZH_STRINGFY(x)
#define LIBCZH_ERROR_LOCATION  __FILE__ ":line " LIBCZH_STRINGFY(__LINE__)

namespace czh::error
{
  class CzhError : public std::runtime_error
  {
  private:
    std::string location;
    std::string detail;
  public:
    CzhError(std::string location_, const std::string &detail_)
        : runtime_error(detail_), location(std::move(location_)),
          detail(detail_) {}
    
    [[nodiscard]] std::string get_content() const
    {
      return {"\033[1;37m" + location + ":"
              + "\033[0;32;31m error : \033[m" + detail};
    }
  };
  
  class Error : public std::logic_error
  {
  private:
    std::string location;
    std::string detail;
  public:
    Error(std::string location_, const std::string &func_name_, const std::string &detail_)
        : logic_error(detail_), location(std::move(location_) + ":" + func_name_ + "()"),
          detail(detail_) {}
    
    [[nodiscard]] std::string get_detail() const
    {
      return detail;
    }
    
    [[nodiscard]] std::string get_content() const
    {
      return {"\033[1;37m" + location + ":"
              + "\033[0;32;31m error : \033[m" + detail};
    }
  };
}
#endif