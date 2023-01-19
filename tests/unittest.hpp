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
#ifndef LIBCZH_UNITTESTS_HPP
#define LIBCZH_UNITTESTS_HPP

#include "libczh/czh.hpp"
#include <random>
#include <map>
#include <set>
#include <functional>
#include <chrono>
#include <type_traits>
#include <experimental/source_location>

#define _LIBCZH_STRINGFY(x) #x
#define LIBCZH_STRINGFY(x) _LIBCZH_STRINGFY(x)

#define LIBCZH_EXPECT_EQ(v1, v2) \
try{::czh::test::get_test().expect_eq(v1, v2);} \
catch(Error& e){throw czh::test::UnitTestError(e);} \
catch(CzhError& e){throw czh::test::UnitTestError(e);}
#define LIBCZH_EXPECT_TRUE(v1) \
try{::czh::test::get_test().expect_eq(v1, true);} \
catch(Error& e){throw czh::test::UnitTestError(e);} \
catch(CzhError& e){throw czh::test::UnitTestError(e);}
#define LIBCZH_EXPECT_FALSE(v1) \
try{::czh::test::get_test().expect_eq(v1, false);} \
catch(Error& e){throw czh::test::UnitTestError(e);} \
catch(CzhError& e){throw czh::test::UnitTestError(e);}
#define LIBCZH_TEST(name) \
void libczh_test_##name(); \
int libczh_test_pos_##name = ::czh::test::get_test().add(LIBCZH_STRINGFY(name), libczh_test_##name); \
void libczh_test_##name()

namespace czh::test
{
  class UnitTestError
  {
  private:
    std::string test_location;
    std::string content;
  
  public:
    UnitTestError(const Error &err, const std::experimental::source_location &l =
    std::experimental::source_location::current())
        : content(err.get_content()), test_location(error::location_to_str(l)) {}
    
    UnitTestError(const CzhError &err, const std::experimental::source_location &l =
    std::experimental::source_location::current())
        : content(err.get_content()), test_location(error::location_to_str(l)) {}
    
    const std::string &get_content() const { return content; }
    
    const std::string &get_location() const { return test_location; }
  };
  
  class Test;
  
  Test &get_test();
  
  class Timer
  {
  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> beg;
  public:
    void start()
    {
      beg = std::chrono::high_resolution_clock::now();
    }
    
    [[nodiscard]] auto get_microseconds() const
    {
      return std::chrono::duration_cast<std::chrono::microseconds>
          (std::chrono::high_resolution_clock::now() - beg).count();
    }
  };
  
  template<typename T>
  T random_digit(T a, T b)//[a,b]
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<T> dis{a, b};
    return dis(gen);
  }
  
  char randichar(int a, int b)//[a,b]
  {
    return static_cast<char>(static_cast<char>(random_digit(a, b)) + '0');
  }
  
  std::string rand_digit_str(size_t n)
  {
    if (n == 0)return "";
    std::string num;
    num += randichar(1, 9);
    for (size_t i = 1; i < n; ++i)
    {
      num += randichar(0, 9);
    }
    return num;
  }
  
  namespace test_internal
  {
    template<typename F>
    auto is_callable(F f)
    -> decltype(f(), std::true_type()) { return std::true_type(); }
    
    template<typename F>
    std::false_type is_callable(F f) { return std::false_type(); }
    
    struct StrTag {};
    struct CzhTag {};
    struct BasicTag {};
    struct BoolTag {};
    struct CharptrTag {};
    template<typename U>
    struct TagDispatch
    {
      using tag = std::conditional_t<std::is_same_v<std::decay_t<U>, std::string>, StrTag,
          std::conditional_t<std::is_same_v<std::decay_t<U>, const char *>, CharptrTag,
              std::conditional_t<std::is_same_v<std::decay_t<U>, bool>, BoolTag,
                  std::conditional_t<std::is_fundamental_v<std::decay_t<U>>, BasicTag, CzhTag>>>>;
    };
    
    std::string internal_to_str(const char *a, CharptrTag)
    {
      return {a};
    }
    
    std::string internal_to_str(std::string a, StrTag)
    {
      return a;
    }
    
    std::string internal_to_str(bool a, BoolTag)
    {
      return a ? "true" : "false";
    }
    
    template<typename T>
    std::string internal_to_str(const T &a, CzhTag)
    {
      return a.to_string();
    }
    
    template<typename T>
    std::string internal_to_str(const T &a, BasicTag)
    {
      return std::to_string(a);
    }
  }
  
  template<typename T>
  std::string to_str(T &&a)
  {
    return test_internal::internal_to_str(a, typename test_internal::TagDispatch<std::decay_t<T>>::tag{});
  }
  
  class Test
  {
  private:
    std::vector<std::pair<std::string, std::function<void()>>> all_tests;
    std::vector<size_t> exceptions;
    std::vector<size_t> failure;
    double time;
    std::string results;
    size_t curr_test;
  public:
    Test() : time(0), curr_test(0) {}
  
    template<typename T1, typename T2>
    void expect_eq(const T1 &t1, const T2 &t2,
                   const std::experimental::source_location &l =
                   std::experimental::source_location::current())
    {
      if (t1 != t2)
      {
        results += ("[\033[0;32;31mFAILED\033[m] Test '" + all_tests[curr_test].first + "' in \033[1;37m"
                    + error::location_to_str(l) + ":\033[m " + to_str(t1) +
                    "\033[0;32;31m != \033[m" + to_str(t2) +
                    "\n");
        failure.emplace_back(curr_test);
      }
    }
  
    template<typename T>
    int add(const std::string &name, const T &func)
    {
      all_tests.template emplace_back(std::make_pair(name, func));
      return all_tests.size() - 1;
    }
  
    template<typename T1, typename T2>
    int add_expect_exception(const std::string &name, const T1 &func, const T2 &exception,
                             const std::experimental::source_location &l =
                             std::experimental::source_location::current())
    {
      all_tests.template emplace_back(
          [this, func, exception, location = error::location_to_str(l)]()
          {
            bool caught = false;
            try
            {
              func();
            }
            catch (T2 &err)
            {
              if (err != exception)
              {
                throw err;
              }
              else
                caught = true;
            }
            if (!caught)
            {
              results += ("[\033[0;32;31mFAILED\033[m] Test '" + all_tests[curr_test].first + "' in \033[1;37m"
                          + location + ":\033[m Exception not captured.");
              failure.template emplace_back(curr_test);
            }
          });
      return all_tests.size() - 1;
    }
    
    int run_tests()
    {
      Timer timer;
      timer.start();
      std::string curr_msg;
      size_t curr_pos = 0;
      bool print_mark = false;
      auto s = timer.get_microseconds();
      auto e = s;
      size_t last_failure = 0;
      size_t last_exception = 0;
      for (curr_test = 0; curr_test < all_tests.size(); ++curr_test)
      {
        try
        {
          all_tests[curr_test].second();
          e = timer.get_microseconds();
        }
        catch (UnitTestError &e)
        {
          results += ("[\033[0;32;31mEXCEPTION\033[m] Test "
                      + std::to_string(curr_test) + " at " + e.get_location()
                      + ":\n" + e.get_content() + "\n");
          exceptions.emplace_back(curr_test);
        }
        if (failure.size() != last_failure || exceptions.size() != last_exception)
        {
          last_exception = exceptions.size();
          last_failure = failure.size();
        }
        else
        {
          results += "[\033[0;32;32mPASSED\033[m] " + all_tests[curr_test].first
                     + " | " + "(" + ::czh::utils::dtoa((e - s) * 0.001) + " ms)\n";
        }
        time += e - s;
        s = timer.get_microseconds();
      }
  
      if (!failure.empty() || !exceptions.empty()) return -1;
      return 0;
    }
    
    void print_results()
    {
      std::cout << results;
      if (failure.empty() && exceptions.empty())
      {
        std::cout << ("\033[0;32;32mUNITTEST PASSED\033[m");
      }
      else
      {
        std::cout << ("\033[0;32;31mUNITTEST FAILED\033[m");
      }
      std::cout << ("(" + ::czh::utils::dtoa(time * 0.000001) + " s)\n");
      if (!exceptions.empty())
      {
        std::cout << "[\033[0;32;33mDETAIL\033[m] "
                  << (std::to_string(exceptions.size()) + " unexpected exception(s) at[");
        std::string str;
        for (auto &r: exceptions)
        {
          str += std::to_string(r) + ',';
        }
        str.pop_back();
        std::cout << str << "]." << std::endl;
      }
      if (!failure.empty())
      {
        std::cout << "[\033[0;32;33mDETAIL\033[m] " << (std::to_string(failure.size()) + " failure at[");
        std::string str;
        for (auto &r: failure)
        {
          str += std::to_string(r) + ',';
        }
        str.pop_back();
        std::cout << str << "]." << std::endl;
      }
    }
  };
  
  Test &get_test()
  {
    static Test test;
    return test;
  }
}
#endif
