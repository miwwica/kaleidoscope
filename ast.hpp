#ifndef __AST_HPP__
#define __AST_HPP__ 1

#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include "llvm/IR/Value.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/LinkAllPasses.h"
#include "llvm/Support/Host.h"

using namespace llvm;

class ExprAST{
public:
    virtual Value* codegen() const = 0;
    virtual ~ExprAST(){}
};

class NumberExprAST : public ExprAST{
public:
    NumberExprAST(double x)
        :Val(x)
    {}
    Value* codegen() const;
private:
    double Val;
};

class VariableExprAST : public ExprAST{
public:
    VariableExprAST(string s)
        :Name(s)
    {}
    Value* codegen() const;
private:
    string Name;
};

class BinaryExprAST : public ExprAST{
public:
    BinaryExprAST(ExprAST *l, ExprAST *r)
        :LHS(l),RHS(r)
    {}
    ~BinaryExprAST();
private:
    BinaryExprAST(const BinaryExprAST&) = delete;
    BinaryExprAST& operator= (const BinaryExprAST&) = delete;
protected:
    ExprAST *LHS, *RHS;    
};

class AddExprAST : public BinaryExprAST{
public:
    AddExprAST(ExprAST *l, ExprAST *r)
        :BinaryExprAST(l,r)
    {}
    Value* codegen() const;
};

class DivExprAST : public BinaryExprAST{
public:
    DivExprAST(ExprAST *l, ExprAST *r)
        :BinaryExprAST(l,r)
    {}
    Value* codegen() const;
};

class MulExprAST : public BinaryExprAST{
public:
    MulExprAST(ExprAST *l, ExprAST *r)
        :BinaryExprAST(l,r)
    {}
    Value* codegen() const;
};

class SubExprAST : public BinaryExprAST{
public:
    SubExprAST(ExprAST *l, ExprAST *r)
        :BinaryExprAST(l,r)
    {}
    Value* codegen() const;
};
class LtExprAST : public BinaryExprAST{
public:
    LtExprAST(ExprAST *l, ExprAST *r)
        :BinaryExprAST(l,r)
    {}
    Value* codegen() const;
};

class GtExprAST : public BinaryExprAST{
public:
    GtExprAST(ExprAST *l, ExprAST *r)
        :BinaryExprAST(l,r)
    {}
    Value* codegen() const;
};

class SeqExprAST : public BinaryExprAST{
public:
    SeqExprAST(ExprAST *l, ExprAST *r)
        :BinaryExprAST(l,r)
    {}
    Value* codegen() const;
};


class CallExprAST : public ExprAST{
public:
    CallExprAST(string s, vector<ExprAST*> v)
        :Callee(s),Args(v)
    {}
    Value* codegen() const;
    ~CallExprAST();    
private:
    CallExprAST(const CallExprAST&) = delete ;
    CallExprAST& operator= (const CallExprAST) = delete;
    string Callee;
    vector<ExprAST*> Args;
};

class IfExprAST : public ExprAST {
public:
    IfExprAST(ExprAST *C,ExprAST *T,ExprAST *E)
        :Cond(C),Then(T),Else(E)
    {}
    ~IfExprAST();
    Value* codegen() const;
private:
    IfExprAST(const IfExprAST&) = delete;
    IfExprAST& operator= (const IfExprAST&) = delete;
    ExprAST *Cond, *Then, *Else; 
};

class ForExprAST : public ExprAST {
public:
    ForExprAST(string V,ExprAST *S,ExprAST *E,ExprAST *St,ExprAST *B)
        :VarName(V),Start(S),End(E),Step(St),Body(B)
    {}
    ~ForExprAST();
    Value* codegen() const;
private:
    ForExprAST(const ForExprAST&) = delete;
    ForExprAST& operator= (const ForExprAST&) = delete;
    string VarName;
    ExprAST *Start, *End, *Step, *Body; 
};

class AssignExprAST : public ExprAST{
public:
    AssignExprAST(string s, ExprAST *e)
        :Name(s),E(e)
    {}
    Value* codegen() const;
    ~AssignExprAST();
private:
    AssignExprAST(const AssignExprAST&) = delete;
    AssignExprAST& operator= (const AssignExprAST) = delete;
    string Name;
    ExprAST* E;
};

class VarExprAST : public ExprAST{
public:
    VarExprAST(vector<pair<string,ExprAST*>> V, ExprAST *e)
        :VarNames(V),Body(e)
    {}
    Value* codegen() const;
    ~VarExprAST();
private:
    VarExprAST(const VarExprAST&) = delete;
    VarExprAST& operator= (const VarExprAST) = delete;
    vector<pair<string,ExprAST*>> VarNames;
    ExprAST* Body;
};

class PrototypeAST {
public:
    PrototypeAST(string s, vector<string> v)
        :Name(s),Args(v)
    {} 
    Function* codegen() const;
    string getName() const {
        return Name;
    }
private:
    string Name;
    vector<string> Args;
};

class FunctionAST {
public:
    FunctionAST(PrototypeAST *p, ExprAST *e)
        :Proto(p),Body(e)
    {}
    ~FunctionAST();
    Value* codegen() const;
private:
    FunctionAST(const FunctionAST&) = delete;
    FunctionAST& operator=(const FunctionAST&) = delete;
    PrototypeAST *Proto; //ovde ne mora pokazivac
    ExprAST *Body;
};

void InitializeModuleAndPassManager();

AllocaInst* CreateEntryBlockAlloca(Function *f, string s);
#endif
