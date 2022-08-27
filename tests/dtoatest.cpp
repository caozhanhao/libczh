//   Copyright 2021-2022 libczh - caozhanhao
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
#include "tests.hpp"
TEST(libczh, dtoa)
{
auto f = [](double a) { return utils::value_to_str(a); };
EXPECT_EQ(f(0.0), "0.0");
EXPECT_EQ(f(-0.0), "-0.0");
EXPECT_EQ(f(1.23456789), "1.23456789");
EXPECT_EQ(f(1234567.89), "1234567.89");
EXPECT_EQ(f(-36.973846435546875), "-36.973846435546875");
EXPECT_EQ(f(0.000001), "0.000001");
EXPECT_EQ(f(1.234567890123456e+30), "1.234567890123456e+30");
EXPECT_EQ(f(2.225073858507201e-308), "2.225073858507201e-308");
EXPECT_EQ(f(2.2250738585072014e-308), "2.2250738585072014e-308");
EXPECT_EQ(f(1.7976931348623157e+308), "1.7976931348623157e+308");
EXPECT_EQ(f(-1.7976931348623157e+308), "-1.7976931348623157e+308");
}