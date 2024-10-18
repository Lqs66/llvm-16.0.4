#include "PreModelChecking.h"

// GetElementPtrSimplifier
llvm::PreservedAnalyses PreModelChecking::GetElementPtrSimplifier::run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM) {
    llvm::outs() << "simplify GetElementPtr...\n";
    for (llvm::Function& F : M) {
        runOnFunction(F);
    }
    return llvm::PreservedAnalyses::none();
}

void PreModelChecking::GetElementPtrSimplifier::runOnFunction(llvm::Function& F) {
    llvm::LLVMContext& context = F.getContext();
    auto i32Type = llvm::IntegerType::getInt32Ty(context);
    auto zero = llvm::ConstantInt::get(i32Type, 0);
    for (llvm::BasicBlock& BB : F) {
        llvm::BasicBlock::iterator BI = BB.begin();
        llvm::BasicBlock::iterator BE = BB.end();
        while (BI != BE) {
            auto& I = *BI++;
            if (I.getOpcode() == llvm::Instruction::GetElementPtr) {
                llvm::GetElementPtrInst* inst = llvm::dyn_cast<llvm::GetElementPtrInst>(&I);
                llvm::Value* indexList[1] = {inst->getOperand(1)};
                auto prev = llvm::GetElementPtrInst::Create(inst->getSourceElementType (), inst->getOperand(0), llvm::ArrayRef<llvm::Value*>(indexList, 1), "_gep__", inst);
                const std::size_t E = inst->getNumOperands();
                for (std::size_t oper = 2; oper < E; ++oper) {
                    llvm::Value* indexList[2] = {zero, inst->getOperand(oper)};
                    prev = llvm::GetElementPtrInst::Create(prev->getResultElementType (), prev, llvm::ArrayRef<llvm::Value*>(indexList, 2), "_gep__", inst);
                }
                I.replaceAllUsesWith(prev);
                I.eraseFromParent();
            }
        }
    }
}