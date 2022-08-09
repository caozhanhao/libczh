#pragma once
#include <sstream>
#include <cstdlib>
#include <iomanip>
#include <charconv>
namespace czh::utils
{
  template <typename T>
  T str_to(const std::string& str) = delete;
  template <>
  double str_to(const std::string& str)
  {
    return std::strtod(str.c_str(), nullptr);
  }
  template <typename T>
  std::string to_str(const T& a)
  {
    return std::to_string(a);
  }
  template <typename T>
  std::string value_to_str(const T& a)
  {
    return std::to_string(a);
  }
  template <>
  std::string value_to_str(const double& a)
  {
    std::stringstream ss;
    ss << std::setprecision(17) << a;
    return ss.str();
  }
}