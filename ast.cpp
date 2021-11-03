#include "ast.hpp"

LLVMContext TheContext;
IRBuilder<> Builder(TheContext);
Module* TheModule;
map<string,AllocaInst*> NamedValues;
legacy::FunctionPassManager *TheFPM;

Value* NumberExprAST::codegen() const{
    return ConstantFP::get(TheContext,APFloat(Val));
}

Value* VariableExprAST::codegen() const{
    AllocaInst* V = NamedValues[Name];
    if(!V){
        cerr << "Nepoznata promenljiva" << Name << endl;
        return nullptr;
    }
    return Builder.CreateLoad(V,Name);
}

BinaryExprAST::~BinaryExprAST(){
    delete LHS;
    delete RHS;
}

Value* AddExprAST::codegen() const{
    Value* L = LHS->codegen();
    Value* R = RHS->codegen();
    if(!L || !R) return nullptr;
    return Builder.CreateFAdd(L,R,"addtmp");
}

Value* DivExprAST::codegen() const{
    Value* L = LHS->codegen();
    Value* R = RHS->codegen();
    if(!L || !R) return nullptr;
    return Builder.CreateFDiv(L,R,"divtmp");
}

Value* MulExprAST::codegen() const{
    Value* L = LHS->codegen();
    Value* R = RHS->codegen();
    if(!L || !R) return nullptr;
    return Builder.CreateFMul(L,R,"multmp");
}

Value* SubExprAST::codegen() const{
    Value* L = LHS->codegen();
    Value* R = RHS->codegen();
    if(!L || !R) return nullptr;
    return Builder.CreateFSub(L,R,"subtmp");
}

Value* LtExprAST::codegen() const{
    Value* L = LHS->codegen();
    Value* R = RHS->codegen();
    if(!L || !R) return nullptr;
    return Builder.CreateUIToFP(Builder.CreateFCmpOLT(L,R,"lttmp"),Type::getDoubleTy(TheContext),"booltmp");
}

Value* GtExprAST::codegen() const{
    Value* L = LHS->codegen();
    Value* R = RHS->codegen();
    if(!L || !R) return nullptr;
    return Builder.CreateUIToFP(Builder.CreateFCmpOGT(L,R,"gttmp"),Type::getDoubleTy(TheContext),"booltmp");
}

Value* SeqExprAST::codegen() const{
    Value* L = LHS->codegen();
    Value* R = RHS->codegen();
    if(!L || !R) return nullptr;
    return R;
}

CallExprAST::~CallExprAST(){
    for(vector<ExprAST*>::iterator i = Args.begin(); i != Args.end(); i++){
        delete *i;
    }
}

Value* CallExprAST::codegen() const{
    Function *f = TheModule->getFunction(Callee);
    if(!f) {
        cerr << "poziv nedefinisane fje " << Callee << endl;
        return nullptr;
    }
    if(f->arg_size() != Args.size()){
        cerr << Callee << " je pozvana sa neodgovarajucim brojem argumenata" << endl;
        return nullptr;
    }
    vector<Value*> ArgsV;
    for(int i = 0; i < Args.size(); i++){
        Value *tmp = Args[i]->codegen();
        if(!tmp) return nullptr;
        ArgsV.push_back(tmp);
    }
    return Builder.CreateCall(f,ArgsV,"calltmp");
}
IfExprAST::~IfExprAST(){
    delete Cond;
    delete Then;
    delete Else;
}
/*
Value* IfExprAST::codegen() const{
    Value* CondV = Cond->codegen();
    if(!CondV) return nullptr;
    
    Value* Tmp = Builder.CreateFCmpONE(CondV,ConstantFP::get(TheContext,APFloat(0.0)));
    
    Function *f = Builder.GetInsertBlock()->getParent();
    
    BasicBlock *ThenBB = BasicBlock::Create(TheContext,"then",f);
    BasicBlock *ElseBB = BasicBlock::Create(TheContext,"else",f);
    BasicBlock *MergeBB = BasicBlock::Create(TheContext,"ifcont",f);
    
    Builder.CreateCondBr(Tmp,ThenBB,ElseBB);
    
    Builder.SetInsertPoint(ThenBB);
    Value* ThenV = Then->codegen();
    if(!ThenV) return nullptr;
    Builder.CreateBr(MergeBB);
    ThenBB = Builder.GetInsertBlock();
    
    Builder.SetInsertPoint(ElseBB);
    Value* ElseV = Else->codegen();
    if(!ElseV) return nullptr;
    Builder.CreateBr(MergeBB);
    ElseBB = Builder.GetInsertBlock();
    
    Builder.SetInsertPoint(MergeBB);
    PHINode* PHI = Builder.CreatePHI(Type::getDoubleTy(TheContext),2,"iftmp");
    PHI->addIncoming(ThenV,ThenBB);
    PHI->addIncoming(ElseV,ElseBB);
    return PHI;
}
*/

Value* IfExprAST::codegen() const{
    Value* CondV = Cond->codegen();
    if(!CondV) return nullptr;
    
    Value* Tmp = Builder.CreateFCmpONE(CondV,ConstantFP::get(TheContext,APFloat(0.0)));
    Function* f = Builder.GetInsertBlock()->getParent();    
    BasicBlock *ThenBB = BasicBlock::Create(TheContext,"then",f);
    BasicBlock *ElseBB = BasicBlock::Create(TheContext,"else",f);
    BasicBlock *MergeBB = BasicBlock::Create(TheContext,"merge",f);

    AllocaInst* Alloca = CreateEntryBlockAlloca(f,"alloca");
    Builder.CreateCondBr(Tmp,ThenBB,ElseBB);
    
    Builder.SetInsertPoint(ThenBB);
    Value* ThenV = Then->codegen();
    if(!ThenV) return nullptr;
    Builder.CreateStore(ThenV,Alloca);
    Builder.CreateBr(MergeBB);
    
    Builder.SetInsertPoint(ElseBB);
    Value* ElseV = Else->codegen();
    if(!ElseV) return nullptr;
    Builder.CreateStore(ElseV,Alloca);
    Builder.CreateBr(MergeBB);
    
    Builder.SetInsertPoint(MergeBB);
    return Builder.CreateLoad(Alloca);
}
ForExprAST::~ForExprAST(){
    delete Start;
    delete End;
    delete Step;
    delete Body;
}
/*
Value* ForExprAST::codegen() const{
    Value* StartV = Start->codegen();
    if(!StartV) return nullptr;
    
    Function* f = Builder.GetInsertBlock()->getParent();
    BasicBlock *LoopBB = BasicBlock::Create(TheContext,"loop",f);
    AllocaInst *Alloca = CreateEntryBlockAlloca(f,VarName);
    Builder.CreateStore(StartV,Alloca);
    Builder.CreateBr(LoopBB);
    
    Builder.SetInsertPoint(LoopBB);
    
    
    AllocaInst* OldVal = NamedValues[VarName];
    NamedValues[VarName] = Alloca;
    
        
    Value* EndV = End->codegen();
    if(!EndV) return nullptr;
    
    Value* Tmp = Builder.CreateFCmpONE(EndV,ConstantFP::get(TheContext,APFloat(0.0)),"loopcond");
    BasicBlock* AfterLoopBB = BasicBlock::Create(TheContext,"afterloop");
    BasicBlock* ContinueBB = BasicBlock::Create(TheContext,"continue",f);
    
    Builder.CreateCondBr(Tmp,ContinueBB,AfterLoopBB);
    
    Builder.SetInsertPoint(ContinueBB);
    
    Value* BodyV = Body->codegen();
    if(!BodyV) return nullptr;
    
    Value* StepV = Step->codegen();
    if(!StepV) return nullptr;
    
    Value* CurrVal = Builder.CreateLoad(Alloca,VarName);
    Value* NextVar = Builder.CreateFAdd(CurrVal,StepV,"nextvar");
    Builder.CreateStore(NextVar,Alloca);

    
    Builder.CreateBr(LoopBB);
    f->getBasicBlockList().push_back(AfterLoopBB);
    Builder.SetInsertPoint(AfterLoopBB);
    if(OldVal){
        NamedValues[VarName] = OldVal;
    } else {
        NamedValues.erase(VarName);
    }
    
 
    return ConstantFP::get(TheContext,APFloat(0.0));
    
}
*/
Value* ForExprAST::codegen() const{
    AllocaInst *OldVal = NamedValues[VarName];
    Function *f = Builder.GetInsertBlock()->getParent();
    AllocaInst* Alloca = CreateEntryBlockAlloca(f,VarName);
    NamedValues[VarName] = Alloca;
    Value* StartV = Start->codegen();
    if(!StartV) return nullptr;
    
    Builder.CreateStore(StartV,Alloca);
    BasicBlock *LoopBB = BasicBlock::Create(TheContext,"loop",f);
    BasicBlock *AfterLoopBB = BasicBlock::Create(TheContext,"afterloop",f);
    BasicBlock *ContinueBB = BasicBlock::Create(TheContext,"continue",f);
    
    Builder.CreateBr(LoopBB);
    
    Builder.SetInsertPoint(LoopBB);
    
    Value* EndV = End->codegen();
    if(!EndV) return nullptr;
    
    Value* Tmp = Builder.CreateFCmpONE(EndV,ConstantFP::get(TheContext,APFloat(0.0)));
    Builder.CreateCondBr(Tmp,ContinueBB,AfterLoopBB);
    
    Builder.SetInsertPoint(ContinueBB);
    
    Value* BodyV = Body->codegen();
    if(!BodyV) return nullptr;
    
    Value* StepV = Step->codegen();
    if(!StepV) return nullptr;
    
    Value* tmp = Builder.CreateLoad(Alloca);
    Value* NextValue = Builder.CreateFAdd(tmp,StepV);
    Builder.CreateStore(NextValue,Alloca);
    
    Builder.CreateBr(LoopBB);
    
    Builder.SetInsertPoint(AfterLoopBB);
    
    if(OldVal){
        NamedValues[VarName] = OldVal;
    }else {
        NamedValues.erase(VarName);
    }
    return ConstantFP::get(TheContext,APFloat(0.0));
    
}

Value* AssignExprAST::codegen() const{
    Value* e = E->codegen();
    if(!e) return nullptr;
    
    Builder.CreateStore(e,NamedValues[Name]);
    return e;
}

AssignExprAST::~AssignExprAST(){
    delete E;
}

Value* VarExprAST::codegen() const{
    vector<AllocaInst*> oldAllocas;
    for(int i = 0; i < VarNames.size();i++){
        oldAllocas.push_back(NamedValues[VarNames[i].first]);
    }
    Function *f = Builder.GetInsertBlock()->getParent();
    for(int i = 0; i < VarNames.size();i++){
        AllocaInst* Alloca = CreateEntryBlockAlloca(f,VarNames[i].first);
        NamedValues[VarNames[i].first] = Alloca;
        Value* tmp = VarNames[i].second->codegen();
        if(!tmp) return nullptr;
        Builder.CreateStore(tmp,Alloca);
    }
    
    Value* b = Body->codegen();
    if(!b) return nullptr;
    
    for(int i = 0; i < oldAllocas.size();i++){
        if(oldAllocas[i]){
           NamedValues[VarNames[i].first] = oldAllocas[i]; 
        } else {
            NamedValues.erase(VarNames[i].first);
        }
    }    
    
    return b;
}

VarExprAST::~VarExprAST(){
    for(int i = 0; i < VarNames.size(); i++){
        delete VarNames[i].second;
    }
    delete Body;
}

Function* PrototypeAST::codegen() const{
    vector<Type*> tmp;
    for(int i = 0; i < Args.size(); i++){
        tmp.push_back(Type::getDoubleTy(TheContext));
    }
    FunctionType *FT = FunctionType::get(Type::getDoubleTy(TheContext),tmp, false);
    Function* f = Function::Create(FT,Function::ExternalLinkage, Name, TheModule);
    int i = 0;
    for(auto &a : f->args()){
        a.setName(Args[i++]);
    }
    return f;
}
FunctionAST::~FunctionAST(){
    delete Proto;
    delete Body;
}
Value* FunctionAST::codegen() const{
    Function *f = TheModule->getFunction(Proto->getName());
    if(!f) 
        f = Proto->codegen();
    if(!f) 
        return nullptr;
    if(!f->empty()){
        cerr << Proto->getName() << " ne moze da se redefinise"<<endl;
        return nullptr;
    }
    BasicBlock *BB = BasicBlock::Create(TheContext,"entry",f);
    Builder.SetInsertPoint(BB);
    
    NamedValues.clear();
    for(auto &a : f->args()){
        AllocaInst* Alloca = CreateEntryBlockAlloca(f,string(a.getName()));
        NamedValues[string(a.getName())] = Alloca;
        Builder.CreateStore(&a,Alloca);
    }
    
    Value* tmp = Body->codegen();
    if(tmp != nullptr){
        Builder.CreateRet(tmp);
        
        verifyFunction(*f);
        TheFPM->run(*f);
        return f;
    }
    f->eraseFromParent();
    return nullptr;
    
}

void InitializeModuleAndPassManager(){
    TheModule = new Module("Moj modul", TheContext);
    TheFPM = new legacy::FunctionPassManager(TheModule);
    TheFPM->add(createInstructionCombiningPass());
    TheFPM->add(createReassociatePass());
    TheFPM->add(createGVNPass());
    TheFPM->add(createCFGSimplificationPass());
    TheFPM->add(createPromoteMemoryToRegisterPass());
    
    TheFPM->doInitialization();
}

AllocaInst* CreateEntryBlockAlloca(Function *f, string s){
    IRBuilder<> TmpBuilder(&(f->getEntryBlock()), f->getEntryBlock().begin());
    return TmpBuilder.CreateAlloca(Type::getDoubleTy(TheContext),0,s);
}
