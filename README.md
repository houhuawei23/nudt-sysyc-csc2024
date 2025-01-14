# NUDT SysY Compiler for CSC-2024

<p align="center"> 
<img class="cropimg" src="NUDT-SysY-Compiler.png" alt="NUDT-SysY-Compiler" style="width: 60%;" />
</p>

<!-- "https://raw.githubusercontent.com/houhuawei23/nudt-sysyc-csc2024/main/NUDT-SysY-Compiler.png" -->

Reconstructioned from [SysYCompiler](https://gitee.com/triple-adventurer/sys-ycompiler).

## 基本情况

Contributors 开发者:

- [侯华玮](https://github.com/houhuawei23), [汤翔晟](https://github.com/TernaryExplorer), [杨俯众](https://gitee.com/westme10n), [简泽鑫](https://github.com/xinchen-jzx)

<a href="https://github.com/houhuawei23/nudt-sysy-compiler-csc2024-origin/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=houhuawei23/nudt-sysy-compiler-csc2024-origin" />
</a>

Made with [contributors-img](https://github.com/lacolaco/contributors-img)

---

- 主要开发周期：2024.02 - 2024.08
- 总代码行数 5W+：
  - 手写代码 3W+，根据自定义模板规则自动生成代码 2W+；
- 使用 [ANTLR4](https://github.com/antlr/antlr4) 生成 C++ 前端（Lexer、Parser），生成 AST
- 参照模仿 [LLVM MLIR](https://mlir.llvm.org/)、[CMMC](https://github.com/dtcxzyw/cmmc) 设计实现了两级中间表示（IR 和 MIR），通过全部功能测试样例：
  - 词法分析、语法分析、语义分析、目标代码生成等；
- 实现若干重要的编译优化技术：
  - 死代码删除、循环剥离、循环并行化等；
- 2024 年全国大学生计算机系统能力大赛-编译系统设计赛-全国总决赛二等奖

<p align="center"> <img src="./doc/pics/csc2024-riscv.png" width = 60%/>

- 优化效果概览：

决赛性能得分（vs Best Performance）：

<p align="center"> <img src="./doc/pics/final_score.png" width = 90%/>

比赛结束后修复“依赖关系分析”等 bug 后，加上循环并行化优化后性能（vs GCC-O3）：

<p align="center"> <img src="./doc/pics/performance.png" width = 90%/>

在”通用矩阵乘“三重循环上的”循环并行化“优化后效果（vs GCC-O3）：

<p align="center"> <img src="./doc/pics/parallel.png" width = 90%/>

- 编译器设计框架：

<p align="center"> <img src="./doc/pics/workflow.png" width = 90%/>

[Commits Histogram](https://github.com/houhuawei23/nudt-sysy-compiler-csc2024-origin/graphs/commit-activity):

<p align="center"> <img src="./doc/pics/commits-histogram.png" width = 90%/>

[Contributors](https://github.com/houhuawei23/nudt-sysy-compiler-csc2024-origin/graphs/contributors):

<p align="center"> <img src="./doc/pics/contributors.png" width = 90%/>

## 项目结构

- `src`：源码目录
- `include`：头文件目录
- `third_party`：第三方库目录（需要手动下载）
  - `antlr4`：
    - `antlr4-cpp-runtime-4.12.0-source`：ANTLR4 C++运行时库源代码
    - `antlr-4.12.0-complete.jar`：ANTLR4 Java jar 包
- `doc`：文档目录
- `test`：测试目录，包含测试用例（需要手动下载）

## 运行

- 推荐使用 Debian-12 bookworm 或 Ubuntu 22.04
- 可使用 wsl2，推荐使用裸机 linux 或双系统

### 安装依赖

#### 下载并安装 ANTLR4

- [ANTLR4 Releases](https://github.com/antlr/antlr4/releases)
- [ANTLR4 Downloads](https://github.com/antlr/website-antlr4/tree/gh-pages/download)

这里使用了 ANTLR4 4.12.0 版本，与比赛测试环境中版本一致。

- 下载 antlr4 jar package [antlr-4.12.0-complete.jar](https://github.com/antlr/website-antlr4/blob/gh-pages/download/antlr-4.12.0-complete.jar)
- 下载 ANTLR4 C++运行时库源代码 [antlr4-cpp-runtime-4.12.0-source.zip](https://github.com/antlr/website-antlr4/blob/gh-pages/download/antlr4-cpp-runtime-4.12.0-source.zip)
- 加压到 `third_party/antlr4/` 目录下

在 `third_party/antlr4/antlr4-cpp-runtime-4.12.0-source/` 目录下，执行：

```bash
# pwd: path_to/third-party/antlr4/antlr4-cpp-runtime-4.12.0-source/
mkdir build
cmake -S . -B build -DANTLR_JAR_LOCATION=$(realpath ../antlr-4.12.0-complete.jar) -DWITH_DEMO=True
cmake --build build -j$(nproc)
cmake --install build # install to /usr/local/
```

#### 安装其他依赖

##### must

```bash
sudo apt-get update
sudo apt-get install -y build-essential uuid-dev libutfcpp-dev pkg-config make git cmake openjdk-11-jre
```

##### optional

```bash
# llvm clang
llvm-15-dev

# ninja
ninja-build

# google test
libgtest-dev

# cross-compiler
sudo apt install gcc-riscv64-linux-gnu # or gcc-riscv64-unknown-elf,gcc-11-riscv64-linux-gnu, gcc-arm-linux-gnueabihf

# g++
g++-12-riscv64-linux-gnu
g++-12-arm-linux-gnueabihf

## tldr: Simplified and community-driven man pages
sudo apt-get install -y tldr && tldr -u

```

#### Build the project

```bash
# cd to project root directory

# default use Unix Makefiles
mkdir build
cmake -S . -B build
cmake --build build -j$(nproc)

# use Ninja
cmake -s . -B build -G Ninja
cmake --build build -j$(nproc)

# generate ./compiler executable under project root directory
# ./compiler has two different command line args parse method:
# one for local test, one for official server test
# 1 local test:
Usage: ./compiler [options]
  -f {filename}                     input file
  -i                                Generate IR
  -t {passname} {pasename} ...      opt passes names to run
  -o {filename}                     output file, default gen.ll (-ir) or gen.s (-S)
  -S                                gen assembly
  -O[0-3]                           opt level
  -L[0-2]                           log level: 0=SILENT, 1=INFO, 2=DEBUG

./compiler -f test.sy -i -t mem2reg dce -o test.ll
./compiler -f test.sy -S -t mem2reg -o test.s

# 2 official server test:
./compiler -S -o test.s test.sy
./compiler -S -o test.s test.sy -O1


```

#### Run the test

```bash
# bash test script
./test.sh -t test/2021/functional/ -p mem2reg -p dce -p scp -p sccp -p simplifycfg -L1
./test_asm.sh -t test/2021/functional/ -p mem2reg -p dce -p scp -p sccp -p simplifycfg -L1

# python test script (multi-threading)
python ./submit/runtest.py compiler_path tests_path output_asm_path output_exe_path output_c_path
```

## 其他

### 重要时间节点梳理

- [timeline.mw](./timeline.mw) or [online](https://meridiem.markwhen.com/houhuawei23/timeline)

<br>

- 2023.12.08 第一次编译比赛宣讲
- 2023.12.21 “编译比赛小群” 4 人群建群
- 2024.01.24 2nd 培训
  - 下午: 梳理了编译的基本流程和编译器的基本结构; 前端算法介绍
  - 晚上: 开发环境配置 和 antlr4 使用简介
  - 布置寒假任务:
    - 熟悉 Sys2022 语言规范
    - 利用 antlr4 实现编译器前端，实现 c -> ast
- 2024.03.08 3rd 培训讲解:
  - 静态单赋值 SSA (Part1);
  - IR 数据结构的一般设计 (Value-Use-User)
- 2024.03.22 往届参赛队经验分享 arm x 2
- 2024.03.29 培训
  - 静态单赋值 SSA (Part2);
  - 往届参赛队经验分享 riscv
- 2024.04.12 培训:
  - 代码生成
- 2024.04.20 比赛正式公示开赛
  - 语法/目标平台跟 23 年保持一致
- 2024.04.24 下发开发板
- 2024.05.17 培训: 协同开发环境搭建/自动化测试
- 2024.05.24 比赛第一次官方培训 湖大
  - 往后每周末基本都有官方培训 内容包括
    - 赛题介绍 编译器设计与测试 中间表示设计 优化
    - 上届优秀参赛队经验分享 cmmc 郑英炜
- 2024.05.30 截止报名 +7days
  - 在线测评系统开放 可以提交代码测评 实时排名
  - 一次测评需要 1 个多小时，建议本地先做好测试 (docker 编译环境/开发板测试)
- 2024.07.20 暑假开始
  - 以参加比赛作为暑期实习
- 2024.08.10 初赛截止
  - rv 6th; arm 5th, 6th
  - rv 41, arm 16
- 2024.08.18 - 08.22 现场决赛 杭电
- 2024.09.03 开学 暑假结束
- 2024.09.04 暑期实习报告; 赛后交流总结
- 2024.09.10 拿到获奖证书 (电子版)

### 经验 教训 收获 

- 工程量大，实现赛越来越卷，需要尽早准备和开展工作
- 速度 vs 质量
  - 速度: 
    - 要尽快构建起“最小可运行程序”，能够测试最简单的样例，也就是要先跑起来再说，先打通流程再逐步细化
  - 质量:
    - 程序框架: 程序架构/编程范式/设计模式
    - 程序细节: 代码质量（严谨性/通用性/可移植性）
    - 否则随着代码量增加，整个程序将变成石山，非常痛苦
  - 两手抓、两手都要硬: 
    - 在追求速度的同时要注重程序质量，在保证程序质量的同时也要注意尽快跑起来
    - 一般来说，要先实现一个能测试最简单样例的程序，然后在逐步细化时要及时代码重构
    - 速度决定你能很快上路，质量决定你能走多远
  - 如何提高代码质量
    - 时刻葆有对高质量代码的追求
    - 编程语言选择: “更安全”的语言?
    - 阅读优秀代码, 学习设计思路和实现方式
    - 阅读 Effective XXX
    - 在 coding 中体会
- 及时整理归档资料，Q&A
  - “吃一堑，长一智”
- 寒假
  - 没有必要学习编译原理
  - 了解编译的基本流程和编译器的基本结构后可以开始着手看代码，写代码
  - 推荐一些高校的编译课程实验文档/指导书
    - [BUAA miniSysY-tutorial](https://github.com/No-SF-Work/miniSysY-tutorial)
    - [nku-rust-doc](https://junimay.github.io/nku-compiler-2024-rs/index.html)
    - [北大编译实践在线文档](https://pku-minic.github.io/online-doc/#/)
    - [THU minidecaf](https://decaf-lang.github.io/minidecaf-tutorial/)