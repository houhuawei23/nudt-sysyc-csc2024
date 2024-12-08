
#include <iostream>

#include "driver/driver.hpp"

#include "SysYLexer.h"
#include "visitor/visitor.hpp"
#include "pass/pass.hpp"

#include "mir/MIR.hpp"
#include "mir/target.hpp"
#include "mir/lowering.hpp"

#include "target/riscv/RISCV.hpp"
#include "target/riscv/RISCVTarget.hpp"

#include "support/config.hpp"
#include "support/FileSystem.hpp"
#include "support/Profiler.hpp"

namespace fs = std::filesystem;

using namespace std;
/*
config.parseTestArgs:
./compiler -f test.c -i -t mem2reg -o gen.ll -O0 -L0

config.parseSubmitArgs
./compiler -S -o testcase.s testcase.sy
 */
static auto& config = sysy::Config::getInstance();

void dumpModule(ir::Module& module, const string& filename) {
  if (not config.genIR) return;
  if (filename.empty()) {
    module.print(std::cout);
    return;
  } else {
    ofstream fout(filename);
    module.print(fout);
    return;
  }
}

void dumpMIRModule(mir::MIRModule& module, mir::Target& target, const string& filename) {
  if (not config.genASM) return;
  if (filename.empty()) {
    target.emit_assembly(std::cout, module);
    return;
  } else {
    ofstream fout(filename);
    target.emit_assembly(fout, module);
    return;
  }
}

void frontendPipeline(const string& infile, ir::Module& module) {
  utils::Stage stage("Frontend Pipeline"sv);

  // mkdir ./.debug/xxx/ for debug info
  if (config.logLevel == sysy::LogLevel::DEBUG) {
    utils::ensure_directory_exists(config.debugDir());
  }

  // utils::Stage antlrStage("ANTLR Parse"sv);

  ifstream fin(config.infile);
  auto input = antlr4::ANTLRInputStream{fin};
  auto lexer = SysYLexer{&input};
  auto tokens = antlr4::CommonTokenStream{&lexer};
  auto parser = SysYParser{&tokens};
  auto ast_root = parser.compUnit();

  // antlrStage.~Stage();

  // utils::Stage emitIRStage("IR Generation"sv);

  auto irGenerator = sysy::SysYIRGenerator(&module, ast_root);
  irGenerator.buildIR();

  // emitIRStage.~Stage();
  if (not module.verify(std::cerr)) {
    module.print(std::cerr);
    std::cerr << "IR verification failed" << std::endl;
  }
}



void backendPipeline(ir::Module& module, pass::TopAnalysisInfoManager& tAIM) {
  utils::Stage stage("Backend Pipeline"sv);
  if (not config.genASM) return;
  auto target = mir::RISCVTarget();
  auto mir_module = mir::createMIRModule(module, target, &tAIM);
  dumpMIRModule(*mir_module, target, config.outfile);
}

void midendPipeline(ir::Module& module, pass::TopAnalysisInfoManager& tAIM) {
  utils::Stage stage("Midend Pipeline"sv);
  auto pm = pass::PassManager(&module, &tAIM);
  pm.runPasses(config.passes);
}
void compilerPipeline() {
  auto module = ir::Module();

  frontendPipeline(config.infile, module);

  auto tAIM = pass::TopAnalysisInfoManager(&module);
  tAIM.initialize();
  midendPipeline(module, tAIM);

  dumpModule(module, config.outfile);

  if (config.genASM) backendPipeline(module, tAIM);

  if (config.logLevel >= sysy::LogLevel::DEBUG) {
    utils::Profiler::get().printStatistics();
  }
}
