#include "PreModelChecking.h"
llvm::PreservedAnalyses PreModelChecking::InfoAppender::run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM) {
    runBBRenamer(M);
    addMetaData(M);
    createGlobalVarSection(M);
    return llvm::PreservedAnalyses::all();
}

void PreModelChecking::InfoAppender::runBBRenamer(llvm::Module &M){
    llvm::outs() << "Renaming basic blocks...\n";
    for(auto& F : M){
        for(auto& BB : F){
            if (F.getName().startswith("dummyAllocSTy.")) {
                continue;
            }
            BB.setName("bbNum" + std::to_string(bbID++));
        }
    }
}

void PreModelChecking::InfoAppender::addMetaData(llvm::Module &M){
    llvm::outs() << "Add MetaData funcID for all functions...\n";
    llvm::outs() << "Add MetaData inCallID for indirect callsites...\n";
    for(auto& F : M){
        if (F.getName().startswith("dummyAllocSTy.")) {
            continue;
        }
        llvm::Constant* funcIDConstant = llvm::ConstantInt::get(llvm::Type::getInt32Ty(M.getContext()), funcID++);
        auto *fidMDNode = llvm::MDNode::get(M.getContext(), llvm::ConstantAsMetadata::get(funcIDConstant));
        F.setMetadata("funcID", fidMDNode);
        if(F.isDeclaration()){
            continue;
        }
        for(auto& BB : F){
            for(auto& I : BB){
                if(auto* callInst = llvm::dyn_cast<llvm::CallBase>(&I)){
                    if (callInst->isIndirectCall()) {
                        llvm::Constant* inCallIDConstant = llvm::ConstantInt::get(llvm::Type::getInt32Ty(M.getContext()), inCallID++);
                        auto *metadataNode = llvm::MDNode::get(M.getContext(), llvm::ConstantAsMetadata::get(inCallIDConstant));
                        callInst->setMetadata("inCallID", metadataNode);
                    } 
                }
            }
        }
    }
}

void PreModelChecking::InfoAppender::createGlobalVarSection(llvm::Module &M){
    llvm::outs() << "Move User global vars to global_vars.xx section...\n";
    unsigned globalVarID = 0;
    for (auto &G : M.globals()) {
        if (G.isConstant()) {
            continue;
        }
        if (G.getName().startswith("llvm.")) {
            continue;
        }
        if (G.isDeclaration()) {
            continue;
        }

        llvm::Constant* gvarIDConstant = llvm::ConstantInt::get(llvm::Type::getInt32Ty(M.getContext()), globalVarID++);
        auto *gvarMDNode = llvm::MDNode::get(M.getContext(), llvm::ConstantAsMetadata::get(gvarIDConstant));
        G.setMetadata("globalVarID", gvarMDNode);
        G.setSection("global_vars." + std::to_string(globalVarID - 1));
    }
}