#include "SymbolTable.h"
#include <iostream>
#include <sstream>
#include"Type.h"
SymbolEntry::SymbolEntry(Type *type, int kind) 
{
    this->type = type;
    this->kind = kind;
}

ConstantSymbolEntry::ConstantSymbolEntry(Type *type, int value) : SymbolEntry(type, SymbolEntry::CONSTANT)
{
    this->value = value;
}

std::string ConstantSymbolEntry::toStr()
{
    std::ostringstream buffer;
    buffer << value;
    return buffer.str();
}

IdentifierSymbolEntry::IdentifierSymbolEntry(Type *type, std::string name, int scope) : SymbolEntry(type, SymbolEntry::VARIABLE), name(name)
{
    this->scope = scope;
    addr = nullptr;
}

std::string IdentifierSymbolEntry::toStr()
{
    return "@" + name;
}

TemporarySymbolEntry::TemporarySymbolEntry(Type *type, int label) : SymbolEntry(type, SymbolEntry::TEMPORARY)
{
    this->label = label;
}

std::string TemporarySymbolEntry::toStr()
{
    std::ostringstream buffer;
    buffer << "%t" << label;
    return buffer.str();
}

SymbolTable::SymbolTable()
{
    prev = nullptr;
    level = 0;
}

SymbolTable::SymbolTable(SymbolTable *prev)
{
    this->prev = prev;
    this->level = prev->level + 1;
}

/*
    Description: lookup the symbol entry of an identifier in the symbol table
    Parameters: 
        name: identifier name
    Return: pointer to the symbol entry of the identifier

    hint:
    1. The symbol table is a stack. The top of the stack contains symbol entries in the current scope.
    2. Search the entry in the current symbol table at first.
    3. If it's not in the current table, search it in previous ones(along the 'prev' link).
    4. If you find the entry, return it.
    5. If you can't find it in all symbol tables, return nullptr.
*/
/*
SymbolEntry* SymbolTable::search_func()
{
    SymbolTable *cur=this;

    while(cur->prev!=nullptr)
    {
        //遍历当前作用域符号表
        std::map<std::string,SymbolEntry*>::iterator iter;
        iter=cur->symbolTable.begin();
        while(iter!=cur->symbolTable.end())
        {
            SymbolEntry* se=iter->second;//获取当前符号表项
            if(se->getType()->isFunc()==1)
            {
                return se;
            }
            iter++;
        }
        cur=cur->getPrev();
    }
    //遍历顶部作用域符号表
    std::map<std::string,SymbolEntry*>::iterator iter;
    iter=cur->symbolTable.begin();
    while(iter!=cur->symbolTable.end()){//获取当前符号表
        SymbolEntry* se=iter->second;
        if(se->getType()->isFunc()==1)
        {
            return se;
        }
        iter++;
    }
    return nullptr;
}
*/


SymbolEntry* SymbolTable::lookup(std::string name)
{
    // Todo
    //找现在的表
    SymbolTable* current=this;
    while(current->prev!=nullptr){
        //如果找到了元素，则返回该元素
        if(current->symbolTable.count(name)!=0){
            return current->symbolTable.find(name)->second; 
        }
        else{//未找到
            current=current->getPrev();
        }
    }
    //如果还没有找到，找表最后一个元素
    if(current->symbolTable.count(name)!=0)
        return current->symbolTable.find(name)->second; 
    return nullptr;
}
SymbolEntry* SymbolTable::lookup(std::string name, int scope)
{
    std::multimap<std::string, SymbolEntry*>::iterator it;
    it = this->symbolTable.find(name);
    if(it != this->symbolTable.end())
    {
        return it->second;
    }
    else
    {
        return nullptr;
    }
}

// install the entry into current symbol table.
void SymbolTable::install(std::string name, SymbolEntry* entry)
{
     symbolTable[name] = entry;
}

int SymbolTable::counter = 0;
static SymbolTable t;
static SymbolTable tf;
SymbolTable *identifiers = &t;
SymbolTable *globals = &t;
