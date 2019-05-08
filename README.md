# my_json_parser
## introduction
借鉴cJSON使用C++实现的一个JSON解析器,主要改进：使用智能指针进行内存管理。

## environment
OS:Ubuntu 18.04.2 LTS
Complier: g++ (Ubuntu 7.3.0-27ubuntu1~18.04) 7.3.0

## 功能检查
完成对string, number, array, object, false, true, null 7种JSON对象的解析和打印。


## 内存管理检查
使用valgrind进行内存检查，查看是否有内存泄露

