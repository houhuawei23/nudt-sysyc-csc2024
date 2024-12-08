#include "support/config.hpp"
#include "support/Profiler.hpp"

#include "ir/ir.hpp"

#include <cstring>
#include <getopt.h>
#include <string_view>
#include <fstream>
#include <iostream>

using namespace std::string_view_literals;

namespace sysy {

/*
-i: Generate IR
-t {passname} {pasename} ...: opt passes names to run
-o {filename}:  output file, default gen.ll (-ir) or gen.s (-S)
-S: gen assembly
-O[0-3]: opt level

./compiler-f test.c -i -t mem2reg dce -o gen.ll
./compiler -f test.c -i -t mem2reg -o gen.ll -O0 -L0
*/

std::string_view HELP = R"(
Usage: ./compiler [options]
  -f {filename}         input file
  -i                    Generate IR
  -t {passname} ...     opt passes names to run
  -o {filename}         output file, default gen.ll (-ir) or gen.s (-S)
  -S                    gen assembly
  -O[0-3]               opt level
  -L[0-2]               log level: 0=SILENT, 1=INFO, 2=DEBUG

Examples:
$ ./compiler -f test.c -i -t mem2reg -o gen.ll -O0 -L0
$ ./compiler -f test.c -i -t mem2reg dce -o gen.ll
)";

void Config::print_help() {
  std::cout << HELP << std::endl;
}

void Config::print_info() {
  if (logLevel > LogLevel::SILENT) {
    std::cout << "In File  : " << infile << std::endl;
    std::cout << "Out File : " << outfile << std::endl;
    std::cout << "Gen IR   : " << (genIR ? "Yes" : "No") << std::endl;
    std::cout << "Gen ASM  : " << (genASM ? "Yes" : "No") << std::endl;
    std::cout << "Opt Level: " << optLevel << std::endl;
    std::cout << "Log Level: " << logLevel << std::endl;
    if (not passes.empty()) {
      std::cout << "Passes   : ";
      for (const auto& pass : passes) {
        std::cout << " " << pass;
      }
      std::cout << std::endl;
    }
  }
}

void Config::parseTestArgs(int argc, char* argv[]) {
  int option;
  while ((option = getopt(argc, argv, "f:it:o:SO:L:")) != -1) {
    switch (option) {
      case 'f':
        infile = optarg;
        break;
      case 'i':
        genIR = true;
        break;
      case 't':
        // optind start from 1, so we need to minus 1
        while (optind <= argc && *argv[optind - 1] != '-') {
          passes.push_back(argv[optind - 1]);
          optind++;
        }
        optind--;  // must!
        break;
      case 'o':
        outfile = optarg;
        break;
      case 'S':
        genASM = true;
        break;
      case 'O':
        optLevel = static_cast<OptLevel>(std::stoi(optarg));
        break;
      case 'L':
        logLevel = static_cast<LogLevel>(std::stoi(optarg));
        break;
      default:
        print_help();
        exit(EXIT_FAILURE);
    }
  }
}
void Config::dumpModule(ir::Module* module, const std::string& filename) const {
  auto path = debugDir() / fs::path(filename);
  std::cerr << "Dumping module to " << path << std::endl;
  std::ofstream out(path);
  module->rename();
  module->print(out);
}

static const auto ifCombinePassesList = std::vector<std::string>{
  "simplifycfg", "loopsimplify", "sccp", "adce",        "gcm",  "gvn",  "licm", "dle",
  "dse",         "dle",          "dse",  "instcombine", "adce", "sccp", "dlae",
};

static const auto basePasses = std::vector<std::string>{"mem2reg", "reg2mem"};

static const auto commonOptPasses =
  std::vector<std::string>{"sccp", "adce", "simplifycfg", "instcombine", "adce"};

static const auto loopOptPasses = std::vector<std::string>{"loopsimplify", "gcm", "gvn", "licm"};

static const auto parallelPasses = std::vector<std::string>{
  "loopsimplify", "gcm", "gvn", "licm",
  // "markpara",
  // "LoopInterChange", "inline", 
  "loopsimplify", 
  "blocksort",
  "cfgprint",
  "parallel",  // "ParallelBodyExtract",
  "inline", "simplifycfg"};

static const auto interProceduralPasses = std::vector<std::string>{
  "inline", "tco", "cache", "inline",  // cant parallel
};

static const auto afterUnrollPasses = std::vector<std::string>{
  "simplifycfg",
  "loopsimplify",
  "sccp",
  "adce",
  "gcm",
  "gvn",
  "licm",
  //  "dle", "dse",         "dle",          "dse",
  "instcombine",
  "adce",
  "sccp",
  "dlae",
};

static const auto gepSplitPasses = std::vector<std::string>{
  // "GepSplit",
  "scp", "dce", "simplifycfg", "instcombine", "scp", "dce",
};
static const auto deadLoopPasses = std::vector<std::string>{
  "ag2l",         "mem2reg", "loopdivest",   "sccp", "adce",        "instcombine", "simplifycfg",
  "loopsimplify", "idvrepl", "sccp",         "adce", "simplifycfg", "DeadLoop",    "simplifycfg",
  "adce",         "sccp",    "loopsimplify", "gcm",  "gvn",         "licm",        "gvn"};

// static const auto
auto collectPasses(OptLevel level) {
  if (level == OptLevel::O0) {
    return basePasses;
  }

  // O1
  std::vector<std::string> clcPasses;
  clcPasses.insert(clcPasses.end(), commonOptPasses.begin(), commonOptPasses.end());
  clcPasses.insert(clcPasses.end(), loopOptPasses.begin(), loopOptPasses.end());
  clcPasses.insert(clcPasses.end(), commonOptPasses.begin(), commonOptPasses.end());

  std::vector<std::string> passes;

  passes.push_back("mem2reg");

  passes.insert(passes.end(), clcPasses.begin(), clcPasses.end());

  // passes.insert(passes.end(), ifCombinePassesList.begin(), ifCombinePassesList.end());
  // passes.insert(passes.end(), deadLoopPasses.begin(), deadLoopPasses.end());

  // IPO
  // passes.insert(passes.end(), interProceduralPasses.begin(), interProceduralPasses.end());

  // passes.insert(passes.end(), clcPasses.begin(), clcPasses.end());

  // passes.push_back("markpara");
  // passes.insert(passes.end(), {"loopsimplify", "unroll"});

  // passes.insert(passes.end(), clcPasses.begin(), clcPasses.end());

  // dont add clc after
  passes.insert(passes.end(), parallelPasses.begin(), parallelPasses.end());

  // passes.insert(passes.end(), gepSplitPasses.begin(), gepSplitPasses.end());

  passes.push_back("reg2mem");
  return std::move(passes);
}

/*
5
功能测试：compiler -S -o testcase.s testcase.sy
6
性能测试：compiler -S -o testcase.s testcase.sy -O1
7
debug: compiler -S -o testcase.s testcase.sy -O1 -L2
*/
void Config::parseSubmitArgs(int argc, char* argv[]) {
  genASM = true;
  outfile = argv[3];
  infile = argv[4];

  if (argc >= 6) {
    if (argv[5] == "-O0"sv) optLevel = OptLevel::O1;
    if (argv[5] == "-O1"sv) optLevel = OptLevel::O1;
  }

  if (argc == 7) {
    if (argv[6] == "-L2"sv) {
      logLevel = LogLevel::DEBUG;
    }
  }

  /* 性能测试 */
  // if (argc == 6) {
  //   optLevel = OptLevel::O1;
  //   // std::cerr << "using default opt level -O1" << std::endl;
  // }
}

void Config::parseCmdArgs(int argc, char* argv[]) {
  if (argv[1] == "-f"sv) {
    parseTestArgs(argc, argv);
  } else if (argv[1] == "-S"sv) {
    parseSubmitArgs(argc, argv);
  } else {
    print_help();
    exit(EXIT_FAILURE);
  }
  if (passes.empty()) passes = collectPasses(optLevel);
}

}  // namespace sysy