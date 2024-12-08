#include "pass/analysis/CFGPrinter.hpp"
using namespace pass;

void CFGPrinter::run(ir::Function* func, TopAnalysisInfoManager* tp){
    using namespace std;
    func->rename();
    cerr<<"In Function"<<func->name()<<":"<<endl;
    for(auto bb:func->blocks()){
        for(auto bbnext:bb->next_blocks()){
            cerr<<bb->name()<<" "<<bbnext->name()<<endl;
        }
    }
} 