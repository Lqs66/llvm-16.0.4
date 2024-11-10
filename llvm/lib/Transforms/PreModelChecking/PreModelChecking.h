#ifndef PREMODELCHECKING_H
#define PREMODELCHECKING_H


#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/IRBuilder.h>
#include "llvm/Passes/PassBuilder.h"

namespace PreModelChecking{

    // interface pass for pre model checking
    class PreModelCheckingWarpper : public llvm::PassInfoMixin<PreModelCheckingWarpper> {
    public:
        llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM);

        static bool isRequired() { return true; }
    };

    /**
     * @brief Eliminating constant expressions (means an expression that can be fully computed during the static analysis phase of the compiler).
     * 
     * Iterate through the operands of each instruction.
     * For each operand, if it is a constant expression (of type llvm::ConstantExpr ), 
     * it is converted to an instruction and inserted in front of the current instruction, 
     * and then the operands of the current instruction are updated to the newly inserted instruction.
     * 
     * Recursive processing of newly generated instructions.
    */
    struct ConstExprRemover : public llvm::PassInfoMixin<ConstExprRemover> {
        
        llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM);

        llvm::ConstantExpr* hasConstantGEP (llvm::Value*  V);

        llvm::ConstantExpr* hasConstantBinaryOrUnaryOp (llvm::Value*  V);

        llvm::ConstantExpr* hasConstantExpr (llvm::Value*  V);

        llvm::Instruction* convertExpression (llvm::ConstantExpr * CE, llvm::Instruction*  InsertPt);

        void runOnModule(llvm::Module &M);

        static bool isRequired() { return true; }
    };

    /**
     * @brief Remove unused instructions.
     * 
     * For each instruction, 
     * if it is not used by another instruction and satisfies the conditions of the deleteable method, 
     * it is removed from the parent basic block and the changed flag is set to true.
     * 
     * The deleteable method is used to determine whether an instruction can be deleted. In this method, it first checks the opcode of the instruction. 
     * If the instruction is Store, Call, or Unreachable, it cannot be deleted and returns false; 
     * otherwise, if the instruction is not a terminating instruction (i.e., it is not a control-flow related instruction), it can be deleted and returns true.
    */
    struct RemoveUnusedInstructions : public llvm::PassInfoMixin<RemoveUnusedInstructions> {

        llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM);

        void runOnModule(llvm::Module &M);

        bool deleteable(llvm::Instruction* inst);

        static bool isRequired() { return true; }
    };

    /**
     * @brief Simplify the GetElementPtr instruction.
     * 
    */
    struct GetElementPtrSimplifier : public llvm::PassInfoMixin<GetElementPtrSimplifier> {

        llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM);

        void runOnFunction(llvm::Function& F);

        static bool isRequired() { return true; }
    };

    struct splitCalls : public llvm::PassInfoMixin<splitCalls> {
    public:

        llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM);

        void splitCallSitesInBlock(llvm::Function* F);

        static bool isRequired() { return true; }

    };

    struct bbReNamer : public llvm::PassInfoMixin<bbReNamer> {
    public:
    
        int bbCount = 0;

        llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &);

        void runRenamer(llvm::Module* M);

        static bool isRequired() { return true; }
    };

    class AddMetaData : public llvm::PassInfoMixin<AddMetaData>{
    public:
        llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM);
        
        static bool isRequired() { return true; }
        void runOnModule(llvm::Module &M);
    }; 
    
}
#endif //PREMODELCHECKING_H