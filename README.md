# FreeTiecodec

FreeTiecodec（FTC）是结绳（Tiecode）语言的编译器，使用 C++17 实现。项目名为 FreeTiecodec（FTC），构建产物/可执行文件名为 `tiec`。

## 概览

- 输入：中文关键字的 `.t` 源文件与标准库 `.t` 文件。
- Android 目标：Java 源码、`AndroidManifest.xml`、资源、`metadata.json`，可选完整 Gradle 工程。
- JVM 目标：纯 Java 源码、`Main.java`、`metadata.json`。
- 默认修正参考实现的若干产物问题（泛型命名、运算符方法名、内联 code 分号等）；`--disable-ftc-features` 用于兼容性对照。
- Android 入口为 `启动窗口`；JVM 入口为 `类 启动类` + `方法 启动方法()`。

完整的语言规范、类型映射、AST/Pass、代码生成和平台产出见 [docs/index.rst](docs/index.rst)。

## 构建编译器

需要 CMake ≥ 3.16 与支持 C++17 的编译器。

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

运行测试（需要 GoogleTest ≥ 1.10；CMake 找不到时会尝试 FetchContent）：

```bash
ctest --test-dir build --output-on-failure
```

代码风格检查：

```bash
bash scripts/check_style.sh
```

## 快速使用

Android：

```bash
./build/tiec --platform android --release --friendly-name original \
    -o out \
    $(find stdlibs/安卓基本库/源代码 -name '*.t') \
    app.t
```

JVM：

```bash
./build/tiec --platform jvm --release --friendly-name original \
    -o out \
    stdlibs/安卓基本库/源代码/Java/JVM_基本类型.t \
    $(find stdlibs/JVM基本库/源代码 -name '*.t') \
    app.t
```

命令行完整选项见 [docs/appendix/cli.rst](docs/appendix/cli.rst)，语言与目标产出见 [docs/index.rst](docs/index.rst)。

## 构建并查看文档

文档使用 reST + Sphinx，依赖清单在 `docs/requirements.txt`。首次使用时创建 `.venv`：

```bash
python3 -m venv .venv
.venv/bin/python -m pip install -i https://pypi.org/simple -r docs/requirements.txt
```

之后构建 HTML 文档：

```bash
.venv/bin/python -m pip check
.venv/bin/sphinx-build -W --keep-going -b html docs docs/_build/html
```

打开 `docs/_build/html/index.html` 即可查看；也可以使用 `make -C docs html`。

## 文档导航

- [语言规范](docs/language/lexical.rst)：词法、EBNF、类型与转换、脱糖、语义。
- [编译器实现](docs/compiler/pipeline.rst)：Pass 管线、AST/IR、代码生成、注解。
- [平台产出](docs/platform/android.rst)：Android 清单/资源/Gradle 与 JVM 启动器。
- [限制与兼容开关](docs/appendix/divergences.rst)：剩余限制、设计范围、FTC 默认修正与 `--disable-ftc-features`。
- [构建与验证](docs/appendix/verification.rst)：文档依赖、测试与端到端验证。

## 许可证

本项目使用 GPL-2.0 许可证，见仓库根目录 `LICENSE`。
