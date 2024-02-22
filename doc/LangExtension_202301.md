# Language Extension Reference For Simple Language (V202301)

为了使得 Simple 语言 (下称：SPL) 更实用，本扩展被提出。

**注意：在下面的描述中，被引号("")括住的内容解释为字面量。**

## Ext.0 Keywords

为了支持扩展语法，新引入的关键字如下：
```
    "return"
    "break"
```

## Ext.1 Globals

### Syntax 

```
    "dim" <id_list> ":" <type> ";"
    |"constant" <id_list> ":" <type> "(" <initval> ")"";"
```

这里的 id_list 即为标识符列表，type 是变量类型。使用 "dim" 认定为变量，而 "constant" 认定为常量。
对于变量，默认为0；对于常量，其值为指定的 initval 。

**对于多种不同的变量，每种需要以单独一行给出定义。** 如：
```
    dim a,b,c : integer;
    dim d,e,f : real;
    constant ca,cb : bool(True);
```

### Overlap

默认的，procedure / program 中的同名变量将覆盖全局变量。这意味着，如果有一个全局变量 a ,在 procedure / program 中再定义一个变量 a ，无论其类型如何，此时引用到的 a 都是后定义的一个。


## Ext.2 Procedure

### Syntax 

本地函数：
```
    "procedure" <PROC_ID> "(" [argumemt list] ")" ":" <return_type> ";"
    [local variable defination]

    "begin" 
        <statements> 
    "end" "."
```

外部函数声明：
```
    "procedure" <PROC_ID> "(" [argumemt list] ")" ":" <return_type> ";"
```

这里 argument list 应符合下列语法：

```"
    <ids_1> ":" <type1> ";" <ids_2> ":" <type2> ";" ...
```

### Implementation

#### General

对于一个过程定义，解析器将通过以下格式的IR

```
    "PROCBGN", , ,
```
构建一个头部，以及下述IR 
```
    "PROCEND", , , 
```
构建一个尾部。

#### Returns

当子过程中使用了 "return" ，该子过程将返回到调用者，返回值保存在**实现相关**的寄存器/内存位置中。解析器将生成下述IR

```
    "RETURN", , , <RETVAL>
```

这里的 RETVAL是返回值，可以是变量或值。

**注意：编译器不会自动加入一条默认返回语句于子过程的结尾！**
**注意：以 program 标识的将被视为一个返回类型为 void / integer (取决于实现) 的空参数子过程**

更多细节，请参阅 [Abstract Calling Conversion](#abstract-calling-conversion).

## Ext.3 Procedure calls

### Syntax

子过程调用有两种形式，分别是忽略返回值的和作为右值的。

对于忽略返回值的形式，为
```
    "call" <proc_name> "(" [args] ")" 
```
这里的 proc_name是要调用的子过程名，args 是参数列表。**值得注意的是，该调用语句是一个单句，其后应以 ";" 结束。**

对于作为右值的调用，语法为：
```
    <proc_name> "(" [args] ")" 
```
含义同上。编译器将该形式的调用的返回值解释为一个右值，可作为一个表达式的一部分或赋值的源值。**该种语义隐含：子过程的返回值类型不为 void 。**

### Implementation

编译器将过程调用转化为一个下列格式的 IR:
```
    "CALL", <proc_name>, , [RET] 
```
此处的 proc_name 为被调用的子过程名，可选值 RET 为返回值的目标位置（如果不存在，将是一个空串），通常是一个变量。

更多细节，请参阅 [Abstract Calling Conversion](#abstract-calling-conversion).

## Abstract Calling Conversion

### General

编译器将目标抽象为一个带有调用栈的平台，且返回值可以通过寄存器/特定内存位置传递。

在每个 procedure 的起始，（"PROCBGN"之后），每个参数被从调用栈中顺序弹出，以下列格式的 IR 给出:

```
    "POPARG", , , <ARG_NAME>
```
其中 ARG_NAME 为参数名称。

对于调用者，在真正调用之前，参数以逆序入栈，以下列格式的 IR 给出:

```
    "PUSHARG", , , <VALUE>
```
其中 VALUE 为参数名称/参数值。

### Translating

对于翻译器，在第一个 "PUSHARG" 时，需记录参数列表，直到 "CALL"。生成的目标代码需符合目标平台的真实调用约定。在 "CALL" 之后，如有返回值，需要送入 "CALL" 中给出的目标地址。

值得注意的是，抽象调用约定虽然假定了后级的结构，但是并没有限制目标平台的真实调用约定。如对于常见的 x86 调用，一部分参数通过寄存器传递，这时翻译器将需要按照真实调用约定，将一部分参数丢到寄存器中，其他参数通过栈传递。栈平衡、现场保存与恢复也应遵循目标平台的调用约定。

## Ext.4 Pointer

### Pointer Type Syntax

```
    <normal_type> ".."
```
此处 normal_type 指的是非指针类型。当前仅支持一维指针。

### Pointer Assignment

#### Syntax

1. 指针到指针变量：
```
    <ID_PTR_DST> ":=" <ID_PTR_SRC>
```
此处 ID_PTR_DST 为目标指针变量标识符，ID_PTR_SRC 为源指针变量标识符。

2. 变量地址到指针变量
```
    <ID_PTR> ":=" ".." <ID>
```
此处 ID_PTR 为指针变量标识符，ID 为待取地址的变量标识符。

#### Limitations

在上述两种情况下，类型限制如下：

- 从 void* 到任意指针类型，或从任意指针类型到 void* 是允许的，但是会触发编译器警告，因为编译器无法确保其安全性。
- 其他情况，操作两边的指针类型必须相同。

### Accessing Value (Not impl. yet)

#### Syntax

存取指针变量目标地址的变量时，使用下列语法：
```
    <ID_PTR> "[" "]"
```
这里 ID_PTR 为指针变量的标识符。

#### Limitations

- void 型指针不允许访问，因为编译器无法确定其大小

## Ext.5 Preprocess Directives

该扩展单独写入 [Preprocessor Manual](./Preprocessor_202301.md) 中。

