# czh-cpp使用方法

### 作用域
- 用`in()`进入作用域，用`[]`获取值
- `[]`获取的类型为`VALUE`
### 获取值
#### get_value函数
- 获取VALUE的值

### 获取map
- 同一作用域的值的类型相同时时，可以使用value_map()获取作用域的map
#### value_map函数
- 获取一个NODE中的值的map

###修改文件
- 注意，无论是修改还是添加值，如果想要应用到文件中，需要`update_text()`
#### 修改值
- 直接对VALUE对象使用`=`即可
#### 添加值
- 对node对象使用add函数
##### add函数
- 需要两个参数
- 第一个是名字
- 第二个是具体的值，例`std::vector<std::string>({ "ex", "am", "ple" })`