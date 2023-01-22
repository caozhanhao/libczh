#include "libczh/czh.hpp"
#include "example.hpp"
#include <iostream>
#include <string>
#include <ranges>

int main()
{
  /*
   *
   *   Parse
   *
   */
  czh::Czh e("examples/czh/example.czh", czh::InputMode::file);
  // or czh::Czh e("example.czh", czh::InputMode::stream);
  // or czh::Czh e("example: a = 1; end;",  czh::InputMode::string);
  auto node = e.parse();// returns nullptr when czh has errors.
  
  /*
   *
   *   Get value
   *
   */
  
  // Node::operator[]/() and Node::get<T>
  // operator() is similar with operator[], but it provides a better error message.
  
  auto str = node["czh"]["😀UTF示例"].get<std::string>();
  if (!node["czh"]["null_example"].is<czh::value::Null>())
  {
    // node["czh"]["null_example"].get<xxx>();
  }
  // When Value is an Array,
  // the T must be a container whose value is convertible to
  // the czh type (except Array and Reference)
  // (aka. int, long long, double, std::string, bool)
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
  node["czh"]["int_array"] = std::ranges::views::iota(1, 10); // or std::ranges
  
  // Add Node
  // Node::add_node(name, before) will add a Node before "before"
  node["czh"].add_node("ref", "block");
  // Add Value
  // Node::add(name, value, before)
  node["czh"].add("add_test1", Range(10, 15), "int");
  //ranges
  node["czh"].add("add_test2", std::ranges::views::iota(15, 20), "int");
  
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
  // writer
  std::fstream output_file("examples/czh/output.czh", std::ios_base::out);
  output_file << node;
  // or
  // czh::BasicWriter<std::fstream> fw{output_file};
  // node.accept(fw);
  output_file.close();
  
  using namespace czh::literals;
  // literal
  auto pretty = R"(
< note >
czh:
    int = +6e+2
    longlong = +6e+12
    😀 = "字符串"
    double = +6e-2
    ref = int
    array = {false, 1.0, "2", 3}
    arrays:
        k1 = {1,2,3}
        k2 = {4,5,6}
    end
end
)"_czh;
  // ColorWriter -> PrettyWriter + color, use PrettyWriter to disable color.
  czh::ColorWriter<std::ostream> cw{std::cout};
  pretty.accept(cw);
  return 0;
}