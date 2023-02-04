<h2 align="center">
libczh
</h2> 

<p align="center">
<strong>An easy-to-use data serialization format</strong>
</p>

<p align="center">
  <a href="https://github.com/caozhanhao/libczh/actions/workflows/tests.yml" >
    <img src="https://img.shields.io/github/actions/workflow/status/caozhanhao/libczh/tests.yml?style=flat-square" alt="build" />  
  </a>
  <a href="LICENSE" >
    <img src="https://img.shields.io/github/license/caozhanhao/libczh?label=License&style=flat-square&color=yellow" alt="License" />  
  </a>
  <a href="https://github.com/caozhanhao/libczh/releases" >
    <img src="https://img.shields.io/github/v/release/caozhanhao/libczh?label=Release&style=flat-square&color=orange" alt="Release" />  
  </a>
</p>

<p align="center">
  <a href="README.md" >
    <img src="https://img.shields.io/badge/Document-English-blue" alt="Document" />  
  </a>
  <a href="README-cn.md" >
    <img src="https://img.shields.io/badge/文档-简体中文-blue" alt="Document" />  
  </a>
</p>

<p align="center">
  <a href="examples/example.png" >
    <img src="examples/example.png" alt="Example" />  
  </a>
</p>

## Tutorial

### Syntax

#### Type

-   `int`,`double`,`string`,`bool`,`Array`,`Reference`

#### Statement

-   Indentation is not required
-   The `;` after the statement is not required

#### Comment

-   `<xxxx>`

#### Node

- Duplicate names are not allowed
- Node: `id: end`
- Value: `id = xxx`

#### Array

- `{}` .

#### Reference

-   `id = a::b::c::id`

### Usage of libczh

#### Setup

-   just `#include "libczh/czh.hpp"`!
-   Requires C++ 20

#### Czh::Czh(str, mode)

##### mode

-   `czh::InputMode::file`   -> `str` is a path
-   `czh::InputMode::string` -> `str` is a `std::string` where czh is stored

```c++
  Czh("example: a = 1; end;", czh::InputMode::string);
```

#### Node::operator[str]

- Returns a Node named str

#### Node::operator(str)

- Similar to `Node::operator[str]`, but it provides a better error message.

#### Node::get<T>()

- When the Array value's type in czh is not unique, T must be czh::value::Array

```c++
auto arr = node["czh"]["any_array"].get<czh::value::Array>();
```

#### value_map

-   When the values under Node are of the same type, use `value_map()` to get a `std::map` consisting of all ids and
    values.

#### Node::value_map<T>()

- Returns `std::map<std::string, T>`

```c++
auto value_map = example["example"]["arrays"].value_map<vector<int>>();
```

```
example: 
  arrays:
    a = {1,2,3}
    b = {1,2,3}
  end
end
```

#### Node::operator=(value)

```c++
node["czh"]["int_array"] = Range(1, 10);        // custom container
node["czh"]["int_array"] = std::ranges::views::iota(1,10); // std::ranges
node["czh"]["int_array"] = {1, 2, 3};           // brace-enclosed initializer list
node["czh"]["any_array"] = {false, 1, "2", 3.0};// czh::value::Array
```

#### Add

##### Node::add(key, value, before)

- Add a new Value named `key` whose value is `value` before the Node named `before`. -`before` defaults to be empty,
  which will add at the end.
- Returns a reference to the added Node.

```c++
example["add"].add("add", "123", "abc");
```

##### Node::add_node(name, before)

-   Add a new Node named `name` before the Node named `before`.
-   Returns a reference to the added Node.

```c++
example.add_node("new", "before");
```

#### Remove

##### Node::remove()

```c++
example["example"].remove();
```

#### Clear

##### Node::clear()

```c++
example["example"].clear();
```

#### Rename

#### Node::rename(name, newname)

```c++
example["a"].rename("b");
```

#### Output

##### Writer

- libczh originally support three writers

| Writer        | Format                               |
|---------------|--------------------------------------|
| BasicWriter   | No Format                            |
| PrettyWriter  | Format                               |
| ColorWriter   | Format + Highlight(ANSI Escape Code) |

##### Node::accept()

- accept a `Writer`

```c++
    writer::BasicWriter<std::ostream> w{ std::cout };
node.accept(w);
```

##### operator<<

- equal to `BasicWriter`

##### Write a Writer

- All we need is to write a class satisfied the following concept.

```c++
template<typename T>
concept Writer =
requires(T w)
{
{ w.node_begin(std::string()) };
{ w.node_end() };
{ w.value_begin(std::string()) };
{ w.value(value::Value{}) };
{ w.value_ref_path_set_global() };
{ w.value_ref_path(std::string()) };
{ w.value_ref_id(std::string()) };
{ w.value_array_begin() };
{ w.value_array_value(value::Array::value_type{}) };
{ w.value_array_end(value::Array::value_type{}) };
{ w.value_array_end() };
};
```

## Contact

- If you have any questions or suggestions, please submit an issue or email me.
- Email: cao2013zh at 163 dot com

## Contribution

- Any contributions are welcomed, just send a PR.

## License

- libczh is licensed under the [Apache-2.0 license](LICENSE)
