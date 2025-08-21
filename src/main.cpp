/**
 * @file main.cpp
 * @brief Entry point for the NUDT SysY Compiler
 * 
 * This file contains the main function that serves as the entry point for the
 * NUDT SysY compiler. It handles command-line argument parsing, configuration
 * validation, and initiates the compilation pipeline.
 * 
 * The compiler supports various modes of operation including:
 * - IR generation and optimization
 * - Assembly code generation
 * - Debug output generation
 * - Performance profiling
 * 
 * @see driver.hpp for the main compilation pipeline
 * @see support/config.hpp for configuration options
 */

#include <iostream>
#include "driver/driver.hpp"
#include "support/config.hpp"

using namespace std;

/// Global reference to the compiler configuration singleton
static auto& config = sysy::Config::getInstance();

/**
 * @brief Main entry point for the NUDT SysY Compiler
 * 
 * This function initializes the compiler by:
 * 1. Parsing command-line arguments to configure compilation options
 * 2. Validating that required input files are specified
 * 3. Displaying configuration information (if requested)
 * 4. Invoking the main compilation pipeline
 * 
 * Command-line usage examples:
 * - Test mode: `./compiler -f test.c -i -t mem2reg -o gen.ll -O0 -L0`
 * - Submit mode: `./compiler -S -o testcase.s testcase.sy`
 * 
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 * @return EXIT_SUCCESS on successful compilation, EXIT_FAILURE on error
 * 
 * @see compilerPipeline() for the main compilation process
 * @see sysy::Config::parseCmdArgs() for argument parsing details
 */
int main(int argc, char* argv[]) {
  // Parse command-line arguments and configure the compiler
  config.parseCmdArgs(argc, argv);
  config.print_info();

  // Validate that an input file was specified
  if (config.infile.empty()) {
    cerr << "Error: input file not specified" << endl;
    config.print_help();
    return EXIT_FAILURE;
  }

  // Execute the main compilation pipeline
  compilerPipeline();
  return EXIT_SUCCESS;
}