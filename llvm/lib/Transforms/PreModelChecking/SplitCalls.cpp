#include "PreModelChecking.h"

// splitCalls
llvm::PreservedAnalyses PreModelChecking::splitCalls::run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM) {
    llvm::outs() << "Base call instructions split BB...\n";
    for (llvm::Function& F : M) {
        if (F.getName().str() == "_Z16dump_static_varsv") {
            llvm::outs() << "split skip _Z16dump_static_varsv()...\n";
            continue;
        }
        splitCallSitesInBlock(&F);
    }
    return llvm::PreservedAnalyses::none();
}

void PreModelChecking::splitCalls::splitCallSitesInBlock(llvm::Function* F) {
    // llvm::outs() << "Function: " << F->getName() << "\n";
    std::vector<llvm::BasicBlock *> origBB;
    // Save all basic blocks
    for (llvm::Function::iterator I = F->begin(), IE = F->end(); I != IE; ++I) {
        origBB.push_back(&*I);
    }
    for (std::vector<llvm::BasicBlock *>::iterator I = origBB.begin(), IE = origBB.end(); I != IE; ++I){
        llvm::BasicBlock *curr = *I;
        // 准备切分点数组,切分点应该为 call 指令的位置和下一条指令的位置。如果多个连续的 call 指令，则切分点应该第一个call指令的位置和最后一个call指令的下一条指令的位置。
        std::vector<int> splitPoints;
        int i = 0;
        std::vector<int> callSites; // call 指令的位置
        for(auto& instr : *curr){
            if (auto *CI = llvm::dyn_cast<llvm::CallInst>(&instr)) {
                // llvm::outs() << "CallInst: " << *CI << "\n";
                auto func = CI->getCalledFunction();
                // 只处理非调试指令和非内置函数的调用，内置函数不会被切分
                if (!CI->isDebugOrPseudoInst() && func && !func->isIntrinsic()) {
                    callSites.push_back(i);
                }
            }
            i++;
        }

        if(callSites.size() == 0){
            continue;
        }

        // 处理 callSites 数组，找到连续的 call 指令
        if (callSites.size() == 1){ // 只有一个 call 指令
            splitPoints.push_back(callSites[0]); // 切分点为 call 指令的位置和下一条指令的位置
            splitPoints.push_back(callSites[0]+1);
        }else{ // 多个 call 指令
            // 双指针法找到连续的 call 指令
            int start = callSites[0];
            int end = callSites[0];
            for(int i = 1; i < callSites.size() - 1; i++){
                if(callSites[i] == end + 1){ // 如果下一个位置是连续的，则更新 end
                    end = callSites[i];
                }else{ // 否则，将 start 和 end 作为一个切分点并更新 start 和 end
                    splitPoints.push_back(start);
                    splitPoints.push_back(end+1);
                    start = callSites[i];
                    end = callSites[i];
                }
            }
            if(callSites[callSites.size()-1] == end + 1){ // 单独处理最后一个 call 指令，最后一个 call 指令是连续的，则更新 end 并将 start 和 end 作为一个切分点
                end = callSites[callSites.size()-1];
                splitPoints.push_back(start);
                splitPoints.push_back(end+1);
            }else{ // 否则
                splitPoints.push_back(start);
                splitPoints.push_back(end+1);
                splitPoints.push_back(callSites[callSites.size()-1]);
                splitPoints.push_back(callSites[callSites.size()-1]+1);
            }
        }

        // 根据切分点数组，切分基本块
        auto it = curr->begin();
        llvm::BasicBlock* toSplit = curr;
        int currSize = curr->size();
        int last = 0;
        // llvm::outs() << curr->size() << "\n";
        for(int i = 0; i < splitPoints.size() ; i++){
            for(int j = 0; j < splitPoints[i] - last ; j++){
                it++;
            }
            // llvm::outs() << curr->size() << "\n";
            if(splitPoints[i] == 0 || splitPoints[i] == currSize - 1){
                continue;
            }
            last = splitPoints[i];
            if(toSplit->size() <= 2)
                continue;
            toSplit = toSplit->splitBasicBlock(it, curr->getName() + ".splitcall");
        }
    }    
}