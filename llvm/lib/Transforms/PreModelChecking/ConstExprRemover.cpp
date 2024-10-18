#include "PreModelChecking.h"

// ConstExprRemover
llvm::PreservedAnalyses PreModelChecking::ConstExprRemover::run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM) {
    llvm::outs() << "Remove Constant Expressions...\n";
    runOnModule(M);
    return llvm::PreservedAnalyses::none();
}

void PreModelChecking::ConstExprRemover::runOnModule(llvm::Module &M) {
    for (llvm::Function& F : M) {
        for (llvm::BasicBlock& BB : F) {
            for (llvm::Instruction& I : BB) {
                handle(&I);
            }
        }
    }
}

void PreModelChecking::ConstExprRemover::handle(llvm::Instruction* inst) {
    auto ops = inst->getNumOperands();
    for (std::size_t i = 0; i < ops; i++) {
        auto op = inst->getOperand(i);
        llvm::ConstantExpr* oop = nullptr;
        if ((oop = llvm::dyn_cast<llvm::ConstantExpr>(op))) {
            auto ninst = oop->getAsInstruction();
            ninst->insertBefore(inst);
            inst->setOperand(i, ninst);
            handle(ninst);
        }
    }
}