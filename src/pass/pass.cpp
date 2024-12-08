#include "pass/pass.hpp"

#include "pass/analysis/dom.hpp"
#include "pass/analysis/CFGAnalysis.hpp"
#include "pass/analysis/loop.hpp"
#include "pass/analysis/pdom.hpp"
#include "pass/analysis/irtest.hpp"
#include "pass/analysis/ControlFlowGraph.hpp"
#include "pass/analysis/indvar.hpp"
#include "pass/analysis/CFGPrinter.hpp"
#include "pass/analysis/MarkParallel.hpp"
#include "pass/analysis/sideEffectAnalysis.hpp"
#include "pass/analysis/dependenceAnalysis/DependenceAnalysis.hpp"
#include "pass/analysis/callgraph.hpp"

#include "pass/optimize/mem2reg.hpp"
#include "pass/optimize/DCE.hpp"
#include "pass/optimize/SCP.hpp"
#include "pass/optimize/SCCP.hpp"
#include "pass/optimize/simplifyCFG.hpp"
#include "pass/optimize/GCM.hpp"
#include "pass/optimize/GVN.hpp"
#include "pass/optimize/inline.hpp"
#include "pass/optimize/reg2mem.hpp"
#include "pass/optimize/ADCE.hpp"
#include "pass/optimize/loopsimplify.hpp"

#include "pass/optimize/GlobalToLocal.hpp"
#include "pass/optimize/TCO.hpp"
#include "pass/optimize/InstCombine/ArithmeticReduce.hpp"
#include "pass/optimize/DSE.hpp"
#include "pass/optimize/DLE.hpp"
#include "pass/optimize/SCEV.hpp"
#include "pass/optimize/loopunroll.hpp"
#include "pass/optimize/loopsplit.hpp"
#include "pass/optimize/loopdivest.hpp"
#include "pass/optimize/DeadLoopElimination.hpp"
#include "pass/optimize/DLAE.hpp"
#include "pass/optimize/DAE.hpp"
#include "pass/optimize/licm.hpp"

#include "pass/optimize/GepSplit.hpp"
#include "pass/optimize/AG2L.hpp"
#include "pass/optimize/indvarEndvarRepl.hpp"
#include "pass/optimize/SROA.hpp"

#include "pass/optimize/Misc/StatelessCache.hpp"

#include "pass/optimize/Loop/LoopBodyExtract.hpp"
#include "pass/optimize/Loop/ParallelBodyExtract.hpp"
#include "pass/optimize/Loop/LoopInterChange.hpp"
#include "pass/optimize/Loop/LoopParallel.hpp"
#include "pass/optimize/Misc/BlockSort.hpp"

#include "pass/optimize/DLAE.hpp"

#include "support/config.hpp"
#include "support/FileSystem.hpp"
#include "support/Profiler.hpp"

#include <fstream>
#include <iostream>
#include <cassert>

namespace pass {
const auto& config = sysy::Config::getInstance();

template <typename PassType, typename Callable>
void runPass(PassType* pass, Callable&& runFunc, const std::string& passName) {
  auto start = std::chrono::high_resolution_clock::now();
  runFunc();
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  double threshold = 1e-3;

  if (elapsed.count() > threshold and config.logLevel >= sysy::LogLevel::DEBUG) {
    std::cout << passName << " " << pass->name() << " took " << elapsed.count() << " seconds.\n";
    // auto fileName = utils::preName(config.infile) + "_after_" + passName + ".ll";
  }
}

void PassManager::run(ModulePass* mp) {
  runPass(mp, [&]() { mp->run(irModule, tAIM); }, "ModulePass");
}

void PassManager::run(FunctionPass* fp) {
  runPass(
    fp,
    [&]() {
      for (auto func : irModule->funcs()) {
        if (func->isOnlyDeclare()) continue;
        fp->run(func, tAIM);
      }
    },
    "FunctionPass");
}

void PassManager::run(BasicBlockPass* bp) {
  runPass(
    bp,
    [&]() {
      for (auto func : irModule->funcs()) {
        for (auto bb : func->blocks()) {
          bp->run(bb, tAIM);
        }
      }
    },
    "BasicBlockPass");
}

static Mem2Reg mem2regPass;
static Reg2Mem reg2memPass;

// CFG
static SimplifyCFG simplifyCFGPass;

// DCE
static DCE dcePass;
static ADCE adcePass;
static DAE daePass;
static DeadLoopElimination deadLoopEliminationPass;
static DLAE dlaePass;
static SimpleDSE dsePass;
static SimpleDLE dlePass;

// Scalar
static SCP scpPass;
static SCCP sccpPass;
static GCM gcmPass;
static GVN gvnPass;

static AggressiveG2L AG2LPass;
static Global2Local G2LPass;
static LICM licmPass;
// static Reassociate reassociatePass;
static SCEV scevPass;
static SROA sroaPass;

// IPO
static Inline inlinePass;
static TailCallOpt tcoPass;

// InstCombine
static ArithmeticReduce instCombinePass;

// Loop
static LoopSimplify loopSimplifyPass;
static LoopUnroll loopUnrollPass;
static LoopSplit loopSplitPass;
static LoopDivest loopDivestPass;
static LoopInterChange loopInterChangePass;
static LoopBodyExtract loopBodyExtractPass;
static ParallelBodyExtract parallelBodyExtractPass;
static LoopParallel loopParallelPass;

// Misc
static StatelessCache cachePass;
static BlockSort blockSortPass;
static GepSplit gepSplitPass;
static IdvEdvRepl idvEdvReplPass;

// Analysis
static CFGAnalysisHHW cfgAnalysisPass;
static CFGPrinter cfgPrinterPass;
static CallGraphBuild callGraphBuildPass;
static LoopAnalysis loopAnalysis;

static DomInfoAnalysis domInfoPass;
static PostDomInfoPass postDomInfoPass;
static IndVarAnalysis indVarAnalysis;

static DependenceAnalysis dependenceAnalysisPass;
static MarkParallel markParallelPass;
static SideEffectAnalysis sideEffectAnalysisPass;

static IRCheck irCheckPass;

static std::unordered_map<std::string, BasePass*> passMap = {
  {"mem2reg", &mem2regPass},
  {"reg2mem", &reg2memPass},
  // CFG
  {"simplifycfg", &simplifyCFGPass},

  // DCE
  {"dce", &dcePass},
  {"adce", &adcePass},
  {"dae", &daePass},
  {"DeadLoop", &deadLoopEliminationPass},
  {"dlae", &dlaePass},
  {"dse", &dsePass},
  {"dle", &dlePass},

  // Scalar
  {"scp", &scpPass},
  {"sccp", &sccpPass},
  {"gcm", &gcmPass},
  {"gvn", &gvnPass},
  {"ag2l", &AG2LPass},
  {"g2l", &G2LPass},
  {"licm", &licmPass},
  {"scev", &scevPass},
  {"sroa", &sroaPass},
  // IPO
  {"inline", &inlinePass},
  {"tco", &tcoPass},
  // InstCombine
  {"instcombine", &instCombinePass},
  // Loop
  {"loopsimplify", &loopSimplifyPass},
  {"unroll", &loopUnrollPass},
  {"loopsplit", &loopSplitPass},
  {"loopdivest", &loopDivestPass},
  {"LoopInterChange", &loopInterChangePass},
  {"LoopBodyExtract", &loopBodyExtractPass},
  {"ParallelBodyExtract", &parallelBodyExtractPass},
  {"parallel", &loopParallelPass},

  // Misc
  {"cache", &cachePass},
  {"blocksort", &blockSortPass},
  {"GepSplit", &gepSplitPass},
  {"idvrepl", &idvEdvReplPass},

  // analysis
  {"cfg", &cfgAnalysisPass},
  {"cfgprint", &cfgPrinterPass},
  {"callgraph", &callGraphBuildPass},
  {"loopanalysis", &loopAnalysis},

  {"dom", &domInfoPass},
  {"pdom", &postDomInfoPass},
  {"indvar", &indVarAnalysis},

  {"da", &dependenceAnalysisPass},
  {"markpara", &markParallelPass},
  {"sideeffect", &sideEffectAnalysisPass},
  // check
  {"check", &irCheckPass},
};

std::vector<BasePass*> collectPases(std::vector<std::string> passNames) {
  std::vector<BasePass*> passes;
  passes.push_back(&cfgAnalysisPass);
  for (auto pass_name : passNames) {
    passes.push_back(passMap.at(pass_name));
    passes.push_back(&irCheckPass);
  }
  passes.push_back(&cfgAnalysisPass);
  return std::move(passes);
}

void PassManager::runPasses(std::vector<std::string> passNames) {
  // if(passes.size() == 0) return;
  utils::Stage stage("Optimization Passes"sv);

  if (config.logLevel >= sysy::LogLevel::DEBUG) {
    std::cerr << "Running passes: ";
    for (auto pass_name : passNames) {
      std::cerr << pass_name << " ";
    }
    std::cerr << std::endl;
    auto fileName = utils::preName(config.infile) + "_before_passes.ll";
    config.dumpModule(irModule, fileName);
  }

  //   runPassesBeta(passNames);
  auto passes = collectPases(passNames);
  for (auto pass : passes) {
    // std::cerr << "Running pass: " << pass->name() << std::endl;
    if (auto modulePass = dynamic_cast<ModulePass*>(pass)) {
      run(modulePass);
    } else if (auto functionPass = dynamic_cast<FunctionPass*>(pass)) {
      run(functionPass);
    } else if (auto basicBlockPass = dynamic_cast<BasicBlockPass*>(pass)) {
      run(basicBlockPass);
    } else {
      assert(false && "Invalid pass type");
    }
    if (config.logLevel >= sysy::LogLevel::DEBUG) {
      auto fileName = utils::preName(config.infile) + "_after_" + pass->name() + ".ll";
      config.dumpModule(irModule, fileName);
    }
  }

  if (config.logLevel >= sysy::LogLevel::DEBUG) {
    auto fileName = utils::preName(config.infile) + "_after_passes.ll";
    config.dumpModule(irModule, fileName);
  }

  irModule->rename();
}

}  // namespace pass