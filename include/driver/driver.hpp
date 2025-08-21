#pragma once

/**
 * @file driver.hpp
 * @brief Main compiler driver interface
 * 
 * This header provides the main entry point for the NUDT SysY compiler pipeline.
 * It orchestrates the entire compilation process from source code to target assembly.
 */

/**
 * @brief Executes the complete compilation pipeline
 * 
 * This function serves as the main entry point for the compilation process.
 * It coordinates the following stages:
 * 
 * 1. **Lexical Analysis**: Tokenizes the input source code using ANTLR4-generated lexer
 * 2. **Syntax Analysis**: Parses tokens into an Abstract Syntax Tree (AST)
 * 3. **Semantic Analysis**: Performs type checking and builds symbol tables
 * 4. **IR Generation**: Converts AST to high-level Intermediate Representation (IR)
 * 5. **IR Optimization**: Applies various optimization passes to the IR
 * 6. **MIR Lowering**: Lowers IR to Machine Intermediate Representation (MIR)
 * 7. **Code Generation**: Generates target-specific assembly code (RISC-V)
 * 
 * The pipeline supports various configuration options including:
 * - Optimization levels (O0, O1, O2, O3)
 * - Individual pass selection and ordering
 * - Debug output generation
 * - Parallel compilation features
 * 
 * @note This function uses global configuration from sysy::Config::getInstance()
 * @see sysy::Config for configuration options
 * @see ir::Module for the high-level IR representation
 * @see mir::MIRModule for the machine-level IR representation
 */
void compilerPipeline();