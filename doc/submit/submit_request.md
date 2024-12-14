# Submit Request for CSC2024 Compiler Design Contest

## 【优化违规行为包括但不限于】

1. 针对特定用例名或函数名等的优化。
2. 通过给定输出的方式蒙混过关。
3. 尝试获得评测机、隐藏用例等信息。
4. 其他试图获取不公平优势的行为。

## 【提交要求】

1. 总体要求：实现一个编译器，将 SysY2022 语言的代码翻译为 RISC-V 汇编代码。仅可使用 C、 C++、Java 或 Rust 。
2. 提交的编译器应支持指定的 SysY2022 语言 。关于 SysY2022 语言的定义请参考下方附件中提供的详细说明文件"[SysY2022 语言定义](https://gitlab.eduxiji.net/nscscc/compiler2023/-/blob/master/SysY2022%E8%AF%AD%E8%A8%80%E5%AE%9A%E4%B9%89-V1.pdf)"。
3. 提交的编译器需要具有 `词法分析`、`语法分析`、`语义分析`、`目标代码生成与优化`等功能。
4. RISC-V 硬件赛道，对于正确编译通过的 SysY2022 基准测试程序，应生成符合要求的 `RISC-V汇编文件`，要求能被编译链接成可执行文件，并在安装有 Linux 操作系统的指定 RISC-V 硬件平台上加载并运行，以测试程序执行时间作为评价依据。生成的汇编程序应为 64 位 ，RISC-V 体系结构。
5. 使用平台内置的 GitLab（页面上方的导航栏）进行协同开发，并直接提交仓库地址（https 地址）进行评测。用大赛平台的账号密码可以直接登录 GitLab；其他参赛队员访问[https://course.educg.net/sv2/indexexp/contest/index.jsp](https://course.educg.net/sv2/indexexp/contest/index.jsp)，注册账号，点击页面顶部的 GitLab，比赛平台会自动创建一个 GitLab 账号。
6. 自行创建 GitLab 项目，项目名称建议：Compiler2024-X，项目描述：队伍名称，学校名称。具体参考[代码托管平台使用文档与规范](https://gitlab.eduxiji.net/nscscc/compiler2021/-/blob/master/GitLab%E5%9F%BA%E7%A1%80%E4%BD%BF%E7%94%A8%E6%96%B9%E5%BC%8F.pdf)。测评程序默认拉取 master 分支，如果需要选择其他分支进行测评，请在提交面板上输入仓库地址后，加空格输入分支名称，具体参照提交页面上的帮助。测评程序将拉取指定的分支进行测评。
7. 测评程序会直接使用 clang/clang++ 、 javac 或 Cargo 对提交的项目代码进行编译，测评程序会自动扫描项目下的代码文件，测评程序支持的文件后缀包括

- C/C++语言实现，支持 C++20 标准：

  - 头文件：h、hh、hpp、H、hxx
  - 源文件：c、cpp、cp、cxx、cc、c++、C、CPP
  - 请确保项目代码中只有一个 main 函数。

- Java 语言实现，支持 Oracle Java SE 17：

  - 后缀：java
  - 主类名：Compiler.java (主类不带包名)
  - 测评程序会将编译好的 class 文件打包成 jar 格式并执行。若项目中包含 res 目录，则测评程序会将其中内容作为资源文件复制到 jar 包的根目录中。

- Rust 语言实现，支持 Rust 1.81.0-nightly：

  - 后缀：rs

### 【输入形式】

学生将编译器统一命名为 compiler 。测试将通过如下命令将 testcase.sy 中的 sysy2022 语言的代码编译成 testcase.s 中的 RISC-V 汇编代码。（注意：测评时命令中的 testcase.sy 和 testcase.s 会被替换为相应文件的绝对路径，系统会保证编译器对这些文件有相应的读写权限,存放测试用例的目录是只读的）

- 功能测试：`compiler -S -o testcase.s testcase.sy`
- 性能测试：`compiler -S -o testcase.s testcase.sy -O1`

测试程序的具体输入输出方法请参考[SysY 运行时库](https://gitlab.eduxiji.net/nscscc/compiler2023/-/blob/master/SysY2022%E8%BF%90%E8%A1%8C%E6%97%B6%E5%BA%93-V1.pdf)。

### 【输出形式】

按如上要求将目标汇编代码生成结果输出至 testcase.s 文件中。

### 【评分要求】

评测过程分为功能测试和性能测试，按照功能得分、性能得分、性能测试总运行时间进行排名。具体评分要求请参考[2024 系统能力大赛-编译系统设计赛技术方案](https://gitlab.eduxiji.net/csc1/nscscc/compiler2024/-/blob/main/2024%E5%85%A8%E5%9B%BD%E5%A4%A7%E5%AD%A6%E7%94%9F%E8%AE%A1%E7%AE%97%E6%9C%BA%E7%B3%BB%E7%BB%9F%E8%83%BD%E5%8A%9B%E5%A4%A7%E8%B5%9B-%E7%BC%96%E8%AF%91%E7%B3%BB%E7%BB%9F%E8%AE%BE%E8%AE%A1%E7%BC%96%E8%AF%91%E7%B3%BB%E7%BB%9F%E5%AE%9E%E7%8E%B0%E8%B5%9B%E9%81%93%E6%8A%80%E6%9C%AF%E6%96%B9%E6%A1%88V1.1.pdf)

### 【测评结果说明】

测评结果按照 CE、RE、TLE、WA、AC 这一优先级进行显示。

- 总评：

  - CE：编译从 gitlab 上拉取的项目时发生了错误
  - RE：整个测评流程过程中发生了运行时错误。如拉取项目代码失败，某个用例发生了 RE 或 TLE，未进行性能测试等
  - TLE：整个测评耗时超过了总限制时长，当前为 30 分
  - WA：存在测试用例 WA
  - AC：测试通过

- 测试点：
  - CE：使用提交的编译器编译用例程序时发生了错误
  - RE：该测试点测评流程过程中发生了运行时错误。如测评程序找不到输出的.s 文件、gcc 汇编链接失败、测评程序没有找到计时函数的输出等
  - TLE：该测试点在生成汇编代码或执行汇编时超过了时间限制。
  - WA：最终在树莓派上的可执行文件的标准输出和期望输出 或 程序实际返回值和期望返回值不一样（注意：包括可执行程序执行失败）
  - AC：测试点测试通过，标准输出和期望输出一致、程序实际返回值和期望返回值一致
