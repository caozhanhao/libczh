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
#ifndef LIBCZH_UTILS_HPP
#define LIBCZH_UTILS_HPP
#pragma once

#include "dtoa.hpp"
#include "value.hpp"
#include <cstdlib>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cmath>
#include <map>

namespace czh::utils
{
  enum class Color
  {
    with_color,
    no_color
  };
  enum class ColorType
  {
    ID, REF_ID, NUM, STR, BOOL, BLOCK_BEG, BLOCK_END, NULL_TYPE
  };
  enum class CzhColor
  {
    BLUE, LIGHT_BLUE, GREEN, PURPLE, YELLOW, WHITE, RED
  };
  
  static CzhColor get_color(ColorType a)
  {
    static std::map<ColorType, CzhColor> colors =
        {
            {ColorType::ID,        CzhColor::PURPLE},
            {ColorType::REF_ID,    CzhColor::LIGHT_BLUE},
            {ColorType::NUM,       CzhColor::BLUE},
            {ColorType::STR,       CzhColor::GREEN},
            {ColorType::BOOL,      CzhColor::BLUE},
            {ColorType::NULL_TYPE, CzhColor::BLUE},
            {ColorType::BLOCK_BEG, CzhColor::LIGHT_BLUE},
            {ColorType::BLOCK_END, CzhColor::LIGHT_BLUE},
        };
    return colors.at(a);
  }
  
  std::string colorify(const std::string &str, Color with_color, ColorType type)
  {
    if (with_color == Color::no_color) return str;
    switch (get_color(type))
    {
      case CzhColor::PURPLE:
        return "\033[35m" + str + "\033[0m";
      case CzhColor::LIGHT_BLUE:
        return "\033[36m" + str + "\033[0m";
      case CzhColor::BLUE:
        return "\033[34m" + str + "\033[0m";
      case CzhColor::GREEN:
        return "\033[32m" + str + "\033[0m";
      case CzhColor::YELLOW:
        return "\033[33m" + str + "\033[0m";
      case CzhColor::WHITE:
        return "\033[37m" + str + "\033[0m";
      case CzhColor::RED:
        return "\033[31m" + str + "\033[0m";
      default:
        error::czh_unreachable();
    }
    error::czh_unreachable();
    return "";
  }
  
  template<class... Ts>
  struct overloaded : Ts ...
  {
    using Ts::operator()...;
  };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
  
  double str_to_num(const std::string &str)
  {
    return std::strtod(str.c_str(), nullptr);
  }
  
  template<typename T>
  std::string to_str(T &&a)
  {
    return std::to_string(std::forward<T>(a));
  }
  
  template<typename T, typename = std::enable_if_t<!std::is_same_v<double, std::decay_t<T>>
                                                   && !std::is_same_v<bool, std::decay_t<T>>>>
  std::string value_to_str(T &&a)
  {
    return std::to_string(std::forward<T>(a));
  }
  
  std::string value_to_str(const bool &a)
  {
    return a ? "true" : "false";
  }
  
  std::string value_to_str(const double &a)
  {
    return dtoa(a);
  }
  
  
  template<typename T>
  std::string to_czhstr(const T &val, Color color = Color::no_color)
  {
    return colorify(value_to_str(val), color, ColorType::NUM);
  }
  
  template<>
  std::string to_czhstr(const bool &val, Color color)
  {
    return colorify((val ? "true" : "false"), color, ColorType::BOOL);
  }
  
  template<>
  std::string to_czhstr(const value::Null &val, Color color)
  {
    return colorify("null", color, ColorType::NULL_TYPE);
  }
  
  template<>
  std::string to_czhstr(const std::string &val, Color color)
  {
    return colorify(("\"" + val + "\""), color, ColorType::STR);
  }
  
  template<>
  std::string to_czhstr(const char *const &val, Color color)
  {
    return to_czhstr(std::string(val), color);
  }
  
  template<>
  std::string to_czhstr(const value::Array &v, Color color)
  {
    auto visitor = [&color](auto &&v) -> std::string { return to_czhstr(v, color); };
    std::string result = "{";
    for (auto it = v.cbegin(); it != std::prev(v.cend()); ++it)
    {
      result += std::visit(visitor, *it);
      result += ",";
    }
    result += std::visit(visitor, *std::prev(v.cend()));
    result += "}";
    return result;
  }
  
  int get_string_edit_distance(const std::string &s1, const std::string &s2)
  {
    std::size_t n = s1.size();
    std::size_t m = s2.size();
    if (n * m == 0) return static_cast<int>(n + m);
    std::vector<std::vector<int>> D(n + 1, std::vector<int>(m + 1));
    for (int i = 0; i < n + 1; i++)
    {
      D[i][0] = i;
    }
    for (int j = 0; j < m + 1; j++)
      D[0][j] = j;
    
    for (int i = 1; i < n + 1; i++)
    {
      for (int j = 1; j < m + 1; j++)
      {
        int left = D[i - 1][j] + 1;
        int down = D[i][j - 1] + 1;
        int left_down = D[i - 1][j - 1];
        if (s1[i - 1] != s2[j - 1]) left_down += 1;
        D[i][j] = std::min(left, std::min(down, left_down));
      }
    }
    return D[n][m];
  }
}
#endif