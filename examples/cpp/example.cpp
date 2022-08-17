#include "../../src/czh.h"
#include <iostream>
#include <vector>
#include <string>

int main()
{
  try
  {
    czh::Czh e("example.czh", czh::InputMode::nonstream);
    // or czh::Czh e("example.czh", czh::InputMode::stream);
    // or czh::Czh e("example: a = 1; end;",  czh::InputMode::string);
    auto example = *e.parse();
    
    //get value
    std::cout << "double: " << example["example"]["double"].get<double>();
    std::cout << "\nref: " << example["example"]["block"]["ref0"].get<int>();
    
    std::cout << "\nexample any array: ";
    auto arr = example["example"]["any_array"].get<czh::value::Value::AnyArray>();
    for (auto &r: arr)
      visit([](auto &&i)
            { std::cout << czh::node::to_czhstr(i, czh::node::Color::no_color) << ", "; }, r);
    
    //value_map
    std::cout << "\nvalue_map: ";
    auto vmap = *example["example"]["value_array_map"].value_map<std::vector<int>>();
    for (auto &r: vmap)
      for (auto &a: r.second)
        std::cout << a << ",";
    
    //edit
    example["example"]["double"].get_value() = "edit example";
    //rename
    example["example"]["double"].rename("edit");
    //add Value
    example["example"].add("add_test", "123", "edit");
    //remove Value
    example["example"]["string"].remove();
    //clear Node
    example["example"]["value_array_map"].clear();
    //add Node
    example["example"].add_node("ref", "block")
        .add("ref", example["example"]["edit"].make_ref());
    //remove Node
    example["example"]["value_map"].remove();
    
    //output to file
    std::ofstream out("output.czh");
    out << example;
    out.close();
    //output(hightlight)
    czh::Czh czh("czh.czh", czh::InputMode::nonstream);
    std::cout << "\n" << czh.parse()->to_string(czh::node::Color::with_color) << std::endl;
    
    //test
    czh::Czh("output.czh", czh::InputMode::nonstream).parse();
    auto p = czh::Czh("onelinetest.czh", czh::InputMode::stream).parse();
    czh::Czh(p->to_string(), czh::InputMode::string).parse();
  }
  catch (czh::error::Error &err)
  {
    std::cout << "internal:\n" << err.get_content() << std::endl;
  }
  catch (czh::error::CzhError &err)
  {
    std::cout << err.get_content() << std::endl;
  }
  return 0;
}
