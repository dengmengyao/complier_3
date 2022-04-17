#ifndef __TYPE_H__
#define __TYPE_H__
#include <vector>
#include <string>
#include "SymbolTable.h"
class Type
{
private:
    int kind;
    std::vector<Type*> paramType;
    Type *returnType;
protected:
    enum {INT, VOID, FUNC, PTR,CONST,BOOL};
public:
    Type(){};
    Type(int kind) : kind(kind) {};
    virtual ~Type() {};
    virtual std::string toStr() {return "type";};
    bool isInt() const {return kind == INT;};
    bool isVoid() const {return kind == VOID;};
    bool isFunc() const {return kind == FUNC;};
    bool isConst() const {return kind == CONST;};
    bool isBool() const {return kind == BOOL;};
    virtual std::vector<Type*> getPara(){return paramType;}
    virtual Type* getRetType() {return nullptr;};
};

class IntType : public Type
{
private:
    int size;
public:
    IntType(int size) : Type(Type::INT), size(size){};
    std::string toStr();
};



class VoidType : public Type
{
public:
    VoidType() : Type(Type::VOID){};
    std::string toStr();
};

class FunctionType : public Type
{
private:
    Type *returnType;
    std::vector<Type*> paramsType;
    std::vector<SymbolEntry*> paramsSe;
public:
    FunctionType(){};
    FunctionType(Type* returnType, std::vector<Type*> paramsType, std::vector<SymbolEntry*> paramsSe) : 
    Type(Type::FUNC), returnType(returnType), paramsType(paramsType),paramsSe(paramsSe){};
    virtual Type* getRetType() {return returnType;};
    virtual std::string toStr();
    std::vector<Type*> getPara(){return paramsType;}
    std::vector<SymbolEntry*> getParamsSe() { return paramsSe; };
};

class ConstType : public Type
{
private:
    int size;
public:
    ConstType(int size) : Type(Type::CONST), size(size){};
    std::string toStr();
};



class PointerType : public Type
{
private:
    Type *valueType;
public:
    PointerType(Type* valueType) : Type(Type::PTR) {this->valueType = valueType;};
    std::string toStr();
};

class TypeSystem
{
private:
    static IntType commonInt;
    static IntType commonBool;
    static VoidType commonVoid;
    static ConstType commonConst;
    static FunctionType commonFunc;
public:
    static Type *intType;
    static Type *voidType;
    static Type *boolType;
    static Type *constType;
    static Type *functionType;
};

#endif
