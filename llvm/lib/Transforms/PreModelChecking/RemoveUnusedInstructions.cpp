#include "PreModelChecking.h"

// RemoveUnusedInstructions
llvm::PreservedAnalyses PreModelChecking::RemoveUnusedInstructions::run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM) {
    llvm::outs() << "Remove Unused Instructions...\n";
    runOnModule(M);
    return llvm::PreservedAnalyses::none();
}

void PreModelChecking::RemoveUnusedInstructions::runOnModule(llvm::Module &M) {
    bool changed = true;
    do {
        changed = false;
        for (llvm::Function& F : M) {
            for (llvm::BasicBlock& BB : F) {
                for (auto I = BB.begin(), E = BB.end(); I != E; ++I) {
                    llvm::Instruction& inst = *I;
                    if (!inst.getNumUses() &&
                        deleteable(&inst)) {
                        I = inst.eraseFromParent();
                        changed = true;
                    }
                }
            }
        }
    } while (changed);
}

bool PreModelChecking::RemoveUnusedInstructions::deleteable(llvm::Instruction* inst) {
    switch (inst->getOpcode()) {
        case llvm::Instruction::Store:
        case llvm::Instruction::Call:
        case llvm::Instruction::Unreachable:
            return false;
        default:
            return !inst->isTerminator();
    }
}