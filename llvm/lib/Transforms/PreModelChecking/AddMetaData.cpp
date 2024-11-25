#include "PreModelChecking.h"
llvm::PreservedAnalyses PreModelChecking::AddMetaData::run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM) {
    llvm::outs() << "Add MetaData funcID for all functions...\n";
    llvm::outs() << "Add MetaData inCallID for indirect callsites...\n";
    runOnModule(M);
    return llvm::PreservedAnalyses::all();
}
void PreModelChecking::AddMetaData::runOnModule(llvm::Module &M){
    int inCallID = 0;
    int funcID = 0;
    for(auto& F : M){
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
                        // llvm::MDBuilder MDB(M.getContext());
                        llvm::Constant* inCallIDConstant = llvm::ConstantInt::get(llvm::Type::getInt32Ty(M.getContext()), inCallID++);
                        auto *metadataNode = llvm::MDNode::get(M.getContext(), llvm::ConstantAsMetadata::get(inCallIDConstant));
                        callInst->setMetadata("inCallID", metadataNode);
                    }
                }
            }
        }
    }
}