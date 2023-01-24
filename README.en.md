# libczh  [![Unit Test](https://img.shields.io/github/actions/workflow/status/caozhanhao/libczh/tests.yml?style=flat-square)](https://github.com/caozhanhao/libczh/actions/workflows/tests.yml) [![License](https://img.shields.io/github/license/caozhanhao/libczh?label=License&style=flat-square)](LICENSE) ![](https://img.shields.io/github/v/release/caozhanhao/libczh?label=Release&style=flat-square)

![example](examples/example.png)

-   [README zh](README.md)
-   [README en](README.en.md)
-   [简体中文主页](https://libczh.vercel.app/)
-   [English Page](https://libczh-en.vercel.app/)

## Introduction

-   czh is a simple data serialization format designed by caozhanhao.

## Tutorial

-   czh and libczh are simple to use, you can directly see the examples in [example.cpp](examples/cpp/example.cpp),
    which covers most of the contents of libczh.

### Syntax

#### Data type

-   `int`,`double`,`string`,`bool`,`array`,`ref`

#### Statement

-   Indentation is not required
-   The `;` after the statement is not required

#### Note

-   `<xxxx>`

#### Node

-   We use Node to represent a node and Value to represent the value under the node
-   Node and Value names are not repeatable
-   Use `id: end` as a Node
-   Use `id = xxx` as a Value

#### Array

-   Use `{}` .

#### Reference

-   Use `id = key` as a Reference
-   Use `id = key` as a Reference
-   The scope of the Reference is connected by '::'

### Usage of libczh

#### Compile

-   just `#include "czh.hpp"`
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

##### Node::operator(str)

- Similar with `Node::operator[str]`, but it provides a better error message.

##### Node::get<T>()

-   Get the Value
-   When the Value is Array, most containers in STL can be used directly.
-   When T is a custom type, its value needs to be convertible to the czh type except Array and Reference, ( int, long long, double, std::string, bool)
-   When the type of Array is not unique, T must be czh::value::Array

```c++
auto arr = node["czh"]["any_array"].get<czh::value::Array>();
```

#### value_map

-   When the values under the same Node are of the same type, you can use value_map() to get a std::map consisting of
    all keys and values.

##### Node::value_map<T>()

-   Returns std::map<std::string, T>

```c++
auto vmap = example["example"]["valmap"].value_map<vector<int>>();
```

#### Node::operator=(value)

-   Like `Node::get<T>`， When Value is Array, T meets the following requirements
-   You can use `brace-enclosed initializer list` directly
-   When the type of Array is not unique, use `czh::value::Array`

```c++
node["czh"]["int_array"] = Range(1, 10);        // custom container
node["czh"]["int_array"] = std::ranges::views::iota(1,10); // std::ranges
node["czh"]["int_array"] = {1, 2, 3};           // brace-enclosed initializer list
node["czh"]["any_array"] = {false, 1, "2", 3.0};// czh::value::Array
```

#### Modify

#### Add

#### Node::add(key, value, before)

-   Add before Node named `before`
-   `before` defaults to empty, which will add at the end
-   Returns a reference to the added Node(contains Value).

```c++
example["add"].add("add", "123", "edit");
example["add"].add("ref", example["add"]["edit"]);
```

```
add = 123
edit = xxx
i = edit
```

#### Node::add_node(name, before)

-   Add before Node named `before`
-   Returns a reference to the added Node(contains Node).

```c++
example.add_node("newnode", "before");
```

```
newnode:
end
before:
xxx
end
```

#### Remove

#### Node::remove()

-   Remove the Node

```c++
example["example"].remove();
```

#### Clear

#### Node::clear()

-   Clear all Nodes

```c++
example["example"].clear();
```

##### Rename

#### Node::rename(name, newname)

-   Rename the Node to `newname`

```c++
example["a"].rename("b");
```

#### Output

-   The output of czh does not contain any Notes.

#### Node::to_string(with_color)

- accept a `Writer`, see `writer.hpp`

#### operator<<

- equal to

```c++
    writer::BasicWriter<std::ostream> w{ os };
node.accept(w);
```
