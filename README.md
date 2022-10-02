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

// 预定义的约束(没错,类似于Rust的trait)
// int/float/str实现了该约束
restriction Add[AnotherT,ReturnT]{
    fn add(another AnotherT) ReturnT;
}

// 定义泛型函数. 没有使用<>, 而是用的[]. 避免语法歧义
// 泛型参数T需要满足约束: Add[T,T]
fn gsum[T Add[T,T]](a T, b T) T{
    return a+b;
}
// 调用泛型函数
var gri = gsum(1, 2);
var grf = gsum(1.0, 2.9);
var grs = gsum("hello", "world");

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
* 2022-09-04 增加了约束和泛型函数, 泛型实现有点复杂, 目前的还非常非常简陋 😅
* 2022-09-17 经过了两周的努力总算有了简单的泛型, 但还有很多问题待解决. 我发现语言设计者要抑制住'加强语法'的冲动, 要在"功能、直观易于理解、可扩展、实现简单"这些多个方面考虑, 甚至有时候不得不做一些妥协. 比如C++的模板相关语法很强大, 但是也太复杂了. 而且还不断的增加新的语法. 这未必就是一个好的选择. 我自己的想法是: "直观易于理解最重要, 其次是功能, 在其次是可扩展性"
* 2022-09-25 发明了一个<复杂函数>语法, 顾名思义就是比较复杂的函数. 可以给一个函数名绑定多个实现. 每个实现可以是普通函数, 也可以是泛型. 比如以下例子
    ```
    fn foo{
        (a int)int{ return a; }
        (a float)str{ return "float spec"; }
        [T Add[T,T]](a T, b T)T{ return a+b; }
        [T Add[T,T]](a T, b T, c T)T{ return a+b+c; }
    }
    ```
    可以算是函数重载和泛型的结合版本. 我还是比较纠结, 让一个函数名绑定这么多实现是否有些过度了.
* 2022-09-25 支持了数组类型, 包括初始化和下标访问. 加了一些编译期常量表达式优化的内容. 泛型函数内部可以使用泛参了
    ```
    fn foo[T ToString](a T){
        printf("T={}\n", T);
        if(T == int){
                printf("T==int: a={}\n", a);
        }else{
                printf("T!=int: a={}\n", a);
        }
    }
    foo(true);
    foo(1);
    ```
    计划让泛型函数内使用泛参来做泛型特化. 不过还有些问题没有想清楚
* 2022-10-01 增加最简陋的class类型和赋值
* 2022-10-02 增加初始化表达式, 替换掉旧的"数组初始化表达式". 假期余额不足了😕
    ```
    var p = Person{
        id: 33,
        name: "dl",
        skills: []str{"sleep", "eat"},
        records: [][]int{ 
            []int{1,2,3},
            {4,5,6}
        }
    };
    ```
