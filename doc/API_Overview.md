# NUDT SysY Compiler Framework API Documentation

## Overview

The NUDT SysY Compiler Framework is a high-performance compiler infrastructure designed for the SysY programming language. It provides a comprehensive intermediate representation (IR) system, extensive optimization passes, and multiple backend targets.

## Architecture

### Core Components

#### 1. IR (Intermediate Representation) System
Located in `include/ir/` and `src/ir/`

**Key Classes:**
- **`ir::Module`** - Top-level container for all IR constructs
- **`ir::Function`** - Function definitions with SSA form
- **`ir::BasicBlock`** - Basic blocks containing instructions
- **`ir::Value`** - Base class for all values (SSA values)
- **`ir::User`** - Base class for values that use other values
- **`ir::Instruction`** - Base class for all IR instructions
- **`ir::IRBuilder`** - High-level interface for constructing IR

**Features:**
- Static Single Assignment (SSA) form
- Strong type system with `ir::Type` hierarchy
- Comprehensive def-use chain management
- LLVM IR compatibility through serialization

#### 2. Pass Framework
Located in `include/pass/` and `src/pass/`

**Core Classes:**
- **`pass::Pass<T>`** - Template base class for all passes
- **`pass::PassManager`** - Manages pass execution
- **`pass::TopAnalysisInfoManager`** - Centralized analysis result caching

**Pass Types:**
- **Analysis Passes**: Compute information about the IR
  - Dominance analysis (`DomTree`, `PDomTree`)
  - Loop analysis (`LoopInfo`)
  - Call graph analysis (`CallGraph`)
  - Induction variable analysis (`IndVarInfo`)
  - Dependence analysis (`DependenceInfo`)

- **Optimization Passes**: Transform the IR to improve performance
  - Scalar optimizations (SCCP, GVN, LICM, etc.)
  - Control flow optimizations (SimplifyCFG, BlockSort)
  - Dead code elimination (DCE, ADCE, DSE, etc.)
  - Loop optimizations (unrolling, peeling, parallelization)
  - Interprocedural optimizations (inlining, TCO)

#### 3. MIR (Machine IR) Backend
Located in `include/mir/` and `src/mir/`

**Key Components:**
- **Machine-level IR** for target-specific code generation
- **Register allocation** with graph coloring
- **Instruction selection** with template-based matching
- **Instruction scheduling** for performance optimization

#### 4. Target Support
Located in `include/target/` and `src/target/`

**Supported Targets:**
- **RISC-V**: Complete backend with optimizations
- **ARM**: Backend support for ARM architectures

### Usage Examples

#### Creating IR Programmatically

```cpp
#include "ir/ir.hpp"
#include "ir/builder.hpp"

// Create a module
auto module = std::make_unique<ir::Module>();

// Create a function
auto funcType = ir::FunctionType::gen(ir::Type::TypeInt32(), {});
auto func = module->addFunction(funcType, "main");

// Create basic blocks
auto entry = func->newEntry("entry");
auto builder = ir::IRBuilder();
builder.setInsetPosEnd(entry);

// Create instructions
auto constant = ir::ConstantInt::gen(42, ir::Type::TypeInt32());
auto retInst = builder.makeInst<ir::ReturnInst>(constant);
```

#### Writing an Optimization Pass

```cpp
#include "pass/pass.hpp"

class MyOptimization : public pass::FunctionPass {
public:
  void run(ir::Function* func, pass::TopAnalysisInfoManager* aim) override {
    // Get analysis information
    auto domTree = aim->getDomTree(func);
    auto loopInfo = aim->getLoopInfo(func);
    
    // Perform optimization
    for (auto& block : func->blocks()) {
      // Transform the block
    }
  }
  
  std::string name() const override {
    return "MyOptimization";
  }
};
```

#### Running Passes

```cpp
#include "pass/pass.hpp"

// Create pass manager
pass::TopAnalysisInfoManager aim(module.get());
pass::PassManager pm(module.get(), &aim);

// Run individual passes
auto myPass = std::make_unique<MyOptimization>();
pm.run(myPass.get());

// Run pass sequences
pm.runPasses({"mem2reg", "dce", "sccp", "simplifycfg"});
```

## Key Design Principles

### 1. SSA Form
All IR is maintained in Static Single Assignment form where:
- Each variable is assigned exactly once
- Each use refers to exactly one definition
- Phi nodes handle control flow merges

### 2. Type Safety
Strong typing throughout the system:
- Every value has an associated type
- Type checking prevents invalid operations
- Automatic type conversions where appropriate

### 3. Def-Use Chains
Efficient tracking of value usage:
- Each value maintains a list of its uses
- Easy to find all uses of a definition
- Supports efficient transformations

### 4. Modular Design
Clean separation of concerns:
- IR is independent of optimization passes
- Analysis passes separate from transformation passes
- Backend is independent of frontend

### 5. Extensibility
Easy to add new components:
- Template-based pass framework
- Pluggable analysis system
- Extensible type system

## Building and Documentation

### Prerequisites
- CMake 3.15+
- C++17 compiler
- ANTLR4 runtime
- Doxygen (for documentation)

### Building the Project
```bash
mkdir build
cmake -S . -B build
cmake --build build -j$(nproc)
```

### Generating Documentation
```bash
doxygen Doxyfile
# Documentation will be generated in ./.doxy/
```

## Contributing

When adding new functionality:

1. **Follow the existing patterns** - Use the established class hierarchies
2. **Add comprehensive documentation** - All public APIs should be documented
3. **Write tests** - Ensure new functionality is tested
4. **Update this overview** - Keep the documentation current

## Performance Characteristics

The compiler framework is designed for high performance:

- **Memory efficient**: Arena-based allocation for IR objects
- **Cache friendly**: Compact data structures and access patterns
- **Scalable**: Efficient algorithms for large programs
- **Parallel**: Some passes support parallel execution

## Error Handling

The framework uses several error handling strategies:

- **Assertions**: For programming errors and invariant violations
- **Return codes**: For recoverable errors
- **Exceptions**: Sparingly used, mainly for resource allocation failures
- **Verification**: IR verification passes to catch corruption

## Debugging Support

Built-in debugging features:

- **IR printing**: Human-readable IR output
- **Verification passes**: Detect IR corruption
- **Pass timing**: Measure pass execution time
- **Debug information**: Preserve source location information

---

For detailed API documentation, see the generated Doxygen documentation.
For implementation details, refer to the source code and inline comments.