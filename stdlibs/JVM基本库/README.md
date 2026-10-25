# JVM 基本库（附加包）

JVM 桌面目标不复制整套基本库，直接复用：

- `../安卓基本库/源代码/Java/JVM_基本类型.t`：对象、文本、数值、遍历器、键值表条目、异步调度器等 JVM 无关声明
- 本目录 `源代码/`：
  - `JVM_控制台.t`：`控制台.输出(...)`，输出到标准输出
  - `JVM_调试.t`：`调试输出(...)`
  - `JVM_兼容类型.t`：`JVM_基本类型.t` 引用到、但最小目标暂不生成实现的最小桩类型

入口约定：

```
类 启动类
    方法 启动方法()
        控制台.输出("你好，结绳！")
    结束 方法
结束 类
```

编译命令：

```bash
./build/tiecodec --platform jvm --release --friendly-name original -o out \
    stdlibs/安卓基本库/源代码/Java/JVM_基本类型.t \
    $(find stdlibs/JVM基本库/源代码 -name '*.t') \
    app.t
```

产物：`out/src/*.java`、`out/src/Main.java`、`out/metadata.json`。有 JDK 时可直接：

```bash
javac -encoding UTF-8 -d classes out/src/*.java
java -cp classes Main
```
