#pragma once
#include <stdexcept>
#include <string>

#define _CZH_STRINGFY(x) #x
#define CZH_STRINGFY(x) _CZH_STRINGFY(x)
#define CZH_ERROR_LOCATION  __FILE__ ":line " CZH_STRINGFY(__LINE__)

namespace czh
{
  namespace error
  {
    class Err : public std::logic_error
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
      Err(std::string _location, std::string _func_name, std::string _details, bool _internal = false)
        :logic_error("Err"), location(_location), func_name(_func_name), details(_details), is_internal_(_internal){}

      std::string get_details(const bool& add_location = true) const
      {
        if (!is_internal())
          return (location + ": error: " + details);
        else if (add_location)
          return (location + ":" + func_name + "(): \n" + details);
        return details;
      }
      bool is_internal() const
      {
        return is_internal_;
      }
    };
  }
}