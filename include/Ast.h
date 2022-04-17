#ifndef __AST_H__
#define __AST_H__

#include <fstream>
#include <string>
#include <vector>
#include "IRBuilder.h"
#include "SymbolTable.h"
#include "Instruction.h"
#include "Operand.h"
#include "Type.h"
using namespace std;
class SymbolEntry;
class Unit;
class Function;
class BasicBlock;
class Instruction;
class IRBuilder;

class Node
{
private:
    static int counter;
    int seq;
public:
    vector<Instruction*> true_list;
    vector<Instruction*> false_list;
    static IRBuilder *builder;
    std::vector<Instruction*> merge(std::vector<Instruction*> &list1, std::vector<Instruction*> &list2);
    void backPatchTrue(vector<Instruction*> &list, BasicBlock*bb);
    void backPatchFalse(vector<Instruction*> &list, BasicBlock*bb);

    Node();
    int getSeq() const {return seq;};
    static void setIRBuilder(IRBuilder*ib) {builder = ib;};
    virtual void output(int level) = 0;
    virtual void typeCheck() = 0;
    virtual void genCode() = 0;
    std::vector<Instruction*>& trueList() {return true_list;}
    std::vector<Instruction*>& falseList() {return false_list;}
};



class ExprNode : public Node
{
public:
    SymbolEntry *symbolEntry;
    Operand *dst;   // The result of the subtree is stored into dst.

    bool isbool;
    ExprNode(SymbolEntry *symbolEntry) : symbolEntry(symbolEntry){};
    Operand* getOperand() {return dst;};
    SymbolEntry* getSymPtr() {return symbolEntry;};
};

class StmtNode : public Node{public:Type *type = nullptr;};

class BinaryExpr : public ExprNode
{
private:
    int op;
    ExprNode *expr1, *expr2;
public:
    enum {ADD, SUB, MUL , DIV, COMOP, AND, OR, LESS, MORE, RELGEQ, RELLEQ, EQUOP, UEQUOP};
    BinaryExpr(SymbolEntry *se, int op, ExprNode *expr1, ExprNode *expr2) : ExprNode(se), op(op), expr1(expr1), expr2(expr2){dst = new Operand(se);};
    void output(int level);
    void typeCheck();
    void genCode();
};

class BinaryExpr0 : public ExprNode//运算
{
private:
    int op;
    ExprNode *expr;
public:
    enum {NOT,ADDU,SUBU};
    BinaryExpr0(SymbolEntry *se, int op, ExprNode *expr) : ExprNode(se), op(op), expr(expr){dst = new Operand(se);};
    void output(int level);
    void typeCheck();
    void genCode();
};


class Constant : public ExprNode
{
public:
    Constant(SymbolEntry *se) : ExprNode(se){dst = new Operand(se);};
    void output(int level);
    void typeCheck();
    void genCode();
};


////Id需要在上次实验基础上进行修改符合要求
class Id : public ExprNode
{
public:
    int value;
    Id(SymbolEntry *se) : ExprNode(se){
        if(se->getType()->isFunc()){
            SymbolEntry *se0 = new TemporarySymbolEntry(se->getType()->getRetType(), SymbolTable::getLabel()); 
            dst = new Operand(se0);

        }
        else{
            SymbolEntry *se0 = new TemporarySymbolEntry(se->getType(), SymbolTable::getLabel()); 
            dst = new Operand(se0);
        }
        };
    Id(SymbolEntry *se, int value) : ExprNode(se), value(value){
        if(se->getType()->isFunc()){
            SymbolEntry *se0 = new TemporarySymbolEntry(se->getType()->getRetType(), SymbolTable::getLabel()); 
            dst = new Operand(se0);

        }
        else{
            SymbolEntry *se0 = new TemporarySymbolEntry(se->getType(), SymbolTable::getLabel()); 
            dst = new Operand(se0);

        }
        };
    void output(int level);
    void typeCheck();
    void genCode();
};


class ParaList : public Node
{
public:
    std::vector<Id*> paraList;
    std::vector<std::string> paraNameList;
    std::vector<ExprNode*> paraexp;
    std::vector<bool> haveVal;
    std::vector<Type*> paramsType;
    std::vector<SymbolEntry*> paramsSy;
    std::vector<Operand*>paramsOpe;
    ParaList(std::vector<std::string> paraNameList, std::vector<bool> haveVal, std::vector<Type*> paramsType,std::vector<SymbolEntry*> paramsSy) : paraNameList(paraNameList), haveVal(haveVal), paramsType(paramsType),paramsSy(paramsSy){
         for(unsigned int i = 0;i < paraNameList.size();i++)  { 
            SymbolEntry *se0 = new TemporarySymbolEntry(paramsSy[i]->getType(), SymbolTable::getLabel()); 
            paraList[i]->dst = new Operand(se0);
         }
    }
    ParaList(std::vector<ExprNode*> paraexp) : paraexp(paraexp) {}
    void output(int level);
    void typeCheck();
    void genCode();
};

//函数参数类


class FunctionCall : public ExprNode
{
private:
    ParaList *paraList;
    bool Para;
public:
    FunctionCall(SymbolEntry *se, ParaList *paraList) : ExprNode(se), paraList(paraList){
        SymbolEntry *se0 = new TemporarySymbolEntry(se->getType()->getRetType(), SymbolTable::getLabel()); 
        dst=new Operand(se0);
        Para = true;
        }
    FunctionCall(SymbolEntry *se) : ExprNode(se){
        SymbolEntry *se0 = new TemporarySymbolEntry(se->getType()->getRetType(), SymbolTable::getLabel()); 
        dst = new Operand(se0);
        Para = false;
        }
    void output(int level);
    void typeCheck();
    void genCode();
};

class IDlist : public Node
{
    //存放所有id列表
public:
     std::map<std::string,ExprNode*> idnamelist;
     std::map<Id*,ExprNode*> idlist;

     IDlist(std::map<std::string,ExprNode*> idnamelist) : idnamelist(idnamelist){};
     void output(int level);
     void typeCheck();
     void genCode();
};



class CompoundStmt : public StmtNode
{
private:
    StmtNode *stmt;
public:
    CompoundStmt(StmtNode *stmt) : stmt(stmt) {};
    CompoundStmt(){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class SeqNode : public StmtNode
{
private:
    StmtNode *stmt1, *stmt2;
public:
    SeqNode(StmtNode *stmt1, StmtNode *stmt2) : stmt1(stmt1), stmt2(stmt2){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class DeclStmt : public StmtNode
{
private:
    IDlist *idlist;
public:
    DeclStmt(IDlist *idlist) : idlist(idlist){};
    void output(int level);
    void typeCheck();
    void genCode();
};

//////
class IfStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *thenStmt;
public:
    IfStmt(ExprNode *cond, StmtNode *thenStmt) : cond(cond), thenStmt(thenStmt){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class IfElseStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *thenStmt;
    StmtNode *elseStmt;
public:
    IfElseStmt(ExprNode *cond, StmtNode *thenStmt, StmtNode *elseStmt) : cond(cond), thenStmt(thenStmt), elseStmt(elseStmt) {};
    void output(int level);
    void typeCheck();
    void genCode();
};

/////需要进行修改
class WhileStmt : public StmtNode//while
{
private:
    ExprNode *cond;
    StmtNode *Stmt;
public:
    WhileStmt(ExprNode *cond, StmtNode *Stmt) : cond(cond), Stmt(Stmt){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class BreakStmt : public StmtNode
{
public:
    BreakStmt(){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class ContinueStmt : public StmtNode
{
public:
    ContinueStmt(){};
    void output(int level);
    void typeCheck();
    void genCode();
};
//////



class ReturnStmt : public StmtNode
{
private:
    ExprNode *retValue;
public:
    ReturnStmt(ExprNode*retValue) : retValue(retValue) {};
    ReturnStmt() {retValue = nullptr;};
    void output(int level);
    void typeCheck();
    void genCode();
};

///需要修改
class ExpStmt : public StmtNode//表达式
{
private:
    ExprNode *Value;
public:
    ExpStmt(ExprNode*Value) : Value(Value) {};
    void output(int level);
    void typeCheck();
    void genCode();
};
////




class AssignStmt : public StmtNode
{
private:
    ExprNode *lval;
    ExprNode *expr;
public:
    AssignStmt(ExprNode *lval, ExprNode *expr) : lval(lval), expr(expr) {};
    void output(int level);
    void typeCheck();
    void genCode();
};

////函数定义部分
class FunctionDef : public StmtNode
{
private:
    SymbolEntry *se;
    StmtNode *stmt;
    ParaList *paramsList;
    bool Para;
public:
    FunctionDef(SymbolEntry *se, StmtNode *stmt) : se(se), stmt(stmt){Para = false;};
    FunctionDef(SymbolEntry *se, StmtNode *stmt, ParaList *paramsList) : se(se), stmt(stmt), paramsList(paramsList){Para = true;};
    void output(int level);
    void typeCheck();
    void genCode();
};




class semicolonStmt : public StmtNode//分号
{
public:
    semicolonStmt(){};
    void output(int level);
    void typeCheck();
    void genCode();
};



class Ast
{
private:
    Node* root;
public:
    Ast() {root = nullptr;}
    void setRoot(Node*n) {root = n;}
    void output();
    void typeCheck();
    void genCode(Unit *unit);
};

#endif
