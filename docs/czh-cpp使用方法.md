# czh-cpp使用方法

### 作用域
##### Node:::operator[]
- 进入作用域
- 返回类型为`Node`

### 获取值
##### Node::get_value()
- 返回Value&，可修改
- 仅存储值的Node可调用
##### Node::get<T>()
- 获取具体类型的值
- 仅存储值的Node可调用

### 获取map
- 同一作用域的值的类型相同时时，可以使用value_map()获取作用域下所有key和value组成的map
##### Node::value_map()
- 获取一个NODE中的值的map
- 仅存储节点的Node可调用

### 修改
- 修改值后使用operator<<，或Node::to_string()
#### 修改值
- 直接对VALUE对象使用`=`即可
#### 添加值
##### Node::add(key, value, before = "")
- 在名为'before'的Node前添加
- before为空时添加在末尾
#### 删除值
##### Node::remove()
- 删除该Node

### 输出
##### Node::to_string()
- 返回格式化后的czh字符串
##### operator<<
- 输出Node::to_string()返回值