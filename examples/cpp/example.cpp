#include "../../include/libczh/czh.hpp"
#include "example.hpp"
#include <iostream>
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
  
  // Node::get<T>
  auto str = node["czh"]["😀UTF示例"].get<std::string>();
  if (!node["czh"]["null_example"].is<czh::value::Null>())
  {
    // node["czh"]["null_example"].get<xxx>();
  }
  // When Value is an Array,
  // the T must be a container that has insert(), end(),
  // default constructor and value_type which is in
  // int, long long, double, std::string, bool [aka. BasicVTList(value.hpp)]
  // For example, Container in example.hpp and
  // most containers in STL can meet these requirements.
  auto arr1 = node["czh"]["int_array"].get<Container>();
  // When the elements in the Array are not of the same type,
  // Use czh::value::Array (std::vector<std::variant<BasicVTList(see above)>>)
  auto arr2 = node["czh"]["any_array"].get<czh::value::Array>();
  
  // Node::value_map<T>
  // When the values under the same Node are of the same type,
  // you can use value_map() to get a std::map consisting of all
  // keys and values.
  // T is consistent with the requirements of Array above
  auto vmap = node["czh"]["value_array_map"].value_map<Container>();
  
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
  node["czh"]["double"] = "edit example";          // const char[] -> std::string
  node["czh"]["block"]["d"] = "d changed";         // modify Reference
  node["czh"]["int_array"] = {1, 2, 3};            // braced initializer list
  node["czh"]["any_array"] = {false, 1, "2", 3.0}; // czh::value::Array(see above)
  node["czh"]["int_array"] = Range(1, 10);         // containers that have begin(), end() and value_type
  
  // Add Node
  // Node::add_node(name, before) will add a Node before "before"
  node["czh"].add_node("ref", "block");
  // Add Value
  // Node::add(name, value, before)
  node["czh"].add("add_test", Range(10, 15), "int");
  node["czh"].add("ref", node["czh"]["int"]);
  
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