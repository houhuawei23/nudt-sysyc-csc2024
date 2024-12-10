# CSC-Former Projects

## csc-former

### arm23

- 0-北航-miaomiao：
  - [那一年喵喵变成了光-gitlab](https://gitlab.eduxiji.net/educg-group-18973-1895971/compiler2023-202310006201934)
  - 手写前端，mir，lir，midend
- 1-北航-atri：
  - [ATRI](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310006201725-78)
  - antlr4前端，arm，riscv
- n-vrabche： antlr4

### arm22

- 3-NUDT-嘉然：

  - [嘉然今天偷着乐](https://gitlab.eduxiji.net/educg-group-12619-928705/RaVincent-2379)
  - antlr+mlir
  - 模块化清晰
  - 值得学习
- 3-西北工业大学-从容应队：

  - [从容应队](https://gitlab.eduxiji.net/educg-group-12619-928705/HammerWang-2412)
  - 优化较多较详细
  - 文档齐全清晰，值得学习

  1. 基于Flex、Bison构建的词法、语法分析器
  2. 建立抽象语法树与符号表
  3. 语义分析、语义检查
  4. MIR和LIR两级中间表示
  5. 支持将MIR转为C源程序，可编译运行
  6. 针对AST、MIR、LIR、ARM的优化
  7. 翻译为ARMv7汇编语言

### 21

- 0-清华-小林家的
  - [小林家的编译器](https://github.com/kobayashi-compiler/kobayashi-compiler)
  - armv7 + risc-v32
  - 自动并行化
  - 可编译通过
  - 先跑起来再说
  - antlr4
  - 比赛经验分享slides：非常好

### arm20

- 0-清华 trivial：

  - [编程是一件很危险的事](https://github.com/TrivialCompiler/TrivialCompiler)
  - 结构清晰
  - 编译通过
  - 手写前端
- 0-中国科学技术大学-ustc：

  - [燃烧我的编译器](https://github.com/mlzeng/CSC2020-USTC-FlammingMyCompiler)
  - 决赛最好成绩
  - 三层ir：高层-中层-底层
  - 文档清晰
  - 性能大多数超过GCC-O3
  - 目标平台树梅派armv8
  - 无法完成构建
- 1-北京科技大：

  - [DR直呼内行](https://github.com/MaxXSoft/MimiC)
  - 编译有问题
  - 架构清晰，arm
  - Hand written frontend.
  - Strong typed IR in SSA form.
  - Optimizer based on pass and pass manager.
  - Auto-scheduling, multi-stage, iterative pass execution.
  - Abstracted unified backend interface.
  - Machine-level IR (MIR) for multi-architecture machine - instruction abstraction.
  - MIR based passes for multi-architecture assembly generation.

### riscv23

- 3-guas 合肥工业大学：antlr4，starfive开发板，可跑通
  - [起床睡觉队](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310359201848-2384)
- 4-carrotcompiler大萝卜编译器：
  - [大萝卜编译生产队](https://gitlab.eduxiji.net/educg-group-18973-1895971/carrotcompiler)
  - 手写前端

### others

- miniSysY_example_compiler：
  - [miniSysY_example_compiler](https://github.com/BUAA-SE-Compiling/miniSysY_example_compiler)
  - 北航实验示例
  - antlr4, java
  - 较简单，值得学习

## Reps

### 2020

| 学校                   | 队伍名称             | 源码仓库地址                                                                                                                      |
| ---------------------- | -------------------- | --------------------------------------------------------------------------------------------------------------------------------- |
| 清华大学               | 编程是一件很危险的事 | [https://github.com/TrivialCompiler/TrivialCompiler](https://github.com/TrivialCompiler/TrivialCompiler)                             |
| 北京科技大学           | DR直呼内行           | [https://github.com/MaxXSoft/MimiC](https://github.com/MaxXSoft/MimiC)                                                               |
| 哈尔滨工业大学（深圳） | 形式语言与复读机     | [https://github.com/nzh63/syc](https://github.com/nzh63/syc)                                                                         |
| 北京航空航天大学       | 段地址不队           | [https://github.com/segviol/indigo](https://github.com/segviol/indigo)                                                               |
| 北京交通大学           | 斯缤楷忒（SpinCat）  | [https://gitlab.eduxiji.net/CSC2020-BJTU-SpinCat/compiler](https://gitlab.eduxiji.net/CSC2020-BJTU-SpinCat/compiler)                 |
| 东北大学               | Q29tcGlsZXI=         | [https://github.com/i-Pear/CSC2020-NEU-Q29tcGlsZXI-](https://github.com/i-Pear/CSC2020-NEU-Q29tcGlsZXI-)                             |
| 电子科技大学（清水河） | 创造1010             | [https://github.com/togetherwhenyouwant/Creator_1010_Compiler.git](https://github.com/togetherwhenyouwant/Creator_1010_Compiler.git) |
| 中国科学技术大学       | 燃烧我的编译器       | [https://github.com/mlzeng/CSC2020-USTC-FlammingMyCompiler](https://github.com/mlzeng/CSC2020-USTC-FlammingMyCompiler)               |
| 成都理工大学           | oops sysy            | [https://gitlab.eduxiji.net/dvs/compiler](https://gitlab.eduxiji.net/dvs/compiler)                                                   |

### 2021

| 学校                   | 队伍名称                          | 源码仓库地址                                                                                                                                     |
| ---------------------- | --------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------ |
| 清华大学               | 小林家的编译器                    | [https://github.com/kobayashi-compiler/kobayashi-compiler](https://github.com/kobayashi-compiler/kobayashi-compiler)                                |
| 北京航空航天大学       | No Segmentation Fault Work        | [https://github.com/No-SF-Work/ayame](https://github.com/No-SF-Work/ayame)                                                                          |
| 湖南大学               | 沙梨酱耶                          | [https://github.com/abrasumente233/SallyCompiler](https://github.com/abrasumente233/SallyCompiler)                                                  |
| 华南理工大学           | TINBAC Is Not Building A Compiler | [https://gitlab.eduxiji.net/tinbac/tinbaccc.git](https://gitlab.eduxiji.net/tinbac/tinbaccc.git)                                                    |
| 西北工业大学           | 胡编乱造不队                      | [https://github.com/luooofan/hblzbd_compiler](https://github.com/luooofan/hblzbd_compiler)                                                          |
| 北京航空航天大学       | 真实匿名队                        | [https://gitlab.eduxiji.net/csc2021-buaa-reallyanonymous/compiler.git master](https://gitlab.eduxiji.net/csc2021-buaa-reallyanonymous/compiler.git) |
| 北京大学               | 全场景分布式优化队                | [https://gitlab.eduxiji.net/PanJason/harmony_comp.git enable_gzz](https://gitlab.eduxiji.net/PanJason/harmony_comp.git)                             |
| 中国科学技术大学       | Maho_shojo                        | [https://github.com/wildoranges/MahoShojo](https://github.com/wildoranges/MahoShojo)                                                                |
| 南开大学               | 天津泰达                          | [https://github.com/Young-Cody/CSC2021-NKU-TEDA](https://github.com/Young-Cody/CSC2021-NKU-TEDA)                                                    |
| 华中科技大学           | 六角亭华工队                      | [https://gitlab.eduxiji.net/3usi9/plain_syc](https://gitlab.eduxiji.net/3usi9/plain_syc)                                                            |
| 中国科学院大学         | ucasCC                            | [https://gitlab.eduxiji.net/csc2021-ucas-ucascc/compiler.git 8_14_1](https://gitlab.eduxiji.net/csc2021-ucas-ucascc/compiler.git)                   |
| 哈尔滨工业大学（深圳） | xwh说的都对                       | [https://gitlab.eduxiji.net/whsu/compile-a zhj](https://gitlab.eduxiji.net/whsu/compile-a)                                                          |
| 北京科技大学           | 你这编译器保熟吗                  | [https://github.com/ustb-owl/Lava](https://github.com/ustb-owl/Lava)                                                                                |
| 西北工业大学           | Calcifer                          | [https://github.com/n13eho/CalciferCompiler](https://github.com/n13eho/CalciferCompiler)                                                            |
| 中国科学技术大学       | 擅长捉弄的编译器                  | [https://github.com/misakihanayo/bazinga_compiler](https://github.com/misakihanayo/bazinga_compiler)                                                |
| 杭州电子科技大学       | 芜湖起飞                          | [https://gitlab.eduxiji.net/sys_compiler/syscompiler.git yztz](https://gitlab.eduxiji.net/sys_compiler/syscompiler.git)                             |
| 北京大学               | 编译原理不汇编                    | [https://gitlab.eduxiji.net/pku1800013122/finals](https://gitlab.eduxiji.net/pku1800013122/finals)                                                  |
| 北京科技大学           | 快马加鞭                          | [https://gitlab.eduxiji.net/USTB_NO1/compiler.git --branch=tmp1](https://gitlab.eduxiji.net/USTB_NO1/compiler.git)                                  |
| 南开大学               | super.calculate                   | [https://gitlab.eduxiji.net/csc2020-nankai-super.calculate/compiler](https://gitlab.eduxiji.net/csc2020-nankai-super.calculate/compiler)            |
| 北京航空航天大学       | 早安！白给人                      | [https://github.com/Forever518/Whitee](https://github.com/Forever518/Whitee)                                                                        |
| 华中科技大学           | DragonAC                          | [https://github.com/showstarpro/sysy](https://github.com/showstarpro/sysy)                                                                          |
| 华中科技大学           | Spica                             | [https://gitlab.eduxiji.net/hust-spica/kisyshot](https://gitlab.eduxiji.net/hust-spica/kisyshot)                                                    |

### 2022

| 学校                   | 队伍名称                            | 源码仓库地址                                                                                                                                                |
| ---------------------- | ----------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 清华大学               | 赫露艾斯塔                          | [https://gitlab.eduxiji.net/educg-group-12619-928705/helesta](https://gitlab.eduxiji.net/educg-group-12619-928705/helesta)                                     |
| 北京航空航天大学       | 喵喵队仰卧起坐                      | [https://gitlab.eduxiji.net/educg-group-12619-928705/compiler2022-meowcompiler](https://gitlab.eduxiji.net/educg-group-12619-928705/compiler2022-meowcompiler) |
| 清华大学               | 啊对对队                            | [https://gitlab.eduxiji.net/educg-group-12619-928705/compiler2022-meowcompiler](https://gitlab.eduxiji.net/educg-group-12619-928705/compiler2022-meowcompiler) |
| 北京理工大学           | HexonZ                              | [https://gitlab.eduxiji.net/educg-group-12619-928705/cbias](https://gitlab.eduxiji.net/educg-group-12619-928705/cbias)                                         |
| 复旦大学               | 编译                                | [https://gitlab.eduxiji.net/educg-group-12619-928705/penguincompiler](https://gitlab.eduxiji.net/educg-group-12619-928705/penguincompiler)                     |
| 国防科技大学           | 嘉然今天偷着乐                      | [https://gitlab.eduxiji.net/educg-group-12619-928705/RaVincent-2379](https://gitlab.eduxiji.net/educg-group-12619-928705/RaVincent-2379)                       |
| 哈尔滨工业大学（深圳） | 萝杨空队                            | [https://gitlab.eduxiji.net/educg-group-12619-928705/ssyc](https://gitlab.eduxiji.net/educg-group-12619-928705/ssyc)                                           |
| 南开大学               | NKUER4                              | [https://gitlab.eduxiji.net/educg-group-12619-928705/1911463-188](https://gitlab.eduxiji.net/educg-group-12619-928705/1911463-188)                             |
| 西北工业大学           | 从容应队                            | [https://gitlab.eduxiji.net/educg-group-12619-928705/HammerWang-2412](https://gitlab.eduxiji.net/educg-group-12619-928705/HammerWang-2412)                     |
| 北京航空航天大学       | （编译（编译（编译器）器）器）队    | [https://gitlab.eduxiji.net/educg-group-14157-894146/compiler](https://gitlab.eduxiji.net/educg-group-14157-894146/compiler)                                   |
| 北京交通大学           | 碰瓷的大白菜                        | [https://gitlab.eduxiji.net/educg-group-12619-928705/end_3/edit](https://gitlab.eduxiji.net/educg-group-12619-928705/end_3/edit)                               |
| 北京邮电大学           | 无色透明队                          | [https://gitlab.eduxiji.net/educg-group-12619-928705/mercuri-v2](https://gitlab.eduxiji.net/educg-group-12619-928705/mercuri-v2)                               |
| 电子科技大学           | 变异开始                            | [https://gitlab.eduxiji.net/educg-group-14158-894147/compiler](https://gitlab.eduxiji.net/educg-group-14158-894147/compiler)                                   |
| 东北大学               | 恩毅优肯派勒                        | [https://gitlab.eduxiji.net/educg-group-12619-928705/SysYCompiler](https://gitlab.eduxiji.net/educg-group-12619-928705/SysYCompiler)                           |
| 哈尔滨工业大学（深圳） | lrc                                 | [https://gitlab.eduxiji.net/educg-group-12619-928705/csc-3927](https://gitlab.eduxiji.net/educg-group-12619-928705/csc-3927)                                   |
| 华南理工大学           | bddd                                | [https://gitlab.eduxiji.net/educg-group-12619-928705/bddd](https://gitlab.eduxiji.net/educg-group-12619-928705/bddd)                                           |
| 华中科技大学           | WASD                                | [https://gitlab.eduxiji.net/educg-group-12619-928705/cbysal-1605](https://gitlab.eduxiji.net/educg-group-12619-928705/cbysal-1605)                             |
| 西北工业大学           | 王力口乐队                          | [https://gitlab.eduxiji.net/educg-group-12619-928705/2019302804-2071](https://gitlab.eduxiji.net/educg-group-12619-928705/2019302804-2071)                     |
| 中国科学技术大学       | 魔法御姐                            | [https://gitlab.eduxiji.net/educg-group-12619-928705/magic_misaka](https://gitlab.eduxiji.net/educg-group-12619-928705/magic_misaka)                           |
| 中国科学技术大学       | 和我签订契约，成为可执行文件！      | [https://gitlab.eduxiji.net/educg-group-12619-928705/compiler2022-be_an_elf](https://gitlab.eduxiji.net/educg-group-12619-928705/compiler2022-be_an_elf)       |
| 中南大学               | 麓南双人组                          | [https://gitlab.eduxiji.net/educg-group-12619-928705/Devotes-419](https://gitlab.eduxiji.net/educg-group-12619-928705/Devotes-419)                             |
| 北京交通大学           | MoeCompiler                         | [https://gitlab.eduxiji.net/educg-group-12619-928705/compiler2022-moecompiler](https://gitlab.eduxiji.net/educg-group-12619-928705/compiler2022-moecompiler)   |
| 北京理工大学           | 弓张编译器同好会                    | [https://gitlab.eduxiji.net/educg-group-12619-928705/yumiharakonnpaira](https://gitlab.eduxiji.net/educg-group-12619-928705/yumiharakonnpaira)                 |
| 电子科技大学           | 没编译就没BUG                       | [https://gitlab.eduxiji.net/educg-group-12619-928705/ohhhh-3145](https://gitlab.eduxiji.net/educg-group-12619-928705/ohhhh-3145)                               |
| 湖南大学               | 旋风巴别鱼                          | [https://gitlab.eduxiji.net/educg-group-12619-928705/april-2384](https://gitlab.eduxiji.net/educg-group-12619-928705/april-2384)                               |
| 华中科技大学           | 编的不坏，译的也快                  | [https://gitlab.eduxiji.net/educg-group-12619-928705/t865486907-3026](https://gitlab.eduxiji.net/educg-group-12619-928705/t865486907-3026)                     |
| 浙江大学               | thcc                                | [https://gitlab.eduxiji.net/educg-group-12619-928705/TimeOrange-3924](https://gitlab.eduxiji.net/educg-group-12619-928705/TimeOrange-3924)                     |
| 中山大学               | Yat-CC: Yet another tiny C compiler | [https://gitlab.eduxiji.net/educg-group-12619-928705/sysu001-1557](https://gitlab.eduxiji.net/educg-group-12619-928705/sysu001-1557)                           |
| 重庆大学               | LNK2022                             | [https://gitlab.eduxiji.net/educg-group-12619-928705/compiler](https://gitlab.eduxiji.net/educg-group-12619-928705/compiler)                                   |
| 北京航空航天大学       | LoveLive!Virtual Magic!             | [https://gitlab.eduxiji.net/educg-group-12619-928705/CoolColoury-2778](https://gitlab.eduxiji.net/educg-group-12619-928705/CoolColoury-2778)                   |
| 北京航空航天大学       | 编不出来不起床                      | [https://gitlab.eduxiji.net/educg-group-12619-928705/MegaSysy](https://gitlab.eduxiji.net/educg-group-12619-928705/MegaSysy)                                   |
| 北京航空航天大学       | 又是困难的取名时间了                | [https://gitlab.eduxiji.net/educg-group-12619-928705/wjh15101051-3196](https://gitlab.eduxiji.net/educg-group-12619-928705/wjh15101051-3196)                   |
| 北京航空航天大学       | coredump: )                         | [https://gitlab.eduxiji.net/educg-group-12619-928705/Combinatorics-626](https://gitlab.eduxiji.net/educg-group-12619-928705/Combinatorics-626)                 |
| 华中科技大学           | 自主可控编译器                      | [https://gitlab.eduxiji.net/educg-group-12619-928705/U201915084-1896](https://gitlab.eduxiji.net/educg-group-12619-928705/U201915084-1896)                     |
| 北京航空航天大学       | lden Compiler                       | [https://gitlab.eduxiji.net/educg-group-12619-928705/elden-compiler](https://gitlab.eduxiji.net/educg-group-12619-928705/elden-compiler)                       |
| 北京理工大学           | 正能量满满队                        | [https://gitlab.eduxiji.net/educg-group-12619-928705/1120191244-1813](https://gitlab.eduxiji.net/educg-group-12619-928705/1120191244-1813)                     |
| 北京理工大学           | 魏公村汽修厂                        | [https://gitlab.eduxiji.net/educg-group-12619-928705/LugerW-642](https://gitlab.eduxiji.net/educg-group-12619-928705/LugerW-642)                               |
| 华中科技大学           | HustSysy                            | [https://gitlab.eduxiji.net/educg-group-12619-928705/U201915125-268](https://gitlab.eduxiji.net/educg-group-12619-928705/U201915125-268)                       |

### 2023

#### ARM赛道参赛作品开源地址

| 队伍ID          | 学校             | 队伍名称             | fork仓库地址                                                                                                                                                        |
| --------------- | ---------------- | -------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 202310006201934 | 北京航空航天大学 | 那一年喵喵变成了光   | [https://gitlab.eduxiji.net/educg-group-18973-1895971/compiler2023-202310006201934](https://gitlab.eduxiji.net/educg-group-18973-1895971/compiler2023-202310006201934) |
| 202310006201725 | 北京航空航天大学 | ATRI                 | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202310006201725-78](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310006201725-78)                     |
| 202310614201437 | 电子科技大学     | ARM32栈错误          | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202310614201437-2550](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310614201437-2550)                 |
| 202310246201860 | 复旦大学         | 去偷毕昇杯           | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202310246201860-767](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310246201860-767)                   |
| 202310532201184 | 湖南大学         | 卷涡鸣人             | [https://gitlab.com/YfsBox/Compiler2023-yfscc.git](https://gitlab.com/YfsBox/Compiler2023-yfscc.git)                                                                   |
| 202310055201422 | 南开大学         | NKUF4                | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202310055201422-212](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310055201422-212)                   |
| 202310055201427 | 南开大学         | 没有op就不配拿奖吗   | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202310055201427-2607](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310055201427-2607)                 |
| 202310246201891 | 复旦大学         | XFD（已经摆烂）      | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202310246201891-3670](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310246201891-3670)                 |
| 202310336201869 | 杭州电子科技大学 | RV64段错误           | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202310336201869-343](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310336201869-343)                   |
| 202310285201433 | 苏州大学         | only-my-compiler     | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202310285201433-2498](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310285201433-2498)                 |
| 202310007201712 | 北京理工大学     | 叮咚队               | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202310007201712-1407](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310007201712-1407)                 |
| 202310614201747 | 电子科技大学     | 美丽无敌队           | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202310614201747-460](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310614201747-460)                   |
| 202390002201726 | 国防科技大学     | 理论结合实践队       | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202390002201726-1057](https://gitlab.eduxiji.net/educg-group-18973-1895971/202390002201726-1057)                 |
| 202390002201745 | 国防科技大学     | 我们四个真厉害       | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202390002201745-1165](https://gitlab.eduxiji.net/educg-group-18973-1895971/202390002201745-1165)                 |
| 202310699201728 | 西北工业大学     | 烫烫烫烫烫           | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202310699201728-813](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310699201728-813)                   |
| 202310699201677 | 西北工业大学     | 鸽鸽鸽鸽             | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202310699201677-2350](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310699201677-2350)                 |
| 202310006201717 | 北京航空航天大学 | 快码加编队           | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202310006201717-3182](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310006201717-3182)                 |
| 202310055201751 | 南开大学         | 北关大学第83号代表队 | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202310055201751-2501](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310055201751-2501)                 |
| 202310006201896 | 北京航空航天大学 | 编译三缺一           | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202310006201896-1138](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310006201896-1138)                 |
| 202310006201909 | 北京航空航天大学 | 高低起个队名         | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202310006201909-3107](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310006201909-3107)                 |
| 202310055201549 | 南开大学         | 诚招广告代理         | [https://gitlab.eduxiji.net/educg-group-18973-1895971/202310055201549-249](https://gitlab.eduxiji.net/educg-group-18973-1895971/202310055201549-249)                   |

#### RISCV赛道参赛作品开源地址

| 队伍ID          | 学校                   | 队伍名称         | fork仓库地址                                                                                                                                        |
| --------------- | ---------------------- | ---------------- | --------------------------------------------------------------------------------------------------------------------------------------------------- |
| 202314325201374 | 南方科技大学           | CMMC             | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202314325201374-1031](https://gitlab.eduxiji.net/educg-group-17291-1894922/202314325201374-1031) |
| 202310007201692 | 北京理工大学           | bit.newnewcc     | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310007201692-1245](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310007201692-1245) |
| 202310558201558 | 中山大学               | Yat-CC           | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310558201558-3109](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310558201558-3109) |
| 202310006201898 | 北京航空航天大学       | 喵喵队引体向上   | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310006201898-2685](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310006201898-2685) |
| 202310055201721 | 南开大学               | 生成式智能人工队 | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310055201721-710](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310055201721-710)   |
| 202310358201729 | 中国科学技术大学       | ggvm             | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310358201729-1528](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310358201729-1528) |
| 202310358201709 | 中国科学技术大学       | Compiling!       | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310358201709-501](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310358201709-501)   |
| 202310007201641 | 北京理工大学           | ForStar          | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310007201641-4047](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310007201641-4047) |
| 202310013201098 | 北京邮电大学           | kirara           | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310013201098-3687](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310013201098-3687) |
| 202310614201204 | 电子科技大学           | 编译青春         | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310614201204-3168](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310614201204-3168) |
| 202310145201386 | 东北大学               | 3TLE3WA          | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310145201386-3016](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310145201386-3016) |
| 202390002201723 | 国防科技大学           | 久远寺有珠       | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202390002201723-738](https://gitlab.eduxiji.net/educg-group-17291-1894922/202390002201723-738)   |
| 202310359201848 | 合肥工业大学           | 起床睡觉队       | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310359201848-2384](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310359201848-2384) |
| 202310487201029 | 华中科技大学           | toge             | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310487201029-2187](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310487201029-2187) |
| 202310486201516 | 武汉大学               | NOP              | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310486201516-942](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310486201516-942)   |
| 202318123201313 | 哈尔滨工业大学（深圳） | CompilerHIT      | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202318123201313-2320](https://gitlab.eduxiji.net/educg-group-17291-1894922/202318123201313-2320) |
| 202310561201924 | 华南理工大学           | SCUT_OPTIMIZE    | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310561201924-639](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310561201924-639)   |
| 202310487201496 | 华中科技大学           | 译一事,吾亦试    | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310487201496-3823](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310487201496-3823) |
| 202310146201457 | 辽宁科技大学           | YetJustSysyc     | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310146201457-3999](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310146201457-3999) |
| 202310284201911 | 南京大学               | CompilerBagel    | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310284201911-1365](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310284201911-1365) |
| 202310284201919 | 南京大学               | 可莉不知道呦     | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310284201919-3046](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310284201919-3046) |
| 202314039201490 | 四川大学锦江学院       | RJ430652         | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202314039201490-116](https://gitlab.eduxiji.net/educg-group-17291-1894922/202314039201490-116)   |
| 202310112201855 | 太原理工大学           | 开摆             | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310112201855-1924](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310112201855-1924) |
| 202310006201733 | 北京航空航天大学       | 药枣杞组         | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310006201733-3833](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310006201733-3833) |
| 202310006201895 | 北京航空航天大学       | HeapCorruption   | [https://gitlab.eduxiji.net/educg-group-17291-1894922/202310006201895-1405](https://gitlab.eduxiji.net/educg-group-17291-1894922/202310006201895-1405) |
| 202310487201551 | 华中科技大学           | 大萝卜编译生产队 | [https://gitlab.eduxiji.net/educg-group-18973-1895971/carrotcompiler](https://gitlab.eduxiji.net/educg-group-18973-1895971/carrotcompiler)             |

### 2024

### ARM赛道参赛作品开源地址

| 队伍ID           | 队伍名称              | 学校             | fork仓库地址                                                                                                                                          |
| ---------------- | --------------------- | ---------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- |
| T202410006203413 | 喵喵队花样滑冰        | 北京航空航天大学 | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203413-1955](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203413-1955) |
| T202410006203854 | 周末疯狂拼            | 北京航空航天大学 | [https://gitlab.eduxiji.net/educg-group-26172-2487152/T202410006203854-3456](https://gitlab.eduxiji.net/educg-group-26172-2487152/T202410006203854-3456) |
| T202410006203618 | 原末鸣初              | 北京航空航天大学 | [https://gitlab.eduxiji.net/educg-group-26172-2487152/T202410006203618-2881](https://gitlab.eduxiji.net/educg-group-26172-2487152/T202410006203618-2881) |
| T202410013203408 | OneLastCompiler       | 北京邮电大学     | [https://gitlab.eduxiji.net/educg-group-26172-2487152/T202410013203408-4017](https://gitlab.eduxiji.net/educg-group-26172-2487152/T202410013203408-4017) |
| T202410614203530 | 伪指令                | 电子科技大学     | [https://gitlab.eduxiji.net/educg-group-26172-2487152/T202410614203530-2812](https://gitlab.eduxiji.net/educg-group-26172-2487152/T202410614203530-2812) |
| T202410246203809 | Compiler_vs_Bugs      | 复旦大学         | [https://gitlab.eduxiji.net/educg-group-26172-2487152/T202410246203809-555](https://gitlab.eduxiji.net/educg-group-26172-2487152/T202410246203809-555)   |
| T202490002203625 | 编译你好香            | 国防科技大学     | [https://gitlab.eduxiji.net/educg-group-26172-2487152/compiler2024-sysy](https://gitlab.eduxiji.net/educg-group-26172-2487152/compiler2024-sysy)         |
| T202490002203562 | 牛牛向前队            | 国防科技大学     | [https://gitlab.eduxiji.net/educg-group-26172-2487152/T202490002203562-586](https://gitlab.eduxiji.net/educg-group-26172-2487152/T202490002203562-586)   |
| T202410336203416 | pinyinggaoshou        | 杭州电子科技大学 | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410336203416-1229](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410336203416-1229) |
| T202410561203866 | 汪汪队立大功队        | 华南理工大学     | [https://gitlab.eduxiji.net/educg-group-26172-2487152/T202410561203866-2221](https://gitlab.eduxiji.net/educg-group-26172-2487152/T202410561203866-2221) |
| T202410284203580 | BanGDream!It’sMySYSY | 南京大学         | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410284203580-3764](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410284203580-3764) |
| T202411065203807 | 编译成蓝色疾旋鼬      | 青岛大学         | [https://gitlab.eduxiji.net/educg-group-26172-2487152/T202411065203807-3933](https://gitlab.eduxiji.net/educg-group-26172-2487152/T202411065203807-3933) |
| T202410699203500 | 正道的光              | 西北工业大学     | [https://gitlab.eduxiji.net/educg-group-26172-2487152/T202410699203500-3366](https://gitlab.eduxiji.net/educg-group-26172-2487152/T202410699203500-3366) |

sds


### RISCV赛道参赛作品开源地址

| 队伍ID           | 队伍名称                           | 学校                     | fork仓库地址                                                                                                                                          |
| ---------------- | ---------------------------------- | ------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------- |
| T202410006203585 | 睿睿也想打编译队                   | 北京航空航天大学         | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203585-3730](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203585-3730) |
| T202410006203374 | 四元式                             | 北京航空航天大学         | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203374-422](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203374-422)   |
| T202410006202880 | 逸动山楂队                         | 北京航空航天大学         | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006202880-2761](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006202880-2761) |
| T202410006203450 | 海底小纵队                         | 北京航空航天大学         | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203450-2508](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203450-2508) |
| T202410006203387 | YellowYaks                         | 北京航空航天大学         | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203387-1337](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203387-1337) |
| T202410006203104 | 这是个队名队                       | 北京航空航天大学         | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203104-3288](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203104-3288) |
| T202410006203413 | 喵喵队花样滑冰                     | 北京航空航天大学         | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203413-1955](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203413-1955) |
| T202410336203416 | pinyinggaoshou                     | 杭州电子科技大学         | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410336203416-1229](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410336203416-1229) |
| T202410284203423 | 冲向广州队                         | 南京大学                 | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410284203423-2518](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410284203423-2518) |
| T202410006203109 | NEL                                | 北京航空航天大学         | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203109-3846](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203109-3846) |
| T202410006203533 | 素履“译”往队                     | 北京航空航天大学         | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203533-3019](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410006203533-3019) |
| T202410027203528 | ACM2Compiler                       | 北京师范大学             | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410027203528-232](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410027203528-232)   |
| T202410614202951 | gpt5.0                             | 电子科技大学             | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410614202951-722](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410614202951-722)   |
| T202410614203403 | 编编又译译                         | 电子科技大学             | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410614203403-1360](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410614203403-1360) |
| T202490002203537 | 三进制冒险家                       | 国防科技大学             | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202490002203537-615](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202490002203537-615)   |
| T202490002203688 | firefox                            | 国防科技大学             | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202490002203688-1262](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202490002203688-1262) |
| T202418123202876 | duskphantom                        | 哈尔滨工业大学（深圳）   | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202418123202876-2939](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202418123202876-2939) |
| T202410359203149 | 星晴                               | 合肥工业大学             | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410359203149-675](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410359203149-675)   |
| T202419359203337 | 决不放弃                           | 合肥工业大学（宣城校区） | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202419359203337-3752](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202419359203337-3752) |
| T202410269203414 | 起床困难户                         | 华东师范大学             | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410269203414-1848](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410269203414-1848) |
| T202410487203858 | 呼啦啦队                           | 华中科技大学             | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410487203858-2311](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410487203858-2311) |
| T202410284203623 | NNVM                               | 南京大学                 | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410284203623-538](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410284203623-538)   |
| T202410284203546 | 蚂蚁派                             | 南京大学                 | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410284203546-2971](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410284203546-2971) |
| T202410055203436 | 人工式生成智能                     | 南开大学                 | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410055203436-1338](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410055203436-1338) |
| T202410055203113 | 八云蓝架构编译器与RE的橙           | 南开大学                 | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410055203113-826](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410055203113-826)   |
| T202410003203847 | RRVM                               | 清华大学                 | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410003203847-1935](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410003203847-1935) |
| T202410003203057 | return_0;                          | 清华大学                 | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410003203057-1317](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410003203057-1317) |
| T202410590203330 | 文山湖之狼                         | 深圳大学                 | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410590203330-200](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410590203330-200)   |
| T202410486202978 | Compiler_in_C_Minor                | 武汉大学                 | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410486202978-1811](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410486202978-1811) |
| T202410699203233 | honkaiCC                           | 西北工业大学             | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410699203233-2083](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410699203233-2083) |
| T202410423203862 | 水军出击                           | 中国海洋大学             | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410423203862-1353](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410423203862-1353) |
| T202410356203271 | nhwc                               | 中国计量大学             | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410356203271-2091](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410356203271-2091) |
| T202410358203100 | 一刻也没有为段错误而哀悼           | 中国科学技术大学         | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410358203100-2802](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410358203100-2802) |
| T202410358203143 | 青春科蝻不会梦到CE队               | 中国科学技术大学         | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410358203143-3735](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410358203143-3735) |
| T202410558203459 | 四个圣甲虫                         | 中山大学                 | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410558203459-3564](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410558203459-3564) |
| T202410558203130 | 派大星说搞优化就像光头强抓美羊羊😋 | 中山大学                 | [https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410558203130-3063](https://gitlab.eduxiji.net/educg-group-26173-2487151/T202410558203130-3063) |
