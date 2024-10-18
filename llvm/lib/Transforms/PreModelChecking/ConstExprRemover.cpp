#include "PreModelChecking.h"

// ConstExprRemover
llvm::PreservedAnalyses PreModelChecking::ConstExprRemover::run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM) {
    llvm::outs() << "Remove Constant Expressions...\n";
    runOnModule(M);
    return llvm::PreservedAnalyses::none();
}


//
// Function: hasConstantGEP()
//
// Description:
//  This function determines whether the given value is a constant expression
//  that has a constant GEP expression embedded within it.
//
// Inputs:
//  V - The value to check.
//
// Return value:
//  nullptr  - This value is not a constant expression with a constant expression
//          GEP within it.
//  ~nullptr - A pointer to the value casted into a ConstantExpr is returned.
//
llvm::ConstantExpr* PreModelChecking::ConstExprRemover::hasConstantGEP (llvm::Value*  V)
{
    if (llvm::ConstantExpr * CE = llvm::dyn_cast<llvm::ConstantExpr>(V))
    {
        if (CE->getOpcode() == llvm::Instruction::GetElementPtr)
        {
            return CE;
        }
        else
        {
            for (uint32_t index = 0; index < CE->getNumOperands(); ++index)
            {
                if (hasConstantGEP (CE->getOperand(index)))
                    return CE;
            }
        }
    }
    return nullptr;
}

//  This function determines whether the given value is a constant expression
//  that has a constant binary or unary operator expression embedded within it.
llvm::ConstantExpr* PreModelChecking::ConstExprRemover::hasConstantBinaryOrUnaryOp (llvm::Value*  V)
{
    if (llvm::ConstantExpr * CE = llvm::dyn_cast<llvm::ConstantExpr>(V))
    {
        if (llvm::Instruction::isBinaryOp(CE->getOpcode()) || llvm::Instruction::isUnaryOp(CE->getOpcode()))
        {
            return CE;
        }
        else
        {
            for (uint32_t index = 0; index < CE->getNumOperands(); ++index)
            {
                if (hasConstantBinaryOrUnaryOp (CE->getOperand(index)))
                    return CE;
            }
        }
    }
    return nullptr;
}

// Return true if this is a constant Gep or binaryOp or UnaryOp expression.
llvm::ConstantExpr* PreModelChecking::ConstExprRemover::hasConstantExpr (llvm::Value*  V)
{
    if (llvm::ConstantExpr * gep = hasConstantGEP(V))
    {
        return gep;
    }
    else if (llvm::ConstantExpr * buop = hasConstantBinaryOrUnaryOp(V))
    {
        return buop;
    }
    else
    {
        return nullptr;
    }
}

//
// Function: convertExpression()
//
// Description:
//  Convert a constant expression into an instruction.  This routine does *not*
//  perform any recursion, so the resulting instruction may have constant
//  expression operands.
//
llvm::Instruction* PreModelChecking::ConstExprRemover::convertExpression (llvm::ConstantExpr * CE, llvm::Instruction*  InsertPt)
{
    //
    // Convert this constant expression into a regular instruction.
    //
    llvm::Instruction* Result = CE->getAsInstruction();
    Result->insertBefore(InsertPt);
    return Result;
}

void PreModelChecking::ConstExprRemover::runOnModule(llvm::Module &M) 
{
    for (llvm::Module::iterator F = M.begin(), E = M.end(); F != E; ++F)
    {
        // Worklist of values to check for constant GEP expressions
        std::vector<llvm::Instruction* > Worklist;

        //
        // Initialize the worklist by finding all instructions that have one or more
        // operands containing a constant GEP expression.
        //
        for (llvm::Function::iterator BB = (*F).begin(); BB != (*F).end(); ++BB)
        {
            for (llvm::BasicBlock::iterator i = BB->begin(); i != BB->end(); ++i)
            {
                //
                // Scan through the operands of this instruction.  If it is a constant
                // expression GEP, insert an instruction GEP before the instruction.
                //
                llvm::Instruction*  I = &(*i);
                for (uint32_t index = 0; index < I->getNumOperands(); ++index)
                {
                    if (hasConstantExpr(I->getOperand(index)))
                    {
                        Worklist.push_back (I);
                    }
                }
            }
        }

        //
        // While the worklist is not empty, take an item from it, convert the
        // operands into instructions if necessary, and determine if the newly
        // added instructions need to be processed as well.
        //
        while (Worklist.size())
        {
            llvm::Instruction*  I = Worklist.back();
            Worklist.pop_back();

            //
            // Scan through the operands of this instruction and convert each into an
            // instruction.  Note that this works a little differently for phi
            // instructions because the new instruction must be added to the
            // appropriate predecessor block.
            //
            if (llvm::PHINode * PHI = llvm::dyn_cast<llvm::PHINode>(I))
            {
                for (uint32_t index = 0; index < PHI->getNumIncomingValues(); ++index)
                {
                    //
                    // For PHI Nodes, if an operand is a constant expression with a GEP, we
                    // want to insert the new instructions in the predecessor basic block.
                    //
                    // Note: It seems that it's possible for a phi to have the same
                    // incoming basic block listed multiple times; this seems okay as long
                    // the same value is listed for the incoming block.
                    //
                    llvm::Instruction*  InsertPt = PHI->getIncomingBlock(index)->getTerminator();
                    if (llvm::ConstantExpr * CE = hasConstantExpr(PHI->getIncomingValue(index)))
                    {
                        llvm::Instruction*  NewInst = convertExpression (CE, InsertPt);
                        for (uint32_t i2 = index; i2 < PHI->getNumIncomingValues(); ++i2)
                        {
                            if ((PHI->getIncomingBlock (i2)) == PHI->getIncomingBlock (index))
                                PHI->setIncomingValue (i2, NewInst);
                        }
                        Worklist.push_back (NewInst);
                    }
                }
            }
            else
            {
                for (uint32_t index = 0; index < I->getNumOperands(); ++index)
                {
                    //
                    // For other instructions, we want to insert instructions replacing
                    // constant expressions immediately before the instruction using the
                    // constant expression.
                    //
                    if (llvm::ConstantExpr * CE = hasConstantExpr(I->getOperand(index)))
                    {
                        llvm::Instruction*  NewInst = convertExpression (CE, I);
                        I->replaceUsesOfWith (CE, NewInst);
                        Worklist.push_back (NewInst);
                    }
                }
            }
        }

    }
}
