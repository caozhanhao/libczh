#pragma once
#include <sstream>
#include <cstdlib>
#include <iomanip>
#include <charconv>
namespace czh::utils
{
  template<class... Ts>
  struct overloaded : Ts ...
  {
    using Ts::operator()...;
  };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
  
  template<typename T>
  T str_to(const std::string &str) = delete;
  
  template<>
  double str_to(const std::string &str)
  {
    return std::strtod(str.c_str(), nullptr);
  }
  
  template<typename T>
  std::string to_str(const T &a)
  {
    return std::to_string(a);
  }
  
  template<typename T>
  std::string value_to_str(const T &a)
  {
    return std::to_string(a);
  }
  
  template<>
  std::string value_to_str(const double &a)
  {
    std::stringstream ss;
    ss << std::setprecision(17) << a;
    return ss.str();
  }
  
    int get_distance(const std::string& s1, const std::string& s2)
    {
      std::size_t n = s1.size();
      std::size_t m = s2.size();
      if (n * m == 0) return n + m;
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