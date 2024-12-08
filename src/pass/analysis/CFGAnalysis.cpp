#include "pass/analysis/CFGAnalysis.hpp"
using namespace ir;
namespace pass {
void CFGAnalysis::run(Module* ctx, TopAnalysisInfoManager* tp) {
  constexpr bool DebugCFG = false;
  for (auto func : ctx->funcs()) {
    for (auto bb : func->blocks()) {
      bb->clear_block_link();
    }
  }
  for (auto func : ctx->funcs()) {
    for (auto bb : func->blocks()) {
      auto endInst = bb->insts().back();
      if (auto brInst = dyn_cast<BranchInst>(endInst)) {
        if (brInst->is_cond()) {
          auto trueDst = brInst->iftrue();
          auto falseDst = brInst->iffalse();
          bb->block_link(bb, trueDst);
          bb->block_link(bb, falseDst);
        } else {
          auto dst = brInst->dest();
          bb->block_link(bb, dst);
        }
      }
      if (DebugCFG) {
        endInst->print(std::cerr);
        std::cerr << "\n";
      }
    }
  }

  if (DebugCFG) dump(std::cerr, ctx);
}

void CFGAnalysis::dump(std::ostream& out, Module* ctx) {
  for (auto func : ctx->funcs()) {
    out << "function " << func->name() << ": \n";
    for (auto bb : func->blocks()) {
      out << "\tblock " << bb->name() << ": \n";

      out << "\t\tpre: ";
      for (auto pre : bb->pre_blocks()) {
        out << pre->name() << " ";
      }
      out << "\n";

      out << "\t\tsuc: ";
      for (auto suc : bb->next_blocks()) {
        out << suc->name() << " ";
      }
      out << "\n";

      out << "\n";
    }
    out << "==============\n";
  }
}
}  // namespace pass