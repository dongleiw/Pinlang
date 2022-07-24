## 说明
写个自己的语言玩. 语言是一个静态类型的解释型语言

lexer/parser: 手写lexer和parser太繁琐也不趣, 就用antlr4生成. 

interpreter: C++写

### 当前状态
    刚开始, 啥都不支持 :)

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


## 历史
* 2022-07-24 支持变量定义
