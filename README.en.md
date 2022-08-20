# libczh

![example](examples/example.png)
- [README zh](README.md)
- [README en](README.en.md)
## Introduction

- czh is a simple data serialization format designed by caozhanhao.
- It was created on March 27, 2021

## Syntax

### Data type

- `int`,`double`,`string`,`bool`,`array`,`ref`

### Statement

- Indentation is not required
- The `;` after the statement is not required
- Connect them with an equal sign

### Note

- `<xxxx>`

### Node

- Node names are not repeatable
- Use `id: end` as a Node
- Use `id = xxx` as a Node that contains Value

### Array

- Use `{}` as an Array.
- Elements are connected by ','.

### Reference

- Use `id = key` as a Reference
- Use `id = key` as a Reference
- The scope of the Reference is connected by '::'
- `::` at the beginning represent the global scope
- The Reference can only reference previously defined keys
- 如下

```
example:
a = 1
end;
block:
    example:
    a = 2;
    end;    
    b = 3
    c = ::example::a <c == 1>
    d = example::a   <d == 2>
    e = b            <e == 3>
end;
```

## Usage of libczh

### Compile

- just `#include "czh.h"`
- Requires C++ 17

### Czh

#### Czh::Czh(str, mode)

##### mode

- `czh::InputMode::stream` -> The first parameter is the file name
- `czh::InputMode::nonstream` -> The first parameter is the file name
- `czh::InputMode::string` -> The first parameter is a `std::string` where czh is stored

```c++
  Czh("example: a = 1; end;", czh::InputMode::string);
```

### Node

##### Node::operator[str]

- Returns a Node named str

##### Node::get<T>()

- Get the value
- Use `Node::get<std::vector <T>>` when the value is Array.
- Only Nodes that contains Value can call

```c++
int a = example["czh"]["this_is_a_int"].get<int>();
```

### value_map

- When the values under the same Node are of the same type, you can use value_map() to get a std::map consisting of all
  keys and values.

##### Node::value_map<T>()

- Returns std::map<std::string, T>
- Only Nodes that contains Value can call

```c++
auto vmap = example["example"]["valmap"].value_map<vector<int>>();
```

### Modify

- After modification, you can use the `operator <<` or `Node::to_string()` to update the file

#### Add

##### Node::add(key, value, before)

- Add before Node named `before`
- `before` defaults to empty, which will add at the end
- Returns a reference to the added Node(contains Value).

```c++
example["add"].add("add", "123", "edit");
```

```
add = 123
edit = xxx
```

##### Node::make_ref()

- Get the "reference" of the Node to add a Reference in czh
- Don't add the Reference before the referenced Node defined

```c++
 example["example"].add("ref", example["example"]["i"].make_ref());
```

```
i = 0
ref = i
```

##### Node::add_node(name, before)

- Add before Node named `before`
- Returns a reference to the added Node(contains Node).

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

##### Node::remove()

- Remove the Node

```c++
example["example"].remove();
```

#### Clear

##### Node::clear()

- Clear all Nodes

```c++
example["example"].clear();
```

#### Rename

##### Node::rename(name, newname)

- Rename the Node to `newname`

```c++
example["a"].rename("b");
```

#### Change the value

##### Node::operator=(value)

- Only Nodes that contains Value can call

```c++
example["czh"]["edit"] = "edit example";
```

### Output

- The output of czh does not contain any Notes.

##### Node::to_string(with_color)

- Returns the formatted czh
- `czh::node::Color::with_colo` -> with highlight
- `czh::node::Color::no_color` -> without highlight
- Do not write highlighted czh to the file, otherwise it will not be able to be parsed.

##### operator<<

- Output `Node::to_string()` return value
- Without highlight

#### Demo

- [libczh demo](https://gitee.com/cmvy2020/libczh/blob/master/examples/cpp/example.cpp)
- [czh demo1](https://gitee.com/cmvy2020/libczh/blob/master/examples/czh/example.czh)
- [czh demo2](https://gitee.com/cmvy2020/libczh/blob/master/examples/czh/czh.czh)
- [czh demo3](https://gitee.com/cmvy2020/libczh/blob/master/examples/czh/onelinetest.czh)
- [czh demo4](https://gitee.com/cmvy2020/wxserver/blob/main/config.czh)