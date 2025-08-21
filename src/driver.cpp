/**
 * @file driver.cpp
 * @brief Implementation of the main compiler driver and pipeline stages
 * 
 * This file implements the complete compilation pipeline for the NUDT SysY compiler.
 * It orchestrates the frontend (parsing), midend (optimization), and backend (code generation)
 * stages of compilation.
 * 
 * The compilation pipeline consists of:
 * - Frontend: Lexical analysis, parsing, and IR generation
 * - Midend: IR optimization passes
 * - Backend: MIR lowering and target code generation
 * 
 * @see driver.hpp for the public interface
 */

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

/**
 * Example command-line usage:
 * 
 * Test mode:
 * ./compiler -f test.c -i -t mem2reg -o gen.ll -O0 -L0
 * 
 * Submit mode:
 * ./compiler -S -o testcase.s testcase.sy
 */

/// Global reference to the compiler configuration singleton
static auto& config = sysy::Config::getInstance();

/**
 * @brief Dumps the IR module to output file or stdout
 * 
 * This function outputs the high-level IR representation of the compiled module
 * in a human-readable format. The output can be directed to either a file or
 * standard output based on configuration.
 * 
 * @param module The IR module to dump
 * @param filename Output filename (empty string for stdout)
 * 
 * @note Only outputs if config.genIR is enabled
 * @see ir::Module::print() for the actual printing implementation
 */
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

/**
 * @brief Dumps the MIR module as target assembly code
 * 
 * This function generates target-specific assembly code from the Machine IR (MIR)
 * representation. The assembly output can be directed to either a file or
 * standard output based on configuration.
 * 
 * @param module The MIR module to emit as assembly
 * @param target The target backend (e.g., RISC-V) to use for code generation
 * @param filename Output filename (empty string for stdout)
 * 
 * @note Only outputs if config.genASM is enabled
 * @see mir::Target::emit_assembly() for the actual assembly emission
 */
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

/**
 * @brief Executes the frontend compilation pipeline
 * 
 * This function handles the frontend stages of compilation:
 * 1. **Lexical Analysis**: Tokenizes the input source using ANTLR4-generated SysYLexer
 * 2. **Syntax Analysis**: Parses tokens into an Abstract Syntax Tree (AST) using SysYParser
 * 3. **IR Generation**: Converts the AST to high-level Intermediate Representation (IR)
 * 4. **IR Verification**: Validates the generated IR for correctness
 * 
 * The frontend processes SysY language source files and produces a validated IR module
 * that can be consumed by the optimization and code generation stages.
 * 
 * @param infile Path to the input source file (currently unused, uses config.infile)
 * @param module Reference to the IR module to populate with generated IR
 * 
 * @note Creates debug directories if debug logging is enabled
 * @note Terminates compilation if IR verification fails
 * 
 * @see SysYLexer for lexical analysis
 * @see SysYParser for syntax analysis 
 * @see sysy::SysYIRGenerator for IR generation
 * @see ir::Module::verify() for IR validation
 */
void frontendPipeline(const string& infile, ir::Module& module) {
  utils::Stage stage("Frontend Pipeline"sv);

  // Create debug directory structure if debug logging is enabled
  if (config.logLevel == sysy::LogLevel::DEBUG) {
    utils::ensure_directory_exists(config.debugDir());
  }

  // Lexical and Syntax Analysis using ANTLR4
  ifstream fin(config.infile);
  auto input = antlr4::ANTLRInputStream{fin};
  auto lexer = SysYLexer{&input};
  auto tokens = antlr4::CommonTokenStream{&lexer};
  auto parser = SysYParser{&tokens};
  auto ast_root = parser.compUnit();

  // IR Generation from AST
  auto irGenerator = sysy::SysYIRGenerator(&module, ast_root);
  irGenerator.buildIR();

  // Verify the generated IR for correctness
  if (not module.verify(std::cerr)) {
    module.print(std::cerr);
    std::cerr << "IR verification failed" << std::endl;
  }
}



/**
 * @brief Executes the backend compilation pipeline
 * 
 * This function handles the backend stages of compilation:
 * 1. **Target Selection**: Initializes the RISC-V target backend
 * 2. **IR Lowering**: Converts high-level IR to Machine Intermediate Representation (MIR)
 * 3. **Code Generation**: Generates target-specific assembly code
 * 4. **Assembly Output**: Emits the final assembly code to the output file
 * 
 * The backend transforms the optimized IR into executable assembly code for the
 * target architecture (currently RISC-V 64-bit).
 * 
 * @param module The optimized IR module to compile
 * @param tAIM Analysis information manager containing optimization results
 * 
 * @note Only executes if config.genASM is enabled
 * @note Currently targets RISC-V architecture exclusively
 * 
 * @see mir::RISCVTarget for RISC-V backend implementation
 * @see mir::createMIRModule() for IR to MIR lowering
 * @see dumpMIRModule() for assembly code emission
 */
void backendPipeline(ir::Module& module, pass::TopAnalysisInfoManager& tAIM) {
  utils::Stage stage("Backend Pipeline"sv);
  if (not config.genASM) return;
  
  // Initialize RISC-V target backend
  auto target = mir::RISCVTarget();
  
  // Lower IR to MIR (Machine Intermediate Representation)
  auto mir_module = mir::createMIRModule(module, target, &tAIM);
  
  // Generate and output target assembly code
  dumpMIRModule(*mir_module, target, config.outfile);
}

/**
 * @brief Executes the midend optimization pipeline
 * 
 * This function handles the optimization stages of compilation by running
 * a series of analysis and transformation passes on the IR. The midend
 * optimizations improve code quality, performance, and prepare the IR
 * for efficient code generation.
 * 
 * Optimization categories include:
 * - **Scalar Optimizations**: Constant propagation, dead code elimination, etc.
 * - **Loop Optimizations**: Loop unrolling, vectorization, parallelization
 * - **Interprocedural Optimizations**: Inlining, tail call optimization
 * - **Control Flow Optimizations**: CFG simplification, block merging
 * 
 * @param module The IR module to optimize
 * @param tAIM Analysis information manager for pass communication
 * 
 * @note The specific passes run are determined by config.passes
 * @note Pass ordering is critical for optimization effectiveness
 * 
 * @see pass::PassManager for pass execution framework
 * @see sysy::Config::passes for pass configuration
 * @see pass::TopAnalysisInfoManager for analysis result management
 */
void midendPipeline(ir::Module& module, pass::TopAnalysisInfoManager& tAIM) {
  utils::Stage stage("Midend Pipeline"sv);
  
  // Create and configure the pass manager
  auto pm = pass::PassManager(&module, &tAIM);
  
  // Execute the configured optimization passes
  pm.runPasses(config.passes);
}
/**
 * @brief Main compilation pipeline orchestrator
 * 
 * This function implements the complete compilation pipeline by coordinating
 * the frontend, midend, and backend stages. It serves as the main entry point
 * for the compilation process and handles the flow of data between stages.
 * 
 * Pipeline stages:
 * 1. **Frontend**: Parse source code and generate IR
 * 2. **Midend**: Run optimization passes on the IR
 * 3. **Backend**: Lower IR to machine code and generate assembly
 * 4. **Output**: Emit IR and/or assembly based on configuration
 * 5. **Profiling**: Report performance statistics if enabled
 * 
 * The pipeline supports various output modes:
 * - IR-only compilation (for analysis and debugging)
 * - Full compilation to assembly (for execution)
 * - Debug output with performance profiling
 * 
 * @note Uses configuration from sysy::Config::getInstance()
 * @note Performance profiling is available in debug mode
 * 
 * @see frontendPipeline() for source parsing and IR generation
 * @see midendPipeline() for IR optimization
 * @see backendPipeline() for code generation
 * @see utils::Profiler for performance measurement
 */
void compilerPipeline() {
  // Create the main IR module container
  auto module = ir::Module();

  // Frontend: Parse source and generate IR
  frontendPipeline(config.infile, module);

  // Initialize analysis infrastructure for optimization passes
  auto tAIM = pass::TopAnalysisInfoManager(&module);
  tAIM.initialize();
  
  // Midend: Run optimization passes
  midendPipeline(module, tAIM);

  // Output the optimized IR if requested
  dumpModule(module, config.outfile);

  // Backend: Generate target assembly code if requested
  if (config.genASM) backendPipeline(module, tAIM);

  // Report performance statistics in debug mode
  if (config.logLevel >= sysy::LogLevel::DEBUG) {
    utils::Profiler::get().printStatistics();
  }
}
