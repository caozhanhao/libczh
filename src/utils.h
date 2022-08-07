#pragma once
#include <sstream>
#include <charconv>
namespace czh::utils
{
  template <typename T>
  T str_to(const std::string& str)
  {
    std::stringstream ss;
    T a = 0;
    ss << str;
    ss >> a;
    return a;
  }
  template <typename T>
  std::string to_str(const T& a)
  {
    std::stringstream ss;
    ss << a;
    return ss.str();
  }
}