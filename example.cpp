#include "czh.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

void test()
{
  czh::Czh e("example.czh", czh::Czh::file);
  auto example = *e.parse();

  //???
  cout << "example int: " << example["example"]["int_example"].get<int>() << endl;
  cout << "example double: " << example["example"]["double_example"].get<double>() << endl;
  cout << "example string: " << example["example"]["string_example"].get<std::string>() << endl;
  cout << "example ref: " << example["example"]["block_example"]["ref_example0"].get<int>() << endl;

  //value_map
  cout << "\nvalue_map(map<string, int>): ";
  auto m1 = *example["example"]["value_map_example"].value_map<int>();
  for (auto& r : m1)
    cout << r.second << ",";
  cout << "\nexample value_array_map(to map<string, vector<int>>): ";
  auto m2 = *example["example"]["value_array_map_example"].value_map<vector<int>>();
  for (auto& r : m2)
    for (auto& a : r.second)
      cout << a << ",";

  //???
  example["example"]["double_example"].get_value() = "edit example";
  //???
  example["example"].rename("double_example", "edit_example");
  //??Value
  example["example"].add("add_test", "123", "edit_example");
  //??Value
  example["example"].remove("string_example");
  //??Node
  example["example"]["value_array_map_example"].clear();
  //??Node
  example.add_node("ref")->add("ref", example["example"]["edit_example"].get_ref());
  //??Node
  example["example"].remove("value_map_example");
  
  //??(??)
  std::cout << "\n" << example.to_string(czh::node::Node::color) << std::endl;
  
  //????
  std::ofstream out("output.czh");
  out << example;
  out.close();

  //test
  czh::Czh e1("output.czh", czh::Czh::file);
  czh::Czh e2("onelinetest.czh", czh::Czh::file);
  auto example1 = *e1.parse();
  auto example2 = *e2.parse();
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
