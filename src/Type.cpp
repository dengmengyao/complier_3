#include "Type.h"
#include <sstream>

IntType TypeSystem::commonInt = IntType(32);
IntType TypeSystem::commonBool = IntType(1);
VoidType TypeSystem::commonVoid = VoidType();
ConstType TypeSystem::commonConst = ConstType(32);
FunctionType TypeSystem::commonFunc = FunctionType();


Type* TypeSystem::intType = &commonInt;
Type* TypeSystem::voidType = &commonVoid;
Type* TypeSystem::boolType = &commonBool;
Type* TypeSystem::constType =&commonConst;
Type* TypeSystem::functionType = &commonFunc;

std::string IntType::toStr()
{
    std::ostringstream buffer;
    buffer << "i" << size;
    return buffer.str();
}

std::string VoidType::toStr()
{
    return "void";
}


std::string FunctionType::toStr()
{
    std::ostringstream buffer;
    buffer << returnType->toStr() ;
    return buffer.str();
}

std::string PointerType::toStr()
{
    std::ostringstream buffer;
    buffer << valueType->toStr() << "*";
    return buffer.str();
}

std::string ConstType::toStr()
{
    std::ostringstream buffer;
    buffer << "i" << size;
    return buffer.str();
}
