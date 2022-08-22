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
#ifndef LIBCZH_UTILS_HPP
#define LIBCZH_UTILS_HPP

#include "dtoa.hpp"
#include <cstdlib>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cmath>

namespace czh::utils
{
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
  
  template<typename T>
  std::string value_to_str(T &&a)
  {
    return std::to_string(std::forward<T>(a));
  }
  
  template<>
  std::string value_to_str(const double &a)
  {
    return dtoa(a);
  }
  
  int get_string_edit_distance(const std::string &s1, const std::string &s2)
  {
    std::size_t n = s1.size();
    std::size_t m = s2.size();
    if (n * m == 0) return static_cast<int>(n + m);
    std::vector<std::vector<int>> D(n + 1, std::vector<int>(m + 1));
    for (int i = 0; i < n + 1; i++)
      D[i][0] = i;
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