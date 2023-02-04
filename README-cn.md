<h2 align="center">
libczh
</h2> 

<p align="center">
<strong>一个简单易用的数据序列化格式</strong>
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

## 使用方法

### 语法

#### 类型

-   `int`,`double`,`string`,`bool`,`Array`,`Reference`

#### 语句

- 可以没有缩进
- 语句后可以有`;`, 但不是必需的

#### 注释

-   `<xxxx>`

#### Node

- 名字不可重复
- Node: `id: end`
- Value: `id = xxx`

#### 数组

- `{}` .

#### 引用

-   `id = a::b::c::id`

### libczh使用方法

#### 配置

-   只需 `#include "libczh/czh.hpp"`!
-   要求 C++ 20

#### Czh::Czh(str, mode)

##### 模式

-   `czh::InputMode::file`   -> `str` 是路径
-   `czh::InputMode::string` -> `str` 是一个存储`czh`的字符串

```c++
  Czh("example: a = 1; end;", czh::InputMode::string);
```

#### Node::operator[str]

- 返回名为str的Node。

#### Node::operator(str)

- 与Node::operator[str]相似，但提供更好的错误提示。

#### Node::get<T>()

- 当czh中数组存储的数据类型不唯一时，`T`必须是`czh::value::Array`

```c++
auto arr = node["czh"]["any_array"].get<czh::value::Array>();
```

#### value_map

-  同一Node下的值的类型相同时时，使用`value_map()`获取一个存储了所有key和value的`std::map`

#### Node::value_map<T>()

- 返回 `std::map<std::string, T>`

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

#### 添加

##### Node::add(key, value, before)

- 在名为 `before`的Node前添加一个值为`value`的Node
- `before` 默认为空，此时添加在末尾
- 返回添加的Node的引用

```c++
example["add"].add("add", "123", "abc");
```

##### Node::add_node(name, before)

- 在名为 `before`的Node前添加一个名为`name`的Node
- 返回添加的Node的引用

```c++
example.add_node("new", "before");
```

#### 删除

##### Node::remove()

```c++
example["example"].remove();
```

#### 清空

##### Node::clear()

```c++
example["example"].clear();
```

#### 重命名

#### Node::rename(name, newname)

```c++
example["a"].rename("b");
```

#### 输出

##### Writer

- libczh 原生支持三个`writer`

| Writer        | 格式                         |
|---------------|----------------------------|
| BasicWriter   | 无格式化                       |
| PrettyWriter  | 格式化                        |
| ColorWriter   | 格式化 + 高亮(ANSI Escape Code) |

##### Node::accept()

- 接受一个 `Writer`

```c++
    writer::BasicWriter<std::ostream> w{ std::cout };
node.accept(w);
```

##### operator<<

- 等同于 `BasicWriter`

##### 写一个Writer

- 我们只需写一个满足如下concept的类。

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

## 联系

- 如果你有任何问题或建议，请提交一个issue或给我发邮件
- 邮箱: cao2013zh at 163 dot com

## 贡献

- 任何贡献都是受欢迎的，只需提一个PR

## 许可

- libczh 根据 [Apache-2.0 license](LICENSE)获得许可
