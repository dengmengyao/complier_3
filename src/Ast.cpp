#include "Ast.h"
#include "SymbolTable.h"
#include "Unit.h"
#include "Instruction.h"
#include "IRBuilder.h"
#include <string>
#include <vector>
#include "Type.h"

extern FILE *yyout;
int Node::counter = 0;
IRBuilder* Node::builder = nullptr;
bool boolnum = false;//标识表达式是否为bool类型
int t=0;
Operand *dsts;
Operand *dsts1,*dsts2;
int u=0;
int v=0;
int w=0;
int x=0;
int y=0;

Node::Node()
{
    seq = counter++;
}

void Node::backPatchTrue(std::vector<Instruction*> &list, BasicBlock*bb)
{
    for(auto &inst:list)
    {
        if(inst->isCond()){
            dynamic_cast<CondBrInstruction*>(inst)->setTrueBranch(bb);
            BasicBlock *block=  inst->parent;
            block->addPred(bb);
            bb->addSucc(block);
            } 
        else if(inst->isUncond())
            dynamic_cast<UncondBrInstruction*>(inst)->setBranch(bb);
    }
}

void Node::backPatchFalse(vector<Instruction*> &list, BasicBlock*bb)
{
    for(auto &inst:list)
    {
        if(inst->isCond()){
            dynamic_cast<CondBrInstruction*>(inst)->setFalseBranch(bb);
            BasicBlock *block=  inst->parent;
            block->addPred(bb);
            bb->addSucc(block);
            } 
        else {//if(inst->isUncond())
            dynamic_cast<UncondBrInstruction*>(inst)->setBranch(bb);
        }
    }
    false_list.clear();
}

std::vector<Instruction*> Node::merge(std::vector<Instruction*> &list1, std::vector<Instruction*> &list2)
{
    std::vector<Instruction*> res(list1);
    res.insert(res.end(), list2.begin(), list2.end());
    return res;
}

void Ast::genCode(Unit *unit)
{
    fprintf(stderr, "Ast gencode.\n");
    IRBuilder *builder = new IRBuilder(unit);
    Node::setIRBuilder(builder);
    root->genCode();
    //printf("0 ");

    fprintf(yyout,"declare void @putint(i32)\n");
    fprintf(yyout,"declare void @putch(i32)\n");
    fprintf(yyout,"declare i32 @getint() \n");
    fprintf(yyout,"declare i32 @getch()\n");
}

void FunctionDef::genCode()
{
    fprintf(stderr, "Function gencode.\n");
    Unit *unit = builder->getUnit();
    Function *func = new Function(unit, se);
    BasicBlock *entry = func->getEntry();
    // set the insert point to the entry basicblock of this function.
    builder->setInsertBB(entry);
    if(paramsList){
       for(unsigned int i = 0;i < paramsList->paraNameList.size();i++)
       {
          IdentifierSymbolEntry *se = dynamic_cast<IdentifierSymbolEntry *>(paramsList->paraList[i]->getSymPtr());
          Instruction *alloca;
          Operand *addr;
          SymbolEntry *addr_se;
          Type *type;
          type = new PointerType(se->getType());
          addr_se = new TemporarySymbolEntry(type, SymbolTable::getLabel());//新建地址
          addr = new Operand(addr_se);
          alloca = new AllocaInstruction(addr, se); 
          entry->insertFront(alloca);                                 // allocate instructions should be inserted into the begin of the entry block.
          se->setAddr(addr); 
          Operand *ope=paramsList->paraList[i]->dst;
          func->op.push_back(ope);//存储每个形参的地址
          BasicBlock* bb = builder->getInsertBB();
          new StoreInstruction( addr,ope, bb);//存储形参
       }
    }
    //printf("1 ");
    stmt->genCode();

    for(std::vector<BasicBlock*>::iterator ite=func->begin();ite!=func->end();ite++)
    {
        //每个块要遍历所有指令
        //int size=func->getBlockList().size();
        if((*ite)->empty()==1){
            fprintf(stderr,"%d\n",(*ite)->getNo());
        }
        if((*ite)->empty()!=1){
          Instruction * ins = (*ite)->begin();
        
          while(ins!=(*ite)->end())
          {
            if(ins->isCond())
            {
                BasicBlock * trueBB = ((CondBrInstruction*) ins)->getTrueBranch();
                BasicBlock * falseBB = ((CondBrInstruction*) ins)->getFalseBranch();
                if(trueBB->empty()){
                    (*ite)->addSucc(trueBB);
                    trueBB->addPred(*ite);
                }
                if(falseBB->empty()){
                   (*ite)->addSucc(falseBB);
                   falseBB->addPred(*ite);
                }
            }
            else if(ins->isUncond())
            {
                BasicBlock * next = ((UncondBrInstruction*) ins)->getBranch();
                if(next->empty()){
                (*ite)->addSucc(next);
                next->addPred(*ite);
                }
            }
            ins = ins->getNext();
         }
      }
    }
    /**
     * Construct control flow graph. You need do set successors and predecessors for each basic block.
     * Todo
    */
}

void ParaList::genCode()
{
    //fprintf(stderr, "ParaList gencode.\n");

}

void FunctionCall::genCode()
{    
    fprintf(stderr, "FunctionCall gencode.\n");
    //Function *func= builder->getInsertBB()->getParent();
    BasicBlock *nowb = builder->getInsertBB();
    vector<Operand*> vo;
    std::string name;
    if(paraList){
        for(unsigned int i = 0;i < paraList->paraexp.size();i++)
        {
            paraList->paraexp[i]->genCode();
            Operand *src=paraList->paraexp[i]->getOperand();
            vo.push_back(src);

        }
    }
    //Operand *addr;
    //SymbolTable* curr = identifiers;
    //SymbolEntry *addr_se;
    //Type *type=new IntType(32);
    //addr_se = new TemporarySymbolEntry(type, SymbolTable::getLabel());
    //addr = new Operand(dst);
    //name = symbolEntry->toStr();
    //FunctionType* funcType = dynamic_cast<FunctionType*>(func->sym_ptr->getType());
    new CallInstruction(symbolEntry,dst,vo,nowb);
    dsts=dst;
    BasicBlock* bb = builder->getInsertBB();
    //fprintf(stderr, "%d.\n",u);
    if (dsts->getType()->toStr()== IntType(32).toStr()&&u==1) {
            Operand* temp = new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
            new CmpInstruction(CmpInstruction::NE, temp, dsts,
                new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)),
                bb);
        dsts = temp;
     }

}


void BinaryExpr::genCode()
{
    fprintf(stderr, "BinaryExpr gencode.\n");
    BasicBlock *bb = builder->getInsertBB();
    Function *func = bb->getParent();
    if (op == AND)
    {
        y=0;
        //fprintf(stderr, "And gencode.\n");
        u=1;
        Instruction* ins;
        BasicBlock *trueBB = new BasicBlock(func);  // if the result of lhs is true, jump to the trueBB.
        expr1->genCode();
        BasicBlock *now_bb = builder->getInsertBB();
        //Operand *src1=expr1->getOperand();
        ins = new CondBrInstruction(trueBB,nullptr,dsts,now_bb);//根据trueBB状态进行条件跳转
        ins->parent=bb;
        backPatchTrue(expr1->trueList(), trueBB);//指令回填
        expr1->false_list.push_back(ins);
        builder->setInsertBB(trueBB);               // set the insert point to the trueBB so that intructions generated by expr2 will be inserted into it.
        expr2->genCode();
        true_list = expr2->trueList();
        false_list = merge(expr1->falseList(), expr2->falseList());
        v=0;w=0;x=0;u=0;
        
        //printf("2 ");
    }
    else if(op == OR)
    {
        // Todo
        y=0;
        fprintf(stderr, "Or gencode.\n");
        u=1;
        Instruction* ins;
        BasicBlock *falseBB=new BasicBlock(func);
        expr1->genCode();
        BasicBlock *now_bb = builder->getInsertBB();
        //Operand *src1;
        ins = new CondBrInstruction(nullptr,falseBB,dsts,now_bb);//根据falseBB状态进行条件跳转
        ins->parent=bb;
        backPatchFalse(expr1->falseList(),falseBB);//指令回填
        expr1->true_list.push_back(ins);
        builder->setInsertBB(falseBB);
        expr2->genCode();
        true_list=merge(expr1->trueList(),expr2->trueList());
        false_list=expr2->falseList();
        v=0;w=0;x=0;u=0;
        
        //printf("3 ");

    }
    else if(op >= LESS && op <= MORE)//大于小于
    {
        // Todo
        expr1->genCode();
        expr2->genCode();
        Operand *src1=expr1->getOperand();
        Operand *src2=expr2->getOperand();
        //fprintf(stderr, "7 %s   %s.\n",src1->getType()->toStr().c_str(),src1->toStr().c_str());
        if (y==1) {
            //fprintf(stderr, "7%s.\n",src1->toStr());
            Operand* dst3 = new Operand(new TemporarySymbolEntry(
                TypeSystem::intType, SymbolTable::getLabel()));
            new ZextInstruction(dst3, src1, bb);
            src1 = dst3;
        }


        int opcode;
        switch(op)
        {
        case LESS:
           opcode=CmpInstruction::L;
           break;
        case MORE:
           opcode=CmpInstruction::G;
           break;
        }
        new CmpInstruction(opcode,dst,src1,src2,bb);
        dsts=dst;
        v=0;w=0;x=0;
        y=1;
        //printf("4 ");
    }
    else if (op>=RELGEQ && op<=UEQUOP)
    {
        expr1->genCode();
        expr2->genCode();
        Operand *src1=expr1->getOperand();
        Operand *src2=expr2->getOperand();
        fprintf(stderr, "6 %s   %s.\n",src1->getType()->toStr().c_str(),src1->toStr().c_str());
        if (y==1) {
            //fprintf(stderr, "7.\n");
            Operand* dst3 = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
            new ZextInstruction(dst3, src1, bb);
            src1 = dst3;
        }

        int opcode;
        switch(op)
        {
        case RELGEQ:
           opcode=CmpInstruction::GE;
           break;
        case RELLEQ:
           opcode=CmpInstruction::LE;
           break;
        case EQUOP:
           opcode=CmpInstruction::E;
           break;
        case UEQUOP:
           opcode=CmpInstruction::NE;
           break;
        }
        new CmpInstruction(opcode,dst,src1,src2,bb);
        dsts=dst;
        //fprintf(stderr, "9 %s   %s.\n",dst->getType()->toStr().c_str(),src1->toStr().c_str());
        v=0;w=0;x=0;
        y=1;
        //printf("5 ");
    }


    else if(op >= ADD && op <= SUB)
    {
        y=0;
        expr1->genCode();
        Operand *src1,*src2;
        src1=expr1->getOperand();
        expr2->genCode();
        if(w!=0)
            src2=dsts1;
        else
            src2=expr2->getOperand();
        w=0;
        int opcode;
        switch (op)
        {
        case ADD:
            opcode = BinaryInstruction::ADD;
            break;
        case SUB:
            opcode = BinaryInstruction::SUB;
            break;
        }

        new BinaryInstruction(opcode, dst, src1, src2, bb);
        v=0;w=0;x=0;
        
        //printf("6 ");
        //fprintf(stderr, "add gencode.\n");
    }
    else if(op>=MUL&&op<=COMOP)
    {
       
        expr1->genCode();
        expr2->genCode();
        Operand *src1=expr1->getOperand();
        Operand *src2=expr2->getOperand();
        int opcode;
        switch(op)
        {
            case MUL:
               opcode=BinaryInstruction::MUL;
               break;
            case DIV:
               opcode=BinaryInstruction::DIV;
               break;
            case COMOP:
               opcode=BinaryInstruction::COMOP;
               break;
        }
        new BinaryInstruction(opcode,dst,src1,src2,bb);
        v=0;w=0;x=0;
        
        //printf("7 ");
    }
}

void BinaryExpr0::genCode(){//单目运算符
    fprintf(stderr, "BinaryExpr0 gencode.\n");
    
    if(op>=ADDU&&op<=SUBU){
        y=0;
        expr->genCode();
        int opcode;

        switch (op)
        {
        case ADDU:{//跳过加号和对应地址
            //fprintf(stderr, "ADD gencode.\n");
            dsts1=expr->getOperand();
           // fprintf(stderr,"%s\n",dsts1->toStr().c_str());
            w+=1;
            opcode=BinaryInstruction0::ADD;
            break;
            }
        case SUBU:{//负数操作
            y=0;
            //fprintf(stderr, "SUB gencode.\n");
            BasicBlock *bb = builder->getInsertBB();
            //Operand* dst0 = new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
            Operand *src=expr->getOperand();
            if(w==0)
                dsts1=src;
            //fprintf(stderr,"%s\n",src->toStr().c_str());
            opcode=BinaryInstruction0::SUB;
            if(v==0){
                if(w!=0&&x==0){
                   new BinaryInstruction0(opcode,dst,dsts1,bb);}
                else if(w!=0&&x!=0){
                   new BinaryInstruction0(opcode,dst,src,bb);}
                else{
                   new BinaryInstruction0(opcode,dst,dsts1,bb);}
            }
            x+=1;
            break;
        }
        default:
            break;
        }
        //printf("8 ");
    }
    else if(op==NOT){
        fprintf(stderr, "NOT gencode.\n");
        expr->genCode();
        BasicBlock* bb = builder->getInsertBB();
        Operand* src = expr->getOperand();
        fprintf(stderr,"1  %s\n",src->getType()->toStr().c_str());
        
        if(v==0){
            dsts2=src;
            v=1;}
        //fprintf(stderr,"2  %s\n",dsts->getType()->toStr().c_str());
        if (dsts2->getType()->toStr()== IntType(32).toStr()) {
            Operand* temp = new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
            new CmpInstruction(CmpInstruction::NE, temp, dsts2,
                new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)),
                bb);
            dsts2 = temp;
        }
        Operand* dst0 = new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
        false_list=expr->trueList();
        true_list=expr->falseList();
        
        new XorInstruction(dst0, dsts2, bb);
        dsts2=dst0;
        dsts=dsts2;
        //printf("9 ");
    }

}


void Constant::genCode()
{
    // we don't need to generate code.
}

void Id::genCode()
{
    fprintf(stderr, "Id gencode.\n");
    y=0;
    BasicBlock *bb = builder->getInsertBB();

    Operand *addr = dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getAddr();

    new LoadInstruction(dst, addr, bb);
    dsts=dst;
    //fprintf(stderr, "%d.  %s\n",u,dsts->toStr().c_str());
    if (dsts->getType()->toStr()== IntType(32).toStr()&&u==1) {
            Operand* temp = new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
            new CmpInstruction(CmpInstruction::NE, temp, dsts,
                new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)),
                bb);
        dsts = temp;
     }

}

void IfStmt::genCode()
{
    y=0;
    fprintf(stderr, "Ifstmt gencode.\n");
    Function *func;
    BasicBlock *then_bb, *end_bb,*now_bb;

    func = builder->getInsertBB()->getParent();
    then_bb = new BasicBlock(func);
    end_bb = new BasicBlock(func);
    fprintf(stderr, "212. \n");  
    cond->genCode();
    v=0;
    //Operand *src=cond->getOperand();
    backPatchTrue(cond->trueList(), then_bb);
    backPatchFalse(cond->falseList(), end_bb);
    now_bb = builder->getInsertBB();
    new CondBrInstruction(then_bb,end_bb,dsts,now_bb);
    //fprintf(stderr, "2322. \n");  
    builder->setInsertBB(then_bb);
    thenStmt->genCode();
    then_bb = builder->getInsertBB();
    new UncondBrInstruction(end_bb, then_bb);
 
    builder->setInsertBB(end_bb);
    //printf("11 ");
}

void IfElseStmt::genCode()
{
    // Todo
    y=0;
    fprintf(stderr, "IfElse gencode.\n");
    Function *func;
    BasicBlock *then_bb,*else_bb,*end_bb,*now_bb;


    func=builder->getInsertBB()->getParent();
    now_bb = builder->getInsertBB();
    then_bb=new BasicBlock(func);
    else_bb=new BasicBlock(func);
    end_bb = new BasicBlock(func);

    cond->genCode();
    v=0;
 
    //Operand *src=cond->getOperand();
    backPatchTrue(cond->trueList(),then_bb);
    backPatchFalse(cond->falseList(),else_bb);
     
    now_bb = builder->getInsertBB();
    new CondBrInstruction(then_bb,else_bb,dsts,now_bb); 
    if(end_bb->empty())
    builder->setInsertBB(then_bb);
    thenStmt->genCode();
    then_bb=builder->getInsertBB();
    new UncondBrInstruction(end_bb, then_bb);

    builder->setInsertBB(else_bb);
    elseStmt->genCode();
    else_bb=builder->getInsertBB();
    new UncondBrInstruction(end_bb, else_bb);
    builder->setInsertBB(end_bb);

    ///fprintf(stderr, "IfElse end.\n");
   // printf("12 ");

}

void WhileStmt::genCode()//while语句
{
    fprintf(stderr, "While gencode.\n");
    y=0;
    Function *func;
    BasicBlock *then_bb, *end_bb,*now_bb,*judge_bb;


    func = builder->getInsertBB()->getParent();
    now_bb = builder->getInsertBB();
    then_bb = new BasicBlock(func);
    end_bb = new BasicBlock(func);
    judge_bb=new BasicBlock(func);
    new UncondBrInstruction(judge_bb,now_bb);

    builder->setInsertBB(judge_bb);
    cond->genCode();
    v=0;
   // Operand *src=cond->getOperand();
    backPatchTrue(cond->trueList(), then_bb);
    backPatchFalse(cond->falseList(), end_bb);
    
    now_bb = builder->getInsertBB();
    new CondBrInstruction(then_bb,end_bb,dsts,judge_bb);
    builder->setInsertBB(then_bb);
    Stmt->genCode();
    then_bb = builder->getInsertBB();
    new UncondBrInstruction(judge_bb, then_bb);

    builder->setInsertBB(end_bb); 
    //printf("13 ");
}

void BreakStmt::genCode()
{}


void ContinueStmt::genCode()
{}

void CompoundStmt::genCode()
{
    // Todo
    //fprintf(stderr, "Compound gencode.\n");
    if(stmt)
        stmt->genCode();
    //printf("14 ");

}

void SeqNode::genCode()
{
    // Todo
    //fprintf(stderr, "Seqnode gencode.\n");
    stmt1->genCode();
    stmt2->genCode();
    //printf("15 ");
}

void DeclStmt::genCode()
{
    //fprintf(stderr, "Declstmt gencode.\n");
    std::map<Id*, ExprNode*>::iterator it;
    for(it = idlist->idlist.begin();it != idlist->idlist.end();it++){
      IdentifierSymbolEntry *se = dynamic_cast<IdentifierSymbolEntry *>(it->first->getSymPtr());
      if(se->isGlobal())
      {
        //fprintf(stderr, "global.\n");
        Operand *addr;
        SymbolEntry *addr_se;
        addr_se = new IdentifierSymbolEntry(*se);
        addr_se->setType(new PointerType(se->getType()));
        addr = new Operand(addr_se); 
        se->setAddr(addr);


        std::string dst, type;
        dst=addr->toStr();
        type = se->getType()->toStr();
        
        new GlobalInstruction(addr, se); 
        if(it->second){
            BasicBlock *bb = builder->getInsertBB();
            it->second->genCode();
            Operand *src = it->second->getOperand();
            new StoreInstruction(addr, src, bb);
            std::string dst1 = addr->toStr();
            std::string src1 = src->toStr();
            std::string dst_type1 = addr->getType()->toStr();
            std::string src_type1 = src->getType()->toStr();
            fprintf(yyout, "%s = global %s %s, align 4\n", dst.c_str(), type.c_str(),src1.c_str());
            //printf("18 ");
        }
        else{
            fprintf(yyout, "%s = global %s 0, align 4\n", dst.c_str(), type.c_str());
        }



        //printf("16 ");
      }
      else if(se->isLocal())
      {
        Function *func = builder->getInsertBB()->getParent();
        BasicBlock *entry = func->getEntry();
        Instruction *alloca;
        Operand *addr;
        SymbolEntry *addr_se;
        Type *type;
        type = new PointerType(se->getType());
        addr_se = new TemporarySymbolEntry(type, SymbolTable::getLabel());
        addr = new Operand(addr_se);
        alloca = new AllocaInstruction(addr, se);                   // allocate space for local id in function stack.
        if(it->second){
            BasicBlock *bb = builder->getInsertBB();
            it->second->genCode();
            Operand *src = it->second->getOperand();
            new StoreInstruction(addr, src, bb);
            //printf("19 ");
        }
        entry->insertFront(alloca);                                 // allocate instructions should be inserted into the begin of the entry block.
        se->setAddr(addr);                                          // set the addr operand in symbol entry so that we can use it in subsequent code generation.
        //printf("17 ");
      }
    }
}


void IDlist::genCode(){
    
}


/*
void putintStmt::genCode()
{
    expr->genCode();
    printf("20 ");
}

void putchStmt::genCode()
{
     expr->genCode();
     printf("21 ");
}
*/
void ReturnStmt::genCode()
{
    // Todo
    //fprintf(stderr, "return gencode.\n");
    BasicBlock *now_bb;
    now_bb = builder->getInsertBB();
    retValue->genCode();
    Operand *src=retValue->getOperand();
    new RetInstruction(src,now_bb);
    //printf("22 ");
}

void ExpStmt::genCode()
{
    //fprintf(stderr, "Exp gencode.\n");
    Value->genCode();
    //printf("23 ");
}

void AssignStmt::genCode()
{
    //fprintf(stderr, "Assign gencode.\n");
    BasicBlock *bb = builder->getInsertBB();

    expr->genCode();
    Operand *addr = dynamic_cast<IdentifierSymbolEntry*>(lval->getSymPtr())->getAddr();

    Operand *src = expr->getOperand();

    /***
     * We haven't implemented array yet, the lval can only be ID. So we just store the result of the `expr` to the addr of the id.
     * If you want to implement array, you have to caculate the address first and then store the result into it.
     */
    new StoreInstruction(addr, src, bb);
    //printf("24 ");
}
/*
void AssignintStmt::genCode()//输入检查
{
    lval->genCode();
    printf("25 ");
}

void AssignchStmt::genCode()
{
    lval->genCode();
    printf("26 ");
}
*/


void semicolonStmt::genCode()
{}

void Ast::typeCheck()
{
    //fprintf(stderr, "Ast typecheck.\n");
    if(root != nullptr)
        root->typeCheck();
    //printf("27 ");
}



void BinaryExpr::typeCheck()//双目运算类型检查
{
    // Todo 
    //fprintf(stderr, "BinaryExpr typecheck.\n");
    expr1->typeCheck();
    expr2->typeCheck();
    Type *type1=expr1->getSymPtr()->getType();
    Type *type2=expr2->getSymPtr()->getType();
    if(type1->isFunc())
        type1 = type1->getRetType();
    
    if(type2->isFunc())
        type2 = type2->getRetType();
    
    if(type1!=type2&&type1->toStr() != type2->toStr()){
        fprintf(stderr," %s和%s不匹配",
        type1->toStr().c_str(),type2->toStr().c_str());
        exit(EXIT_FAILURE);
    }
    symbolEntry->setType(type1);

    if(op>=0&&op<=4)
        isbool=false;
    else 
    {
        t=1;
        isbool=true;//比较运算时默认变换类型
        if(op>=5&&op<=6)//进行与或运算时提示类型转换，但不退出报错
        {
            if(!expr1->isbool)
                fprintf(stderr, "%s应该从int转换成bool\n", expr1->getSymPtr()->toStr().c_str());
            if(!expr2->isbool)
                fprintf(stderr, "%s应该从int转换成bool\n", expr2->getSymPtr()->toStr().c_str());
            
        }
    }
    //printf("30 ");
}

void BinaryExpr0::typeCheck()//单目运算类型检查
{
    //fprintf(stderr, "SingleExpr typecheck.\n");
    Type *type=expr->getSymPtr()->getType();
    symbolEntry->setType(type);
    if(op==ADDU||op==SUBU)
        isbool=false;
    else
        isbool=true;
    //printf("31 ");
}

void Constant::typeCheck()//常数不用检查
{
    // Todo
}

void Id::typeCheck()//id类型检查
{
    // Todo
    //fprintf(stderr, "id typecheck.\n");
    if(boolnum==1&&t==1)
    {
        fprintf(stderr, "%s应该从int转换成bool\n",  symbolEntry->toStr().c_str());
        boolnum = false;
    }
    t=0;
    //fprintf(stderr, "Id typecheck.\n");
    //printf("32 ");
}



void IfStmt::typeCheck()
{
    // Todo
    boolnum=true;
    cond->typeCheck();
    boolnum=false;
    thenStmt->typeCheck();
    type = thenStmt->type;
    //printf("33 ");
}

void IfElseStmt::typeCheck()
{
    // Todo
    boolnum = true;
    cond->typeCheck();
    boolnum = false;
    thenStmt->typeCheck();
    elseStmt->typeCheck();
    //printf("34 ");
    if(thenStmt->type != nullptr)
        type = thenStmt->type;
    if(elseStmt->type != nullptr)
        type = elseStmt->type;
}


void WhileStmt::typeCheck(){
    boolnum=true;
    cond->typeCheck();
    boolnum=false;
    Stmt->typeCheck();
    //printf("36 ");
}




void CompoundStmt::typeCheck()
{
    // Todo
   //fprintf(stderr, "CompoundStmt typecheck.\n");
   if(stmt){
       stmt->typeCheck();
       type = stmt->type;
       }
}

void SeqNode::typeCheck()
{
    // Todo
    //fprintf(stderr, "SepNode typecheck.\n");
    stmt1->typeCheck();
    stmt2->typeCheck();
    if(stmt1->type != nullptr)
        type = stmt1->type;
    
    if(stmt2->type != nullptr)
        type = stmt2->type;
    
    //printf("38 ");

}

void DeclStmt::typeCheck()
{
    // Todo
    //fprintf(stderr, "declstmt typecheck\n");
    std::map<Id*,ExprNode*>::iterator ite;
    for(ite = idlist->idlist.begin();ite != idlist->idlist.end();ite++)//对idlist中的每一个id进行类型检查
    {
        ExprNode *ex = ite->second;
        SymbolEntry *se = ite->first->getSymPtr();//检查变量类型是否匹配
        if(ex != nullptr)
        {
            ex->typeCheck();
            if(se->getType()->toStr() != ex->getSymPtr()->getType()->toStr())
            {
                fprintf(stderr, "类型%s和类型%s不匹配\n", se->getType()->toStr().c_str(), ex->getSymPtr()->getType()->toStr().c_str());
                exit(EXIT_FAILURE);
            }
        }

    }
    //printf("39 ");
}

void ParaList::typeCheck()
{   
}

void FunctionCall::typeCheck()
{   
 

}

void IDlist::typeCheck()
{
    
}
/*
void ConstStmt::typeCheck()//常值检查
{
    Type *type1=id->getSymPtr()->getType();
    if(type1->isInt()!=1){
        fprintf(stderr,"type %s mismatch in line xx",
        type1->toStr().c_str());
        exit(EXIT_FAILURE);
    }
    

    SymbolEntry *se=id->getSymPtr();
    std::string name=se->toStr();
    if(identifiers->lookup(name)!=nullptr){
        fprintf(stderr,"%s has been declared",name.c_str());
        exit(EXIT_FAILURE);
    }
    printf("40 ");
}
*/


void BreakStmt::typeCheck()//Break
{
     
}

void ContinueStmt::typeCheck()//Continue
{

}


void ReturnStmt::typeCheck()//返回值检查
{
    // Todo

    //检查作用域
    //检查return是否包含在函数内以及函数内函数返回形式是否正确
    //fprintf(stderr, "returnst typecheck.\n");
    if(retValue)
    {
        retValue->typeCheck();
        type = retValue->getSymPtr()->getType();
    }
    else
        type = new VoidType();
    
}

void ExpStmt::typeCheck()//表达式检查
{
    Value->typeCheck();
    //printf("44 ");
}

///
void AssignStmt::typeCheck()//等于检查
{
    // Todo
    //fprintf(stderr, "AssignStmt typecheck\n");
    Type *type1=lval->getSymPtr()->getType();
    Type *type2=expr->getSymPtr()->getType();

    lval->typeCheck();
    expr->typeCheck();
    if(type1->toStr()!=type2->toStr()){
        fprintf(stderr,"类型%s和%s不匹配\n",
        type1->toStr().c_str(),type2->toStr().c_str());
    }
    //printf("45 ");
}
/*
void AssignintStmt::typeCheck()//输入检查
{
    Type *type1=lval->getSymPtr()->getType();
    lval->typeCheck();
    if(type1->isInt()!=1){
        fprintf(stderr,"type %s mismatch in line xx",
        type1->toStr().c_str());
        exit(EXIT_FAILURE);
    }
    printf("46 ");
}

void AssignchStmt::typeCheck()//输出检查
{
    Type *type1=lval->getSymPtr()->getType();
    lval->typeCheck();
    if(type1->isInt()!=1){
        fprintf(stderr,"type %s mismatch in line xx",
        type1->toStr().c_str());
        exit(EXIT_FAILURE);
    }
    printf("47 ");
}
*/
void semicolonStmt::typeCheck()
{}

void FunctionDef::typeCheck()
{

    // Todo
    //fprintf(stderr, "FunctionDef \"%s\" typecheck.\n", se->toStr().c_str());
    stmt->typeCheck();
    type = se->getType()->getRetType();
    if(stmt->type == nullptr)
    {
        fprintf(stderr, "函数\"%s\"无返回值 \n", se->toStr().c_str());
        exit(EXIT_FAILURE);
    }
    else
    {
        if(type->toStr() != stmt->type->toStr())
        {
            fprintf(stderr, "函数 \"%s\" 返回形式为%s,但是%s不符合\n", se->toStr().c_str(), type->toStr().c_str(), stmt->type->toStr().c_str());
            exit(EXIT_FAILURE);
        }
    }
    //printf("28 ");
}
void BinaryExpr::output(int level)
{
    std::string op_str;
    switch(op)
    {
        case ADD:
            op_str = "add";
            break;
        case SUB:
            op_str = "sub";
            break;
        case MUL:
            op_str = "mul";
            break;
        case DIV:
            op_str = "div";
            break;
        case COMOP:
            op_str = "comop";
            break;
        case AND:
            op_str = "and";
            break;
        case OR:
            op_str = "or";
            break;
        case LESS:
            op_str = "less";
            break;
        case MORE:
            op_str = "more";
            break;
        case RELGEQ:
            op_str = "relgeq";
            break;
        case RELLEQ:
            op_str = "relleq";
            break;
        case EQUOP:
            op_str = "equop";
            break;
        case UEQUOP:
            op_str = "uequop";
            break;
    }
    fprintf(yyout, "%*cBinaryExpr\top: %s\n", level, ' ', op_str.c_str());
    expr1->output(level + 4);
    expr2->output(level + 4);
}


/////加
void BinaryExpr0::output(int level)
{
    std::string op_str;
    switch(op)
    {
        case NOT:
            op_str = "not";
            break;
        case ADDU:
            op_str = "addu";
            break;
        case SUBU:
            op_str = "subu";
            break;

    }
    fprintf(yyout, "%*cBinaryExpr\top: %s\n", level, ' ', op_str.c_str());
    expr->output(level + 4);
}
//////



void Ast::output()
{
    fprintf(stderr,"1\n");
    fprintf(yyout, "program\n");
    if(root != nullptr)
        root->output(4);
}

void Constant::output(int level)
{
    std::string type, value;
    type = symbolEntry->getType()->toStr();
    value = symbolEntry->toStr();
    fprintf(yyout, "%*cIntegerLiteral\tvalue: %s\ttype: %s\n", level, ' ',
            value.c_str(), type.c_str());
}

void Id::output(int level)
{
    std::string name, type;
    int scope;
    name = symbolEntry->toStr();
    type = symbolEntry->getType()->toStr();
    scope = dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getScope();
    fprintf(yyout, "%*cId\tname: %s\tscope: %d\ttype: %s\n", level, ' ',
            name.c_str(), scope, type.c_str());   
}

void IDlist::output(int level)
{
    std::map<Id*, ExprNode*>::iterator it;
    for(it = idlist.begin();it != idlist.end();it++)
    {
        //如果含有表达式
        if(it->second)
        {
            it->first->output(level);
            it->second->output(level);
        }
        else
        {
            it->first->output(level);
        }
    }
}

void CompoundStmt::output(int level)
{
    fprintf(yyout, "%*cCompoundStmt\n", level, ' ');
    stmt->output(level + 4);
}

void SeqNode::output(int level)
{
    stmt1->output(level);
    stmt2->output(level);
}


void DeclStmt::output(int level)
{
    fprintf(yyout, "%*cDeclStmt\n", level, ' ');
    idlist->output(level + 4);
}

/*
加

void putintStmt::output(int level)
{
    fprintf(yyout, "%*cputintStmt\n", level, ' ');
    expr->output(level + 4);
}

void putchStmt::output(int level)
{
    fprintf(yyout, "%*cputchStmt\n", level, ' ');
    expr->output(level + 4);
}
*/


void IfStmt::output(int level)
{
    fprintf(yyout, "%*cIfStmt\n", level, ' ');
    cond->output(level + 4);
    thenStmt->output(level + 4);
}

void IfElseStmt::output(int level)
{
    fprintf(yyout, "%*cIfElseStmt\n", level, ' ');
    cond->output(level + 4);
    thenStmt->output(level + 4);
    elseStmt->output(level + 4);
}

/////加
void WhileStmt::output(int level)
{
    fprintf(yyout, "%*cWhileStmt\n", level, ' ');
    cond->output(level + 4);
    Stmt->output(level + 4);
}

void BreakStmt::output(int level)
{
    fprintf(yyout, "%*cBreakStmt\n", level, ' ');
}

void ContinueStmt::output(int level)
{
    fprintf(yyout, "%*cContinueStmt\n", level, ' ');
}
/////


void ReturnStmt::output(int level)
{
    fprintf(yyout, "%*cReturnStmt\n", level, ' ');
    retValue->output(level + 4);
}

void ExpStmt::output(int level)
{
    fprintf(yyout, "%*cExpStmt\n", level, ' ');
    Value->output(level + 4);
}


void AssignStmt::output(int level)
{
    fprintf(yyout, "%*cAssignStmt\n", level, ' ');
    lval->output(level + 4);
    expr->output(level + 4);
}
/*
void AssignintStmt::output(int level)
{
    fprintf(yyout, "%*cgetintStmt\n", level, ' ');
    lval->output(level + 4);
}

void AssignchStmt::output(int level)
{
    fprintf(yyout, "%*cgetchStmt\n", level, ' ');
    lval->output(level + 4);

}
*/
void FunctionDef::output(int level)
{
    std::string name, type;
    name = se->toStr();
    type = se->getType()->toStr();
    fprintf(yyout, "%*cFunctionDefine function name: %s, type: %s\n", level, ' ', 
            name.c_str(), type.c_str());
    if(Para)
    {
        paramsList->output(level + 4);
    }
    stmt->output(level + 4);
}

void FunctionCall::output(int level)
{
    fprintf(yyout, "%*cFunctionCall:\n", level, ' ');
    std::string name, type;
    name = symbolEntry->toStr();
    type = symbolEntry->getType()->toStr();
    fprintf(yyout, "%*cFunctionCall function name: %s, type: %s\n", level + 4, ' ',
            name.c_str(),type.c_str());
    if(Para)
    {
        fprintf(yyout, "%*cParaList:\n", level + 8, ' ');
        for(unsigned int i = 0;i < paraList->paraexp.size();i++)
        {
            paraList->paraexp[i]->output(level + 12);
        }
    }
}

void ParaList::output(int level)
{
    fprintf(yyout, "%*cParaList:\n", level, ' ');
    for(unsigned int i = 0;i < paraList.size();i++)
    {
        paraList[i]->output(level + 4);
        //如果有值
        if(haveVal[i] == true)
        {
            fprintf(yyout, "%*cvalue: %d\n", level + 4, ' ', paraList[i]->value);
        }
    }
}

void semicolonStmt::output(int level)
{
    fprintf(yyout, "%*csemicolonStmt\n", level, ' ');
}
