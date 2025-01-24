#include "PreModelChecking.h"
#include <fstream>
llvm::PreservedAnalyses PreModelChecking::CreateFuncSections::run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM) {
    llvm::outs() << "Create single section for all functions...\n";
    runOnModule(M);
    return llvm::PreservedAnalyses::all();
}

void PreModelChecking::CreateFuncSections::runOnModule(llvm::Module &M){
    // we create a single section for each function '.text.<function_name>'
    // dump all section names into a file 'func_sections.txt'
    std::ofstream sectionFile("func_sections.txt");
    if (!sectionFile.is_open()){
        llvm::errs() << "Failed to open file 'func_sections.txt'\n";
        return;
    }
    for(auto& F : M){
        if(F.isDeclaration()){
            continue;
        }
        std::string sectionName = ".text." + F.getName().str();
        F.setSection(sectionName);
        sectionFile << sectionName << "\n";
    }
    sectionFile.close();
}
