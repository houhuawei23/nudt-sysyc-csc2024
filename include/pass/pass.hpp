#pragma once
#include <vector>
#include "ir/ir.hpp"
#include "pass/AnalysisInfo.hpp"

#include <chrono>
namespace pass {

/**
 * @brief Base class for all compiler passes
 * 
 * BasePass provides the common interface that all compiler passes must implement.
 * It uses type erasure to allow passes operating on different unit types
 * (Module, Function, BasicBlock) to be stored in the same containers.
 * 
 * @see Pass for the templated version that provides type safety
 */
class BasePass {
public:
  /**
   * @brief Runs the pass on a given unit
   * @param pass_unit The unit to run the pass on (type-erased)
   * @param tp The analysis info manager for accessing analysis results
   */
  virtual void run(void* pass_unit, TopAnalysisInfoManager* tp) = 0;
  
  /**
   * @brief Gets the name of the pass
   * @return The pass name as a string
   */
  virtual std::string name() const = 0;

  /// Virtual destructor for proper cleanup
  virtual ~BasePass() = default;
};

/**
 * @brief Templated base class for type-safe compiler passes
 * 
 * Pass<T> provides a type-safe interface for passes that operate on specific
 * IR unit types. It automatically handles the type casting from the type-erased
 * BasePass interface.
 * 
 * @tparam PassUnit The type of IR unit this pass operates on (Module, Function, BasicBlock)
 * 
 * Example usage:
 * @code
 * class MyOptimization : public Pass<ir::Function> {
 * public:
 *   void run(ir::Function* func, TopAnalysisInfoManager* tp) override {
 *     // Optimization logic here
 *   }
 *   std::string name() const override { return "MyOptimization"; }
 * };
 * @endcode
 */
template <typename PassUnit>
class Pass : public BasePass {
public:
  /**
   * @brief Type-erased run method (implements BasePass interface)
   * @param pass_unit The unit to run the pass on (type-erased)
   * @param tp The analysis info manager
   */
  void run(void* pass_unit, TopAnalysisInfoManager* tp) override {
    run(static_cast<PassUnit*>(pass_unit), tp);
  }
  
  /**
   * @brief Type-safe run method (to be implemented by derived classes)
   * @param pass_unit The unit to run the pass on (properly typed)
   * @param tp The analysis info manager for accessing analysis results
   */
  virtual void run(PassUnit* pass_unit, TopAnalysisInfoManager* tp) = 0;
  
  /**
   * @brief Gets the name of the pass
   * @return The pass name as a string
   */
  virtual std::string name() const = 0;
  
  /// Virtual destructor to allow proper cleanup of derived classes
  virtual ~Pass() = default;
};

/// @brief Convenience alias for passes that operate on modules
using ModulePass = Pass<ir::Module>;

/// @brief Convenience alias for passes that operate on functions  
using FunctionPass = Pass<ir::Function>;

/// @brief Convenience alias for passes that operate on basic blocks
using BasicBlockPass = Pass<ir::BasicBlock>;

/**
 * @brief Manages the execution of compiler passes
 * 
 * PassManager is responsible for running passes on the IR in the correct order
 * and managing the analysis information that passes depend on. It provides
 * methods to run individual passes or sequences of passes.
 * 
 * The PassManager works with the TopAnalysisInfoManager to ensure that
 * analysis results are properly cached and invalidated when the IR changes.
 */
class PassManager {
  ir::Module* irModule;                    ///< The IR module to run passes on
  pass::TopAnalysisInfoManager* tAIM;      ///< Analysis info manager

public:
  /**
   * @brief Constructs a new PassManager
   * @param pm The IR module to run passes on
   * @param tp The analysis info manager
   */
  PassManager(ir::Module* pm, TopAnalysisInfoManager* tp) {
    irModule = pm;
    tAIM = tp;
  }
  
  /**
   * @brief Runs a module pass
   * @param mp The module pass to run
   */
  void run(ModulePass* mp);
  
  /**
   * @brief Runs a function pass on all functions
   * @param fp The function pass to run
   */
  void run(FunctionPass* fp);
  
  /**
   * @brief Runs a basic block pass on all basic blocks
   * @param bp The basic block pass to run
   */
  void run(BasicBlockPass* bp);
  
  /**
   * @brief Runs a sequence of passes by name
   * @param passes Vector of pass names to run in order
   */
  void runPasses(std::vector<std::string> passes);
};

/**
 * @brief Manages analysis information for the entire compilation unit
 * 
 * TopAnalysisInfoManager serves as a centralized repository for all analysis
 * results computed by analysis passes. It provides caching and lazy evaluation
 * of analysis results, ensuring that expensive analyses are only computed when
 * needed and are properly invalidated when the IR changes.
 * 
 * The manager organizes analysis information hierarchically:
 * - Module-level analyses (call graph, side effects)
 * - Function-level analyses (dominance, loops, induction variables, etc.)
 * - Basic block-level analyses (future extension point)
 * 
 * Analysis results are computed on-demand and cached until the relevant
 * IR structures are modified.
 * 
 * @see DomTree for dominance analysis
 * @see LoopInfo for loop structure analysis
 * @see CallGraph for interprocedural analysis
 */
class TopAnalysisInfoManager {
private:
  ir::Module* mModule;                     ///< The module being analyzed
  
  // Module-level analysis results
  CallGraph* mCallGraph;                   ///< Call graph analysis
  SideEffectInfo* mSideEffectInfo;         ///< Side effect analysis
  
  // Function-level analysis results (cached per function)
  std::unordered_map<ir::Function*, DomTree*> mDomTree;           ///< Dominance trees
  std::unordered_map<ir::Function*, PDomTree*> mPDomTree;         ///< Post-dominance trees
  std::unordered_map<ir::Function*, LoopInfo*> mLoopInfo;         ///< Loop structure info
  std::unordered_map<ir::Function*, IndVarInfo*> mIndVarInfo;     ///< Induction variable info
  std::unordered_map<ir::Function*, DependenceInfo*> mDepInfo;    ///< Dependence analysis
  std::unordered_map<ir::Function*, ParallelInfo*> mParallelInfo; ///< Parallelization info
private:
  /**
   * @brief Initializes analysis info for a new function
   * 
   * Creates and caches analysis objects for a function that hasn't been
   * analyzed before. This is called lazily when analysis info is requested.
   * 
   * @param func The function to initialize analysis info for
   */
  void addNewFunc(ir::Function* func) {
    auto pnewDomTree = new DomTree(func, this);
    mDomTree[func] = pnewDomTree;
    auto pnewPDomTree = new PDomTree(func, this);
    mPDomTree[func] = pnewPDomTree;
    auto pnewLoopInfo = new LoopInfo(func, this);
    mLoopInfo[func] = pnewLoopInfo;
    auto pnewIndVarInfo = new IndVarInfo(func, this);
    mIndVarInfo[func] = pnewIndVarInfo;
    auto pnewDepInfo = new DependenceInfo(func, this);
    mDepInfo[func] = pnewDepInfo;
    auto pnewParallelInfo = new ParallelInfo(func, this);
    mParallelInfo[func] = pnewParallelInfo;
  }

public:
  /**
   * @brief Constructs a new TopAnalysisInfoManager
   * @param pm The module to manage analysis info for
   */
  TopAnalysisInfoManager(ir::Module* pm = nullptr) : mModule(pm), mCallGraph(nullptr) {}
  
  /**
   * @brief Gets the dominance tree for a function
   * 
   * Returns the dominance tree analysis for the given function, computing it
   * if necessary. The analysis is automatically refreshed to ensure correctness.
   * 
   * @param func The function to get dominance info for
   * @return Pointer to the dominance tree, or nullptr for declarations
   */
  DomTree* getDomTree(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto domctx = mDomTree[func];
    if (domctx == nullptr) {
      addNewFunc(func);
    }
    domctx = mDomTree[func];
    domctx->refresh();
    return domctx;
  }
  /**
   * @brief Gets the post-dominance tree for a function
   * @param func The function to get post-dominance info for
   * @return Pointer to the post-dominance tree, or nullptr for declarations
   */
  PDomTree* getPDomTree(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto domctx = mPDomTree[func];
    if (domctx == nullptr) {
      addNewFunc(func);
    }
    domctx = mPDomTree[func];
    domctx->refresh();
    return domctx;
  }
  
  /**
   * @brief Gets the loop information for a function
   * @param func The function to get loop info for  
   * @return Pointer to the loop info, or nullptr for declarations
   */
  LoopInfo* getLoopInfo(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto lpctx = mLoopInfo[func];
    if (lpctx == nullptr) {
      addNewFunc(func);
    }
    lpctx = mLoopInfo[func];
    lpctx->refresh();
    return lpctx;
  }
  IndVarInfo* getIndVarInfo(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto idvctx = mIndVarInfo[func];
    if (idvctx == nullptr) {
      addNewFunc(func);
    }
    idvctx = mIndVarInfo[func];
    idvctx->setOff();
    idvctx->refresh();
    return idvctx;
  }
  DependenceInfo* getDepInfo(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto dpctx = mDepInfo[func];
    if (dpctx == nullptr) {
      addNewFunc(func);
    }
    dpctx = mDepInfo[func];
    dpctx->setOff();
    dpctx->refresh();
    return dpctx;
  }

  CallGraph* getCallGraph() {
    mCallGraph->refresh();
    return mCallGraph;
  }
  SideEffectInfo* getSideEffectInfo() {
    mSideEffectInfo->setOff();
    mSideEffectInfo->refresh();
    return mSideEffectInfo;
  }

  DomTree* getDomTreeWithoutRefresh(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto domctx = mDomTree[func];
    if (domctx == nullptr) {
      addNewFunc(func);
      domctx->refresh();
    }
    return domctx;
  }
  PDomTree* getPDomTreeWithoutRefresh(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto domctx = mPDomTree[func];
    if (domctx == nullptr) {
      addNewFunc(func);
      domctx->refresh();
    }
    return domctx;
  }
  LoopInfo* getLoopInfoWithoutRefresh(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto lpctx = mLoopInfo[func];
    if (lpctx == nullptr) {
      addNewFunc(func);
      lpctx->refresh();
    }
    return lpctx;
  }
  IndVarInfo* getIndVarInfoWithoutRefresh(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto idvctx = mIndVarInfo[func];
    if (idvctx == nullptr) {
      addNewFunc(func);
      idvctx->refresh();
    }
    return idvctx;
  }
  DependenceInfo* getDepInfoWithoutRefresh(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto dpctx = mDepInfo[func];
    if (dpctx == nullptr) {
      addNewFunc(func);
      dpctx->refresh();
    }
    return dpctx;
  }

  CallGraph* getCallGraphWithoutRefresh() { return mCallGraph; }
  SideEffectInfo* getSideEffectInfoWithoutRefresh() { return mSideEffectInfo; }
  ParallelInfo* getParallelInfo(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto dpctx = mParallelInfo[func];
    if (dpctx == nullptr) {
      addNewFunc(func);
    }
    dpctx = mParallelInfo[func];
    return dpctx;
  }

  /**
   * @brief Initializes the analysis info manager
   */
  void initialize();
  
  /**
   * @brief Invalidates analyses when control flow changes
   * 
   * Called when the control flow graph of a function is modified.
   * Invalidates all analyses that depend on CFG structure.
   * 
   * @param func The function whose CFG changed
   */
  void CFGChange(ir::Function* func) {
    if (func->isOnlyDeclare()) return;
    if (mDomTree.find(func) == mDomTree.cend()) {
      std::cerr << "DomTree not found for function " << func->name() << std::endl;
      return;
    }
    mDomTree[func]->setOff();
    mPDomTree[func]->setOff();
    mLoopInfo[func]->setOff();
    mIndVarInfo[func]->setOff();
  }
  
  /**
   * @brief Invalidates call graph when function calls change
   */
  void CallChange() { mCallGraph->setOff(); }
  
  /**
   * @brief Invalidates induction variable info when it changes
   * @param func The function whose induction variables changed
   */
  void IndVarChange(ir::Function* func) {
    if (func->isOnlyDeclare()) return;
    mIndVarInfo[func]->setOff();
  }
};

}  // namespace pass