#include "../../src/czh.h"
#include <iostream>
#include <vector>
#include <set>
#include <list>
#include <string>

int main()
{
  czh::Czh e("examples/czh/example.czh", czh::InputMode::stream);
  // or czh::Czh e("example.czh", czh::InputMode::stream);
  // or czh::Czh e("example: a = 1; end;",  czh::InputMode::string);
  auto nodeptr = e.parse();
  if (nodeptr == nullptr) return -1;
  auto &node = *nodeptr;
  //get value
  std::cout << "double: " << node["czh"]["double"].get<double>() << std::endl;
  std::cout << "字符串: " << node["czh"]["字符串"].get<std::string>() << std::endl;
  
  //array
  std::cout << "\narray: ";
  auto arr = node["czh"]["any_array"].get<czh::value::Array>();
  for (auto &r: arr)
  {
    visit([](auto &&i)
          { std::cout << czh::node::to_czhstr(i, czh::node::Color::no_color) << ", "; }, r);
  }
  auto arr1 = node["czh"]["int_array"].get<std::vector<int>>();
  auto arr3 = node["czh"]["int_array"].get<std::list<int>>();
  auto arr2 = node["czh"]["int_array"].get<std::set<int>>();
  //containers that have insert() and end()
  //value_map
  std::cout << "\nvalue map: ";
  auto vmap = node["czh"]["value_array_map"].value_map<std::set<int>>();
  auto vmap1 = node["czh"]["value_array_map"].value_map<std::vector<int>>();
  for (auto &r: vmap)
  {
    for (auto &a: r.second)
    {
      std::cout << a << ",";
    }
  }
  //iterator
  std::cout << "\n";
  for (auto &r: node["czh"])
  {
    std::cout << r.get_name() << " ";
  }
  //edit
  node["czh"]["double"] = "edit example";
  node["czh"]["int_array"].get_value() = {1, 2, 3};//braced initializer list
  node["czh"]["int_array"].get_value() = std::vector{1, 2, 3};
  node["czh"]["int_array"].get_value() = std::set{1, 2, 3};
  node["czh"]["any_array"].get_value() = czh::value::Array{1, "2", 3.0};
  //containers that have begin() and end()
  
  //edit ref
  node["czh"]["block"]["d"] = "d changed";
  //rename
  node["czh"]["double"].rename("edit");
  //add Value
  node["czh"].add("add_test", "123", "edit");
  //remove Value
  node["czh"]["字符串"].remove();
  //clear Node
  node["czh"]["value_array_map"].clear();
  //add Node
  node["czh"].add_node("ref", "block")
      .add("ref", node["czh"]["edit"].make_ref());
  //remove Node
  node["czh"]["value_map"].remove();
  
  //output to file
  std::fstream outputczh("examples/czh/output.czh", std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
  outputczh << node;
  //output(hightlight)
  czh::Czh czh("examples/czh/czh.czh", czh::InputMode::stream);
  auto czhp = czh.parse();
  if (czhp == nullptr) return -1;
  std::cout << "\n" << czhp->to_string(czh::node::Color::with_color) << std::endl;
  
  //test
  //output.czh
  if (czh::Czh("examples/czh/output.czh", czh::InputMode::nonstream).parse() == nullptr) return -1;
  //output.czh -> onelinetest.czh
  std::fstream onelineczh("examples/czh/onelinetest.czh", std::ios_base::out | std::ios_base::trunc);
  std::string tmp;
  outputczh.clear();
  outputczh.seekg(std::ios_base::beg);
  while (std::getline(outputczh, tmp))
  {
    onelineczh << tmp << " ";
  }
  onelineczh.close();
  auto p = czh::Czh("examples/czh/onelinetest.czh", czh::InputMode::nonstream).parse();
  if (p == nullptr) return -1;
  //onelinetest.czh -> string
  czh::Czh(p->to_string(), czh::InputMode::string).parse();
  outputczh.close();
  return 0;
}