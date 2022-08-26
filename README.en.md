# libczh [![](https://img.shields.io/github/license/caozhanhao/libczh?style=flat-square)](LICENSE)  ![](https://img.shields.io/github/v/release/caozhanhao/libczh?style=flat-square)

![example](examples/example.png)

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
-   The Reference can only reference previously defined keys

### Usage of libczh

#### Compile

-   just `#include "czh.hpp"`
-   Requires C++ 17

#### Czh

##### Czh::Czh(str, mode)

###### mode

-   `czh::InputMode::stream` -> The first parameter is the file name
-   `czh::InputMode::nonstream` -> The first parameter is the file name
-   `czh::InputMode::string` -> The first parameter is a `std::string` where czh is stored

```c++
  Czh("example: a = 1; end;", czh::InputMode::string);
```

#### Node

###### Node::operator[str]

-   Returns a Node named str

###### Node::get<T>()

-   Get the Value
-   Only Node can call
-   When the Value is Array, most containers in STL can be used directly.W
-   When T is a custom type, T meets the following requirements
-   have `insert()`, `end()` and default constructor
-   have a member named `value_type` to describe a type
-   The type inside the container is the czh type except Array and Reference
-   When the type of Array is not unique, T must be czh::value::Array
```c++
auto arr = node["czh"]["any_array"].get<czh::value::Array>();
```

#### value_map

-   When the values under the same Node are of the same type, you can use value_map() to get a std::map consisting of
    all keys and values.

###### Node::value_map<T>()

-   Returns std::map<std::string, T>
-   Only Node can call

```c++
auto vmap = example["example"]["valmap"].value_map<vector<int>>();
```

#### Value

###### Node::operator=(value)

-   Only Value can call
-   Like `Node::get<T>`， When Value is Array, T meets the following requirements
-   have member functions `begin()`、`end()`
-   have a member named `value_type` to describe a type
-   Can use `std::initializer_list` directly
-   When the type of Array is not unique, use `czh::value::Array`

```c++
node["czh"]["int_array"] = Range(1, 10);//begin() end() value_type
node["czh"]["int_array"] = {1, 2, 3};      
node["czh"]["any_array"] = {false, 1, "2", 3.0};//czh::value::Array
```

#### Modify

-   After modification, you can use the `operator <<` or `Node::to_string()` to update the file

##### Add

###### Node::add(key, value, before)

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
###### Node::add_node(name, before)

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

##### Remove

###### Node::remove()

-   Remove the Node

```c++
example["example"].remove();
```

##### Clear

###### Node::clear()

-   Clear all Nodes

```c++
example["example"].clear();
```

##### Rename

###### Node::rename(name, newname)

-   Rename the Node to `newname`

```c++
example["a"].rename("b");
```

#### Output

-   The output of czh does not contain any Notes.

###### Node::to_string(with_color)

-   Returns the formatted czh
-   `czh::node::Color::with_color` -> with highlight
-   `czh::node::Color::no_color`   -> no highlight
-   Do not write highlighted czh to the file, otherwise it will not be able to be parsed.

###### operator<<

-   Output `Node::to_string()` return value
-   Without highlight
