#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include "Operand.h"
#include <vector>
#include <map>
using namespace std;
class BasicBlock;

class Instruction
{
public:
    Instruction(unsigned instType, BasicBlock *insert_bb = nullptr);
    virtual ~Instruction();
    BasicBlock *getParent();
    bool isUncond() const {return instType == UNCOND;};
    bool isCond() const {return instType == COND;};
    void setParent(BasicBlock *);
    void setNext(Instruction *);
    void setPrev(Instruction *);
    Instruction *getNext();
    Instruction *getPrev();
    virtual void output() const = 0;
    unsigned instType;
    unsigned opcode;
    Instruction *prev;
    Instruction *next;
    BasicBlock *parent;
    std::vector<Operand*> operands;
    enum {BINARY, COND, UNCOND, RET, LOAD, STORE, CMP, ALLOCA,CALL,GLOBAL,XOR,ZEXT};
};

// meaningless instruction, used as the head node of the instruction list.
class DummyInstruction : public Instruction
{
public:
    DummyInstruction() : Instruction(-1, nullptr) {};
    void output() const {};
};

class AllocaInstruction : public Instruction
{
public:
    AllocaInstruction(Operand *dst, SymbolEntry *se, BasicBlock *insert_bb = nullptr);
    ~AllocaInstruction();
    void output() const;
private:
    SymbolEntry *se;
};

class GlobalInstruction : public Instruction
{
public:
    GlobalInstruction(Operand *dst, SymbolEntry *se);
    ~GlobalInstruction();
    void output() const;
    SymbolEntry *se;
};

class LoadInstruction : public Instruction//从内存地址中加载值到中间变量
{
public:
    LoadInstruction(Operand *dst, Operand *src_addr, BasicBlock *insert_bb = nullptr);
    ~LoadInstruction();
    void output() const;
};

class StoreInstruction : public Instruction//将值存储到内存地址中
{
public:
    StoreInstruction(Operand *dst_addr, Operand *src, BasicBlock *insert_bb = nullptr);
    ~StoreInstruction();
    void output() const;
};

class BinaryInstruction : public Instruction//二元运算，包含一个目的操作数，两个源操作数
{
public:
    BinaryInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb = nullptr);
    ~BinaryInstruction();
    void output() const;
    enum {ADD, SUB, MUL , DIV, COMOP, AND, OR};
};

class BinaryInstruction0 : public Instruction//一元运算
{
public:
    BinaryInstruction0(unsigned opcode,Operand *dst,Operand *src,BasicBlock *insert_bb=nullptr);
    ~BinaryInstruction0();
    void output() const;
    enum {NOT,ADD,SUB};
};


class CmpInstruction : public Instruction//关系运算指令
{
public:
    CmpInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb = nullptr);
    ~CmpInstruction();
    void output() const;
    enum {E, NE, L, GE, G, LE};
};

// unconditional branch
class UncondBrInstruction : public Instruction//无条件跳转指令
{
public:
    UncondBrInstruction(BasicBlock*, BasicBlock *insert_bb = nullptr);
    void output() const;
    void setBranch(BasicBlock *);
    BasicBlock *getBranch();
protected:
    BasicBlock *branch;
};

// conditional branch
class CondBrInstruction : public Instruction//条件跳转指令
{
public:
    CondBrInstruction(BasicBlock*, BasicBlock*, Operand *, BasicBlock *insert_bb = nullptr);
    ~CondBrInstruction();
    void output() const;
    void setTrueBranch(BasicBlock*);
    BasicBlock* getTrueBranch();
    void setFalseBranch(BasicBlock*);
    BasicBlock* getFalseBranch();
    BasicBlock* true_branch;
    BasicBlock* false_branch;
};

class RetInstruction : public Instruction//返回
{
public:
    RetInstruction(Operand *src, BasicBlock *insert_bb = nullptr);
    ~RetInstruction();
    void output() const;
};

class CallInstruction : public Instruction//函数调用
{
public:
    std::string names;
    vector<Operand*> vo;
    int u;
    CallInstruction(SymbolEntry *symbolentry,Operand *dst,vector<Operand*> vo,BasicBlock *insert_bb = nullptr);
    ~CallInstruction();
    void output() const;
};

class XorInstruction : public Instruction {
   public://非运算指令
    XorInstruction(Operand* dst, Operand* src, BasicBlock* insert_bb = nullptr);
    void output() const;
};

class ZextInstruction : public Instruction {
   public:
    ZextInstruction(Operand* dst,Operand* src,BasicBlock* insert_bb = nullptr);
    void output() const;
};


#endif