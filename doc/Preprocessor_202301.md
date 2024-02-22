# PreProcessor for Simple Language (V202301)

## 概述

本语言扩展出的预处理器语法基本参考C语言的预处理器条目。语法和功能和C语言的预处理器基本一致，由于时间受限，仅实现了部分功能，包括

 - `#define` 定义宏
 - `#undef` 取消宏定义
 - `#ifdef` 条件编译
 - `#ifndef` 条件编译
 - `#if` 条件编译，能够处理常量表达式
 - `#include` 包含文件，并进行递归预处理和异常处理。

## 语法

### 宏定义

```
#define NOW_YOU_SEE_ME
#define ARTICLE_LEN 233
#define ARTICLE_TITLE "Preprocessor doc"

#undef NOW_YOU_DONT
```

定义宏。

由于时间关系，暂时未实现代码文本替换的功能。

我们内部认为宏有几种类型，分别是
    
- 字符串宏
- 数字宏
- 没有值的简单宏

### 条件编译

```
#ifdef SIMPLE_FLAG
...
#ifndef ANOTHER_FLAG
...
#if AYACHI and ALICHI or (true or false and THAT_S_A_QUESTION)
...
#endif
```

条件编译语句，`#ifdef` 和 `#ifndef` 用于判断宏是否被定义。

`#if` 用于判断常量表达式的真假。求值的行为如下：

 - 若某个宏定义的类型为字符串或简单宏，则其值为1。
 - 若某个宏定义的类型为数字宏，则取其值是否非零作真假。
 - 若某个宏未定义，则其值为0。

以上条件可以嵌套使用。

### 文件包含

```
#include {relative/path/to/file}
```

包含文件，包含文件时会自动对包含的文件运行预处理器。预处理器会处理和打开文件相关的异常。

包含文件的路径应当包括在大括号中。