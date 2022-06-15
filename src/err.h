#pragma once
#include <stdexcept>
#include <string>

#define _CZH_STRINGFY(x) #x
#define CZH_STRINGFY(x) _CZH_STRINGFY(x)
#define CZH_ERROR_LOCATION  __FILE__ ":line " CZH_STRINGFY(__LINE__)

namespace czh::error
{
  class Error : public std::logic_error
  {
  public:
    static const bool internal = true;
    static const bool dont_add_location = false;
  private:
    std::string location;
    std::string func_name;
    std::string details;
    bool is_internal_;
  public:
    Error(std::string location_, std::string func_name_, const std::string& details_, bool internal_ = false)
        : logic_error(details_), location(std::move(location_)),
        func_name(std::move(func_name_)), details(details_),
          is_internal_(internal_) {}
    
    [[nodiscard]] std::string get(const bool &add_location = true) const
    {
      std::string ret;
      if (add_location)
        ret += "\033[1;37m" + location + ":";
      if (!is_internal())
        ret += "\033[0;32;31m error : \033[m" + details;
      return ret;
    }
  
    [[nodiscard]] std::string get_detail() const
    {
      return details;
    }
  
    [[nodiscard]] bool is_internal() const
    {
      return is_internal_;
    }
  };
}