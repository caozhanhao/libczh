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
#include "dtoa_test.hpp"
#include "czh_test.hpp"

int main()
{
  int ret = 0;
  try
  {
    auto &test = czh::test::get_test();
    czh::test::czh_test();
    czh::test::dtoa_test();
    ret = test.run_tests();
    test.print_results();
  }
  catch (Error &e)
  {
    std::cerr << e.get_content() << std::endl;
    return -1;
  }
  return ret;
}