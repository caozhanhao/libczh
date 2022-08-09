# czh-cpp
![example](https://gitee.com/cmvy2020/czh-cpp/raw/master/examples/example.png)
## 介绍
- czh是caozhanhao设计的一个简单的数据序列化格式
- 创建于 2021年3月27日
## 语法
### 数据类型
- `int`,`double`,`string`,`bool`,`array`,`ref`
### 语句
- 可以没有缩进
- 语句后可以有`;`但不是必需的
- 使用`=`连接key和value
### 注释
- `/b/ xxxx /e/`
### Node
- 使用`id:`作为一个Node的开始，使用`end`表示结束
### array
- 使用`[]`作为array,元素由`,`连接
### 引用
- 使用`-xxx-xxx:`表示引用
- 引用的作用域由`-`连接，key由`:`连接
- 只可以引用前面已定义的key
- 引用中可使用`.`表示本作用域,`..`表示上级作用域
## czh-cpp使用方法
### 编译
- `#include "czh.h"`即可
- 需要C++17
### Node
##### Node::operator[]
- 进入Node
- 返回类型为Node
### Value
##### Node::get_value()
- 返回Value&，可修改值
- 仅存储值的Node可调用
##### Node::view_char<T>()
- 获取具体类型的值
- 当值为array时，用Node::view_char<std::vector<T>>
- 仅存储值的Node可调用
### value_map
- 同一Node下的值的类型相同时时，可以使用value_map()获取所有key和value组成的map
##### Node::value_map<T>()
- 返回std::map<std::string, T>
- 仅存储节点的Node可调用
### 修改
- 修改后可使用operator<<或Node::to_string()以更新文件
#### 添加
##### Node::add(key, value, before)
- 在名为'before'的Node前添加
- before默认为空，此时添加在末尾
#### 删除
##### Node::remove(name)
- 删除该Node下名为name的Node
#### 清除
##### Node::clear()
- 清除该Node下所有的Node
#### 重命名
##### Node::rename(name, newname)
- 将该Node下名为name的Node改为newname
### 输出
- 输出的czh仍保留注释
##### Node::to_string(with_color)
- 返回格式化后的czh
- 传入Node::color时，返回有高亮的czh，默认无高亮
- 不要将高亮的czh写入文件，否则无法解析
##### operator<<
- 输出Node::to_string()返回值
- 无高亮
#### 示例
- [czh-cpp示例](https://gitee.com/cmvy2020/czh-cpp/blob/master/examples/cpp/example.cpp)
- [czh示例1](https://gitee.com/cmvy2020/czh-cpp/blob/master/examples/czh/example.czh)
- [czh示例2](https://gitee.com/cmvy2020/czh-cpp/blob/master/examples/czh/onelinetest.czh)
- [czh示例](https://gitee.com/cmvy2020/wxserver/blob/main/config.czh)