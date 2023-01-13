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
#ifndef LIBCZH_DTOA_TEST_HPP
#define LIBCZH_DTOA_TEST_HPP

#include "unittest.hpp"

namespace czh::test
{
  void dtoa_test()
  {
    LIBCZH_MARK("DTOA TEST");
    LIBCZH_EXPECT_EQ("Dtoa 1", ::czh::utils::dtoa(0.0), "0.0");
    LIBCZH_EXPECT_EQ("Dtoa 2", ::czh::utils::dtoa(-0.0), "-0.0");
    LIBCZH_EXPECT_EQ("Dtoa 3", ::czh::utils::dtoa(1.0), "1.0");
    LIBCZH_EXPECT_EQ("Dtoa 4", ::czh::utils::dtoa(-1.0), "-1.0");
    LIBCZH_EXPECT_EQ("Dtoa 5", ::czh::utils::dtoa(1.2345), "1.2345");
    LIBCZH_EXPECT_EQ("Dtoa 6", ::czh::utils::dtoa(1.2345678), "1.2345678");
    LIBCZH_EXPECT_EQ("Dtoa 7", ::czh::utils::dtoa(0.123456789012), "0.123456789012");
    LIBCZH_EXPECT_EQ("Dtoa 8", ::czh::utils::dtoa(1234567.8), "1234567.8");
    LIBCZH_EXPECT_EQ("Dtoa 9", ::czh::utils::dtoa(-79.39773355813419), "-79.39773355813419");
    LIBCZH_EXPECT_EQ("Dtoa 10", ::czh::utils::dtoa(-36.973846435546875), "-36.973846435546875");
    LIBCZH_EXPECT_EQ("Dtoa 11", ::czh::utils::dtoa(0.000001), "0.000001");
    LIBCZH_EXPECT_EQ("Dtoa 12", ::czh::utils::dtoa(0.0000001), "1e-7");
    LIBCZH_EXPECT_EQ("Dtoa 13", ::czh::utils::dtoa(1e30), "1e30");
    LIBCZH_EXPECT_EQ("Dtoa 14", ::czh::utils::dtoa(1.234567890123456e30), "1.234567890123456e30");
    LIBCZH_EXPECT_EQ("Dtoa 15", ::czh::utils::dtoa(5e-324), "5e-324");
    LIBCZH_EXPECT_EQ("Dtoa 16", ::czh::utils::dtoa(2.225073858507201e-308), "2.225073858507201e-308");
    LIBCZH_EXPECT_EQ("Dtoa 17", ::czh::utils::dtoa(2.2250738585072014e-308), "2.2250738585072014e-308");
    LIBCZH_EXPECT_EQ("Dtoa 18", ::czh::utils::dtoa(1.7976931348623157e308), "1.7976931348623157e308");
  }
}
#endif
