#include "czh.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

void test()
{
  czh::Czh e("example.czh", czh::Czh::file);
  auto example = *e.parse();

  cout << "example int: " << example["example"]["int_example"].get<int>() << endl;
  cout << "example double: " << example["example"]["double_example"].get<double>() << endl;
  cout << "example string: " << example["example"]["string_example"].get<std::string>() << endl;
  cout << "example ref: " << example["example"]["block_example"]["ref_example0"].get<int>() << endl;

  cout << "\nvalue_map(map<string, int>): ";
  auto m1 = *example["example"]["value_map_example"].value_map<int>();
  for (auto& r : m1)
    cout << r.second << ",";
  //同理，还有map<string, double>和map<string, string>
  cout << "\nexample value_array_map(to map<string, vector<int>>): ";
  auto m2 = *example["example"]["value_array_map_example"].value_map<vector<int>>();
  for (auto& r : m2)
    for (auto& a : r.second)
      cout << a << ",";
  //同理，还有map<string, vector<double>>和map<string, vector<string>>

  example["example"]["double_example"].get_value() = "edit test";
  example["example"]["bool_example"].get_value() = false;
  example["example"].add("add_test", "123", "double_example");
  example["example"].remove("string_example");
  example["example"]["value_array_map_example"].clear();

  std::ofstream out("output.czh");
  out << example;
  out.close();
  czh::Czh e1("output.czh", czh::Czh::file);
  auto example1 = *e1.parse();
}
int main()
{
  try
  {
    test();
  }
  catch (Error& err)
  {
    if (err.is_internal())
      std::cout << "internal:\n";
    std::cout << err.get() << std::endl;
  }
  return 0;
}
