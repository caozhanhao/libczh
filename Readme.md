# czh-cpp

#### 介绍
- czh是caozhanhao设计的一个简单的数据序列化格式
- [更多介绍](https://gitee.com/cmvy2020/czh-cpp/blob/main/docs/%E4%BB%8B%E7%BB%8D.md)

#### 语法
- [语法](https://gitee.com/cmvy2020/czh-cpp/blob/main/docs/%E8%AF%AD%E6%B3%95.md)

#### 使用方法
- [使用方法](https://gitee.com/cmvy2020/czh-cpp/blob/main/docs/czh-cpp%E4%BD%BF%E7%94%A8%E6%96%B9%E6%B3%95.md)

#### 示例
- [czh-cpp示例](https://gitee.com/cmvy2020/czh-cpp/blob/main/example.cpp)
- [czh示例1](https://gitee.com/cmvy2020/czh-cpp/blob/main/example.czh)
- [czh示例2](https://gitee.com/cmvy2020/czh-cpp/blob/main/onelinetest.czh)


#### 更新
###### 2022.4.8
- 重构

###### 2021.5.3
- 原RESULT类改名为VALUE，VALUE的存储是一个ANY类
- 原as函数改名为get_value,去除转化类型的功能
- 缩进由一个TAB改为两个空格

###### 2021.4.10
- 改为header only

###### 2021.4.3
- 修改`.add()`
- 增加了添加值的方法
- 增加了写入文件的方法(重载了<<)

###### 2021.4.2
- 修复若干bug
- 添加`.value_map()`

###### 2021.3.28
- 更新了错误显示
- 更新了`.as()`

###### 2021.3.27
- 功能有些简陋，仍在更新
- 现在没有写入文件的接口，只能读和输出，可以修改但是不能写入
- 还几个月中考，更新可能会很慢

