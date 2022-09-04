## 说明
写个自己的语言玩. 语言是一个静态类型的解释型语言. lexer/parser用antlr4生成. 



### 当前状态
持续开发中 

### 目录结构
    antlr4: antlr4的代码, 库, 以及文法
    example_code: 实例代码
    src: 解释器代码
    src_parser: lexer和parser(antlr4生成)

### 编译
1. 进入antlr4, 执行`sh generate.sh`, 生成lexer和parser
2. 创建build目录`mkdir -p build && cd build`
3. 编译`cmake build .. && make`
4. 编译完成后, 会在build目录下生成解释器`./pinlang`


## 语法
```
// 定义变量
const PersonId = int; 
// 定义变量 支持+-*/%
var i = (3+5-4)*3/2; 
// 定义函数
fn sum(a int, b int) int{
    return a+b;
}
fn sum(a int) int{
    return a;
}
// 调用函数
var r1=sum(1,2); // call sum(int,int)int
var r2=sum(3);   // call sum(int)int
```

## 想法
1. ASCII的符号不够用, 也许可以考虑选择一些unicode符号? 不过输入有点复杂

## 历史
* 2022-07-24 支持变量定义
* 2022-07-30 支持了内置方法. 将 (A + B)抽象为A的add方法的调用. 可以让解释器实现简单一些. 也方便后续实现操作符重载
* 2022-07-31 支持了普通函数定义. return. 还不支持执行. 😃
* 2022-08-20 支持了函数执行
* 2022-08-21 支持了函数重载 😄
* 2022-08-28 支持str和float类型. 增加一个内置的printf函数输出
