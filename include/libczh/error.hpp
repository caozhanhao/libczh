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
#ifndef LIBCZH_ERROR_HPP
#define LIBCZH_ERROR_HPP
#pragma once

#include <stdexcept>
#include <string>
#include <experimental/source_location>

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
              + "\033[0;32;31m report_error : \033[m" + detail};
    }
  };
  
  std::string location_to_str(const std::experimental::source_location &l)
  {
    return std::string(l.file_name()) + ":" + std::to_string(l.line()) +
           ":" + l.function_name() + "()";
  }
  
  class Error : public std::logic_error
  {
  private:
    std::string location;
    std::string detail;
  
  public:
    Error(const std::string &detail_, const std::experimental::source_location &l =
    std::experimental::source_location::current())
        : logic_error(detail_),
          location(location_to_str(l)),
          detail(detail_) {}
    
    [[nodiscard]] std::string get_content() const
    {
      return "\033[0;32;31mError: \033[1;37m" + location + ":\033[m " + detail;
    }
    
    [[nodiscard]] std::string get_detail() const
    {
      return detail;
    }
    
    //For Unit Test
    bool operator==(const Error &e) const
    {
      return detail == e.detail;
    }
  };
  
  constexpr auto czh_invalid_file = "Invalid file";
  
  auto czh_unreachable(const std::string &detail_ = "", const std::experimental::source_location &l =
  std::experimental::source_location::current())
  {
    throw Error("Unreachable code: " + detail_, l);
  }
  
  void czh_assert(bool b,
                  const std::string &detail_ = "Assertion failed.",
                  const std::experimental::source_location &l =
                  std::experimental::source_location::current())
  {
    if (!b)
    {
      throw Error(detail_, l);
    }
  }
}
#endif