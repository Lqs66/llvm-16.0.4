#include "PreModelChecking.h"

// bbReNamer
llvm::PreservedAnalyses PreModelChecking::bbReNamer::run(llvm::Module &M, llvm::ModuleAnalysisManager &){
    llvm::outs() << "Renaming basic blocks...\n";
    runRenamer(&M);
    return llvm::PreservedAnalyses::none();
}

void PreModelChecking::bbReNamer::runRenamer(llvm::Module* M){
    for(auto& F : *M){
        for(auto& BB : F){
            if (F.getName().startswith("dummyAllocSTy.")) {
                continue;
            }
            BB.setName("bbNum" + std::to_string(bbCount++));
        }
    }
}