# czh-cpp使用方法

### Node
##### Node::operator[]
- 进入Node
- 返回类型为Node

### Value
##### Node::get_value()
- 返回Value&，可修改值
- 仅存储值的Node可调用
##### Node::get<T>()
- 获取具体类型的值
- 当值为array时，用Node::get<std::vector<T>>
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
