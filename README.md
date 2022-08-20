# libczh
![example](examples/example.png)
- [README zh](README.md)
- [README en](README.en.md)
## 介绍
- czh是caozhanhao设计的一个简单的数据序列化格式
- 创建于 2021年3月27日
## 语法
### 数据类型
- `int`,`double`,`string`,`bool`,`array`,`ref`

### 语句

- 可以没有缩进
- 语句后可以有`;`但不是必需的

### 注释

- `<xxxx>`

### Node

- Node名不可重复
- 使用`id: end`作为一个Node
- 使用`id = xxx`作为一个存储值的Node

### Array

- 使用`{}`作为Array
- 元素由`,`连接

### 引用

- 使用`id = key`表示引用
- 引用的作用域由`::`连接
- `::`在开头时为全局作用域
- 只可以引用前面已定义的值
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

## libczh使用方法
### 编译
- `#include "czh.h"`即可
- 需要C++17

### Czh

#### Czh::Czh(str, mode)

##### mode

- `czh::InputMode::stream` -> 第一个参数为文件名
- `czh::InputMode::nonstream`->第一个参数为文件名
- `czh::InputMode::string`-> 第一个参数为存储czh的`std::string`

```c++
  Czh("example: a = 1; end;", czh::InputMode::string);
```

### Node

##### Node::operator[str]

- 返回名为str的Node

##### Node::get<T>()

- 获取具体类型的值
- 当值为array时，用`Node::get<std::vector<T>>`
- 仅存储值的Node可调用

```c++
int a = example["czh"]["this_is_a_int"].get<int>();
```

### value_map

- 同一Node下的值的类型相同时时，可以使用value_map()获取所有key和value组成的map

##### Node::value_map<T>()
- 返回std::map<std::string, T>
- 仅存储节点的Node可调用
```c++
auto vmap = example["example"]["valmap"].value_map<vector<int>>();
```
### 修改

- 修改后可使用`operator<<`或`Node::to_string()`以更新文件
#### 添加
##### Node::add(key, value, before)

- 在名为`before`的Node前添加
- `before`默认为空，此时添加在末尾
- 返回添加的Node(存储Value)的引用
```c++
example["add"].add("add", "123", "edit");
```
```
add = 123
edit = xxx
```
##### Node::make_ref()

- 获取该Node的”引用“，用以在czh中添加引用
- 不可在被引用对象之前添加
```c++
 example["example"].add("ref",example["example"]["i"].make_ref());
```
```
i = 0
ref = i
```
##### Node::add_node(name, before)
- 在名为'before'的Node前添加
- 返回添加的Node的引用
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
#### 删除
##### Node::remove()
- 删除该Node
```c++
example["example"].remove();
```
#### 清除
##### Node::clear()
- 清除该Node下所有的Node
```c++
example["example"].clear();
```
#### 重命名
##### Node::rename(name, newname)

- 将该Node改为`newname`
```c++
example["a"].rename("b");
```
#### 更改值
##### Node::operator=(value)
- 仅储存值Node可调用
```c++
example["czh"]["edit"] = "edit example";
```
### 输出
- 输出的czh不保留注释
##### Node::to_string(with_color)
- 返回格式化后的czh
- czh::node::Color::with_color 有高亮
- czh::node::Color::no_color 无高亮(默认)
- 不要将高亮的czh写入文件，否则无法解析
##### operator<<
- 输出Node::to_string()返回值
- 无高亮
#### 示例

- [libczh示例](https://gitee.com/cmvy2020/libczh/blob/master/examples/cpp/example.cpp)
- [czh示例1](https://gitee.com/cmvy2020/libczh/blob/master/examples/czh/example.czh)
- [czh示例2](https://gitee.com/cmvy2020/libczh/blob/master/examples/czh/czh.czh)
- [czh示例3](https://gitee.com/cmvy2020/libczh/blob/master/examples/czh/onelinetest.czh)
- [czh示例4](https://gitee.com/cmvy2020/wxserver/blob/main/config.czh)