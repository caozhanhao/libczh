#include "../../src/czh.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

int main()
{
  try
  {
    czh::Czh e("example.czh", czh::InputMode::nonstream);
    // or czh::Czh e("example.czh", czh::InputMode::stream);
    // or czh::Czh e("example: a = 1; end;",  czh::InputMode::string);
    auto example = *e.parse();
  
    //get value
    cout << "double: " << example["example"]["double"].get<double>();
    cout << "\nref: " << example["example"]["block"]["ref0"].get<int>();
  
    cout << "\nexample any array: ";
    auto arr = example["example"]["any_array"].get<czh::value::Value::AnyArray>();
    for(auto& r : arr)
      visit([](auto&& i){cout << czh::node::to_czhstr(i, false) << ", ";}, r);
  
    //value_map
    cout << "\nvalue_map: ";
    auto vmap = *example["example"]["value_array_map"].value_map<vector<int>>();
    for (auto &r: vmap)
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
  
    //output to file
    std::ofstream out("output.czh");
    out << example;
    out.close();
    //output(hightlight)
    czh::Czh czh("czh.czh", czh::InputMode::nonstream);
    std::cout << "\n" << czh.parse()->to_string(czh::node::Node::color) << std::endl;
    
    //test
    czh::Czh("output.czh", czh::InputMode::nonstream).parse();
    auto p = czh::Czh("onelinetest.czh", czh::InputMode::stream).parse();
    czh::Czh(p->to_string(), czh::InputMode::string).parse();
  }
  catch (Error &err)
  {
    if (err.is_internal())
      std::cout << "internal:\n";
    std::cout << err.get() << std::endl;
  }
  return 0;
}
