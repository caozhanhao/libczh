#include "../../src/czh.hpp"
#include "example.hpp"
#include <iostream>
#include <vector>
#include <string>

int main()
{
  /*
   *
   *   Parse
   *
   */
  czh::Czh e("examples/czh/example.czh", czh::InputMode::stream);
  // or czh::Czh e("example.czh", czh::InputMode::stream);
  // or czh::Czh e("example: a = 1; end;",  czh::InputMode::string);
  auto nodeptr = e.parse();// returns nullptr when czh has errors.
  if (nodeptr == nullptr) return -1;
  auto &node = *nodeptr;
  
  
  /*
   *
   *   Get value
   *
   */
  auto str = node["czh"]["😀UTF示例"].get<std::string>();
  // array
  auto arr1 = node["czh"]["any_array"].get<czh::value::Array>();
  // czh::value::Array is a std::vector contains std::variant
  auto arr2 = node["czh"]["int_array"].get<EgContainer>();
  // The T must be a container that has insert(), end(), default constructor and value_type.
  // Most containers in STL can be used directly.
  // EgContainer is defined in example.hpp
  
  // value_map
  // When the values under the same Node are of the same type,
  // you can use value_map() to get a std::map consisting of all
  // keys and values.
  auto vmap = node["czh"]["value_array_map"].value_map<std::vector<int>>();
  
  //iterator
  std::cout << "\n";
  for (auto &r: node["czh"])
  {
    // something
  }
  
  /*
   *
   *   Modify
   *
   */
  node["czh"]["double"] = "edit example";    // const char[] -> std::string
  node["czh"]["int_array"] = {1, 2, 3};      // braced initializer list
  node["czh"]["int_array"] = EgRange(1, 10); // containers that have begin(), end() and value_type
  // When the type of Array is not unique, use czh::value::Array
  node["czh"]["any_array"] = czh::value::Array{false, 1, "2", 3.0};
  
  // modify Reference
  node["czh"]["block"]["d"] = "d changed";
  // add Reference
  node["czh"].add("ref", node["czh"]["int"].make_ref());
  // add Node
  node["czh"].add_node("ref", "block");
  // add Value, add(name, value, before)
  node["czh"].add("add_test", EgRange(10, 15), "int");
  // rename
  node["czh"]["double"].rename("edit");
  // remove
  node["czh"]["bool_array"].remove();
  // clear
  node["czh"]["value_array_map"].clear();
  
  /*
   *
   *   Output
   *
   */
  // operator<< or Node::to_string()
  std::fstream output_file("examples/czh/output.czh", std::ios_base::out);
  output_file << node;
  // or output_file << node.to_string();
  
  // prettify
  czh::Czh preety_file("examples/czh/czh.czh", czh::InputMode::stream);
  auto pretty_ptr = preety_file.parse();
  if (pretty_ptr == nullptr) return -1;
  std::cout << pretty_ptr->to_string(czh::node::Color::with_color) << std::endl;
  // to_string(czh::node::Color::with_color) will use ANSI Escape Code to colorify the czh.
  // So don't write this czh to the file, otherwise it won't be able to be parsed.
  
  return 0;
}