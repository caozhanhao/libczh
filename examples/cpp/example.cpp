#include "../../src/czh.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

void test()
{
//  czh::Czh x("example.czh", czh::Czh::file);
//  auto xx = *x.parse();
  czh::Czh e(std::make_unique<std::ifstream>("example.czh"));
  auto example = *e.parse();
  
  //view_char value
  cout << "example int: " << example["example"]["int"].get<int>() << endl;
  cout << "example long: " << example["example"]["long_long"].get<long long>() << endl;
  cout << "example double: " << example["example"]["double"].get<double>() << endl;
  cout << "example string: " << example["example"]["string"].get<std::string>() << endl;
  cout << "example ref: " << example["example"]["block"]["ref0"].get<int>() << endl;
  
  
  cout << "example any array: ";
  auto arr = example["example"]["any_array"].get<czh::value::Value::AnyArray>();
  for(auto& r : arr)
    visit([](auto&& i){cout << czh::node::to_czhstr(i, false) << ", ";}, r);
  
  //value_map
  cout << "\nvalue_map(map<string, int>): ";
  auto m1 = *example["example"]["value_map"].value_map<int>();
  for (auto &r: m1)
    cout << r.second << ",";
  cout << "\nexample value_array_map(to map<string, vector<int>>): ";
  auto m2 = *example["example"]["value_array_map"].value_map<vector<int>>();
  for (auto &r: m2)
    for (auto &a: r.second)
      cout << a << ",";
  
  //edit
  example["example"]["double"].get_value() = "edit example";
  //rename
  example["example"].rename("double", "edit");
  //add Value
  example["example"].add("add_test", "123", "edit");
  //remove Value
  example["example"].remove("string");
  //clear Node
  example["example"]["value_array_map"].clear();
  //add Node
  example["example"].add_node("ref", "block")
  ->add("ref", example["example"]["edit"].get_ref());
  //remove Node
  example["example"].remove("value_map");
  
  //output(hightlight)
  std::cout << "\n" << example.to_string(czh::node::Node::color) << std::endl;
  
  //output to file
  std::ofstream out("output.czh");
  out << example;
  out.close();
  
  //test
  czh::Czh e1("output.czh", czh::Czh::file);
  auto example1 = *e1.parse();
  czh::Czh e2("onelinetest.czh", czh::Czh::file);
  auto example2 = *e2.parse();
  czh::Czh e3("czh.czh", czh::Czh::file);
  auto example3 = *e3.parse();
  //std::cout << "\n" << example3.to_string(czh::node::Node::color) << std::endl;
}
int main()
{
  try
  {
    test();
  }
  catch (Error &err)
  {
    if (err.is_internal())
      std::cout << "internal:\n";
    std::cout << err.get() << std::endl;
  }
  return 0;
}
