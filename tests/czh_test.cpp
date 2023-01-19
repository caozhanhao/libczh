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
#include "unittest.hpp"

#include <vector>

namespace czh::test
{
  class ContainerTest
  {
  public:
    using value_type = int;
  public:
    ContainerTest() = default;
  
    std::vector<int> c;
  
    auto begin() { return c.begin(); }
  
    auto end() { return c.end(); }
  
    auto insert(typename std::vector<int>::iterator it, int i) { return c.insert(it, i); }
  };
  
  class RangeTest
  {
  public:
    using value_type = int;
  public:
    class IteratorTest
    {
    private:
      int i;
    public:
      explicit IteratorTest(int position = 0) : i{position} {}
      
      int operator*() const { return i; }
      
      IteratorTest &operator++()
      {
        ++i;
        return *this;
      }
      
      bool operator!=(const IteratorTest &other) const { return i != other.i; }
    };
  
  private:
    int a;
    int b;
  public:
    RangeTest(int from, int to)
        : a{from}, b{to} {}
  
    IteratorTest begin() const { return IteratorTest{a}; }
  
    IteratorTest end() const { return IteratorTest{b}; }
  };
  
  
  LIBCZH_TEST(czh)
  {
    czh::Czh czhtest("../../tests/czh/inputtest.czh", czh::InputMode::nonstream);
    auto nodetest = czhtest.parse();
    LIBCZH_EXPECT_EQ(nodetest["czh"]("dt1").get<double>(), 1.0000000000000002);
    LIBCZH_EXPECT_EQ(nodetest("czh")["dt2"].get<double>(), 2.2250738585072009e-308);
    LIBCZH_EXPECT_EQ(nodetest["czh"]["dt3"].get<double>(), -2.2250738585072009e-308);
    LIBCZH_EXPECT_EQ(nodetest("czh")("dt4").get<double>(), 2.2250738585072014e-308);
    LIBCZH_EXPECT_EQ(nodetest("czh")("dt5").get<double>(), -2.2250738585072014e-308);
    LIBCZH_EXPECT_EQ(nodetest("czh")("dt6").get<double>(), 1.7976931348623157e308);
    LIBCZH_EXPECT_EQ(nodetest("czh")("dt7").get<double>(), -1.7976931348623157e308);
    LIBCZH_EXPECT_EQ(utils::dtoa(nodetest("czh")("dt8").get<double>()), "0.0");
    LIBCZH_EXPECT_EQ(utils::dtoa(nodetest("czh")("dt9").get<double>()), "-0.0");
    bool a = nodetest("czh")("int_array").get<ContainerTest>().c == (std::vector<int>{-600, 2, -9000});
    LIBCZH_EXPECT_TRUE(a);
    std::string edit_example{"edit example"};
    nodetest("czh")("double") = edit_example;
    nodetest("czh")("block")("d") = "d changed";
    nodetest("czh")("int_array") = {1, 2, 3};
    int arr_modify[] = {1, 2, 3};
    nodetest("czh")("int_array") = arr_modify;
    nodetest("czh")("any_array") = {false, 1, "2", 3.0};
    LIBCZH_EXPECT_EQ(std::get<std::string>(nodetest("czh")("any_array").get<czh::value::Array>()[2]), "2");
    nodetest("czh")("int_array") = RangeTest(1, 10);
    nodetest("czh").add_node("ref", "block");
    nodetest("czh").add("add_test", RangeTest(10, 15), "int");
    nodetest("czh").add("ref", nodetest("czh")("int"));
    nodetest("czh")("double").rename("edit");
    nodetest("czh")("bool_array").remove();
    nodetest("czh")("value_array_map").clear();
    LIBCZH_EXPECT_EQ(nodetest.to_string(),
                     "czh:\n  int_array = {1, 2, 3, 4, 5, 6, 7, 8, 9}\n  string_array = {\"1\", \"2\", \"3\"}\n  any_array = {false, 1, \"2\", 3.0}\n  \xF0\x9F\x98\x80UTF\xE7\xA4\xBA\xE4\xBE\x8B = \"\xE6\xB5\x8B\xE8\xAF\x95\xF0\x9F\x93\x95\"\n  edit = \"edit example\"\n  bool = true\n  add_test = {10, 11, 12, 13, 14}\n  int = \"d changed\"\n  long_long = 200000000000\n  null_example = null\n  ref:\n  end\n  block:\n    czh:\n      int = 0\n    end\n    a = int\n    b = czh::int\n    c = int\n    d = c\n  end\n  value_map:\n    k1 = 7\n    k2 = 8\n    k3 = 9\n  end\n  value_array_map:\n  end\n  dt1 = 1.0000000000000002\n  dt2 = 2.225073858507201e-308\n  dt3 = -2.225073858507201e-308\n  dt4 = 2.2250738585072014e-308\n  dt5 = -2.2250738585072014e-308\n  dt6 = 1.7976931348623157e308\n  dt7 = -1.7976931348623157e308\n  dt8 = 0.0\n  dt9 = -0.0\n  ref = int\nend\n");
  }
}