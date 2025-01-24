#include "PreModelChecking.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Scalar.h" // LowerSwitchPass
#include "llvm/Transforms/Scalar/SimplifyCFG.h" // SimplifyCFGPass
#include "llvm/Transforms/IPO/GlobalOpt.h" // GlobalOptPass
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Utils.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"

llvm::PreservedAnalyses PreModelChecking::PreModelCheckingWarpper::run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM){
    llvm::FunctionPassManager FPM;
    llvm::ModulePassManager MPM, MPM2;

    llvm::legacy::PassManager lpm;
    lpm.add(llvm::createLowerSwitchPass());
    lpm.run(M);
 
    MPM.addPass(ConstExprRemover());
    MPM.addPass(RemoveUnusedInstructions());
    MPM.addPass(GetElementPtrSimplifier());
    MPM.run(M, MAM);

    FPM.addPass(llvm::SimplifyCFGPass());
    llvm::FunctionAnalysisManager &FAM = MAM.getResult<llvm::FunctionAnalysisManagerModuleProxy>(M).getManager();
    for(auto& F : M){
        if(F.isDeclaration()){
            continue;
        }
        FPM.run(F, FAM);
    }

    MPM2.addPass(llvm::GlobalOptPass());
    MPM2.addPass(bbReNamer());
    MPM2.addPass(AddMetaData());
    MPM2.addPass(CreateFuncSections());
    MPM2.run(M, MAM);

    return llvm::PreservedAnalyses::none();
}

// register pass
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, 
        "PreModelChecking",      
        "v0.1",                  
        [](llvm::PassBuilder &PB) {   
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::ModulePassManager &MPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                    if (Name == "pre-model-checking") {
                        MPM.addPass(PreModelChecking::PreModelCheckingWarpper());
                        return true;
                    }
                    return false;
                });
        }};
}