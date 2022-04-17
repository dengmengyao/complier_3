#include "Function.h"
#include "Unit.h"
#include "Type.h"
#include <list>

extern FILE* yyout;

Function::Function(Unit *u, SymbolEntry *s)
{
    u->insertFunc(this);
    entry = new BasicBlock(this);
    sym_ptr = s;
    parent = u;
}

Function::~Function()
{
    //auto delete_list = block_list;
    //for (auto &i : delete_list)
    //    delete i;
    //parent->removeFunc(this);
}

// remove the basicblock bb from its block_list.
void Function::remove(BasicBlock *bb)
{
    block_list.erase(std::find(block_list.begin(), block_list.end(), bb));
}

void Function::output() const
{
    fprintf(stderr,"i\n ");
    FunctionType* funcType = dynamic_cast<FunctionType*>(sym_ptr->getType());
    Type *retType = funcType->getRetType();
    fprintf(yyout, "define %s %s(", retType->toStr().c_str(), sym_ptr->toStr().c_str());
    std::vector<SymbolEntry*> paramsSe = funcType->getParamsSe();
    std::vector<Operand*>paramsop=op;
    std::string str1="";
    for(unsigned int i=0;i<paramsSe.size();i++){//输出形参类型与地址
          if(i==paramsSe.size()-1){
              str1+=paramsSe[i]->getType()->toStr();
              str1+=" ";
              str1+=op[i]->toStr();
              fprintf(stderr,"%s\n",paramsSe[i]->toStr().c_str());
          }
          else{
              str1+=paramsSe[i]->getType()->toStr();
              str1+=" ";
              str1+=op[i]->toStr();
              str1+=",";
          }
    }
    fprintf(yyout,"%s) {\n",str1.c_str());
    
    std::set<BasicBlock *> v;
    std::list<BasicBlock *> q;
    q.push_back(entry);
    v.insert(entry);
    //int u=q.size();
    //printf("%d ",u);
    while (!q.empty())
    {
        auto bb = q.front();
        q.pop_front();
        if(bb->empty()!=1){
        bb->output();
        for (auto succ = bb->succ_begin(); succ != bb->succ_end(); succ++)
        {
            if (v.find(*succ) == v.end())
            {
                v.insert(*succ);
                q.push_back(*succ);
            }
        }
        //if(q.empty())
         }   //printf("y");
    }
    fprintf(yyout, "}\n");
}
