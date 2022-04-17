%code top{
    #include <iostream>
    #include <assert.h>
    #include "parser.h"
    extern Ast ast;
    int yylex();
    int yyerror( char const * );
}

%code requires {
    #include "Ast.h"
    #include "SymbolTable.h"
    #include "Type.h"
}

%union {
    int itype;
    char* strtype;
    StmtNode* stmttype;
    ExprNode* exprtype;
    Type* type;
    IDlist* idlisttype;
    ParaList* Parameterlisttype;
    
}

%start Program
%token <strtype> ID 
%token <itype> INTEGER
%token FOR WHILE  BREAK CONTINUE
%token IF ELSE NOD
%token INT VOID CONST
%token LPAREN RPAREN LBRACE RBRACE SEMICOLON LB RB
%token MUL DIV COMOP
%token ADD SUB AND OR LESS MORE RELGEQ RELLEQ EQUOP UEQUOP  ASSIGN
%token ADDU SUBU NOT
%token RETURN
%nterm <stmttype> Stmts Stmt AssignStmt  BlockStmt IfStmt ExpStmt ReturnStmt DeclStmt FuncDef  WhileStmt BreakStmt ContinueStmt semicolonStmt
%nterm <exprtype> Exp AddExp MulExp LnotExp Cond LOrExp PrimaryExp LVal RelExp LAndExp 
%nterm <type> Type 
%nterm <idlisttype> IDlist  
%nterm <Parameterlisttype> Para CPara
%precedence THEN
%precedence ELSE
%%
Program
    : Stmts {
        ast.setRoot($1);
    }  
    ;
Stmts
    :  
    Stmt {$$=$1;}
    | Stmts Stmt{
        $$ = new SeqNode($1, $2);
    }
    ;
Stmt
    : AssignStmt {$$=$1;}
    | BlockStmt {$$=$1;}
    | IfStmt {$$=$1;}
    | WhileStmt {$$=$1;}
    | BreakStmt {$$=$1;}
    | ContinueStmt {$$=$1;}
    | ExpStmt {$$=$1;}
    | ReturnStmt {$$=$1;}
    | DeclStmt {$$=$1;}
    | FuncDef {$$=$1;}
    | semicolonStmt {$$=$1;}
    ;
IDlist
    :

    IDlist NOD ID {
        IDlist* idlist = $1;
        idlist->idnamelist[$3] = nullptr;
        $$ = new IDlist(idlist->idnamelist);
        delete []$3;
    }
    |
    IDlist NOD ID ASSIGN Exp{
        IDlist* idlist = $1;
        idlist->idnamelist[$3] = $5;
        $$ = new IDlist(idlist->idnamelist);
        delete []$3;
    }
    |
    ID ASSIGN Exp{
        std::map<std::string, ExprNode*> idlist;
        idlist[$1] = $3;
        $$ = new IDlist(idlist);
        delete []$1;
    }
    |
    ID{
        std::map<std::string, ExprNode*> idlist;
        idlist[$1] = nullptr;
        $$ = new IDlist(idlist);
        delete []$1;
    }
    ;

LVal
    : ID {
        SymbolEntry *se;
        se = identifiers->lookup($1);
        if(se == nullptr)
        {
            fprintf(stderr, "\"%s\"未定义\n", (char*)$1);
            delete [](char*)$1;
            assert(se != nullptr);
        }
        $$ = new Id(se);
        delete []$1;
    }
    ;

AssignStmt
    :
    LVal ASSIGN Exp SEMICOLON {
        $$ = new AssignStmt($1, $3);
    }
    ;


BlockStmt
    :   
    LBRACE 
        {identifiers = new SymbolTable(identifiers);} 
        Stmts RBRACE 
        {
            $$ = new CompoundStmt($3);
            SymbolTable *top = identifiers;
            identifiers = identifiers->getPrev();
            delete top;
        }
    |
    LBRACE RBRACE {
        $$ = new CompoundStmt;
    }
    ;
IfStmt
    : IF LPAREN Cond RPAREN Stmt %prec THEN {
        $$ = new IfStmt($3, $5);
    }
    | IF LPAREN Cond RPAREN Stmt ELSE Stmt {
        $$ = new IfElseStmt($3, $5, $7);
    }
    ;
WhileStmt
    :
    WHILE LPAREN Cond RPAREN Stmt {
        $$ = new WhileStmt($3, $5);
    }
    ;
BreakStmt
    :
    BREAK SEMICOLON {
        $$ = new BreakStmt;
    }
    ;
ContinueStmt
    :
    CONTINUE SEMICOLON {
        $$ = new ContinueStmt;
    }
    ;
ReturnStmt
    :
    RETURN Exp SEMICOLON {
        $$ = new ReturnStmt($2);
    }
    |
    RETURN SEMICOLON{
        $$ = new ReturnStmt();
    }
    ;
ExpStmt
    :
    Cond SEMICOLON {
        $$ = new ExpStmt($1);  
    }
    ;
Exp
    :
    LOrExp {$$ = $1;}
    ;
Cond
    :
    LOrExp {$$ = $1;}
    ;

PrimaryExp
    :
    LVal {
        $$ = $1;
    }
    | INTEGER {
        SymbolEntry *se = new ConstantSymbolEntry(TypeSystem::intType, $1);
        $$ = new Constant(se);
    }
    |
    LPAREN Exp RPAREN {
        $$ = $2;
    }
    |
    ID LPAREN CPara RPAREN {

        ParaList *pl = $3;
        SymbolEntry *se = nullptr;
        std::multimap<std::string, SymbolEntry*>::iterator it;
        SymbolTable* curr = identifiers;
        while(curr)
        {
            for(it = curr->symbolTable.begin();it != curr->symbolTable.end();it++)
            {
                Type *t = it->second->getType();
                if(it->first == $1 && t->isFunc())
                {
                    //判断函数参数个数是否相同
                    if(t->getPara().size() != pl->paraexp.size())
                        continue;
                    else
                    {
                        for(unsigned int i = 0;i < pl->paraexp.size();i++)
                        {
                            if(pl->paraexp[i]->getSymPtr()->getType()->isFunc())
                            {
                                if(t->getPara()[i] != pl->paraexp[i]->getSymPtr()->getType()->getRetType())
                                    break;
                            }
                            else
                            if(t->getPara()[i] != pl->paraexp[i]->getSymPtr()->getType())
                                break;
                            
                            se = it->second;
                        }
                    }
                }
            }
            curr = curr->prev;
        }
        
        if(se == nullptr)
        {
            fprintf(stderr, "函数 \"%s\"参数调用错误或重载函数未定义\n", (char*)$1);
            delete [](char*)$1;
            assert(se != nullptr);
        }
        $$ = new FunctionCall(se, $3);

    }
    |
    ID LPAREN RPAREN {

        SymbolEntry *se = nullptr;
        std::multimap<std::string, SymbolEntry*>::iterator it;
        SymbolTable* curr = identifiers;

        while(curr)
        {
            for(it = curr->symbolTable.begin();it != curr->symbolTable.end();it++)
            {
                Type *t = it->second->getType();
                if(it->first == $1 && t->isFunc())
                {
                    //判断两个参数数量是否相等
                    if(t->getPara().size() != 0)
                        continue;

                    se = it->second;
                    break;
                }
            }
            curr = curr->prev;
        }
        if(se == nullptr)
        {
            fprintf(stderr, "函数 \"%s\"参数调用错误或重载函数未定义\n", (char*)$1);
            delete [](char*)$1;
            assert(se != nullptr);
        }
        $$ = new FunctionCall(se);

    }
    ;

LnotExp
    :
    PrimaryExp {$$ = $1;}
    |
    SUB LnotExp %prec SUBU
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr0(se, BinaryExpr0::SUBU, $2);
    }
    |
    ADD LnotExp %prec ADDU
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr0(se, BinaryExpr0::ADDU, $2);
    }
    |
    NOT LnotExp 
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr0(se, BinaryExpr0::NOT, $2);
    }
    ;
MulExp
    :
    LnotExp {$$ = $1;}
    |
    MulExp MUL LnotExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::MUL, $1, $3);
    }
    |
    MulExp DIV LnotExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::DIV, $1, $3);
    }
    |
    MulExp COMOP LnotExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::COMOP, $1, $3);
    }
    ;
AddExp
    :
    MulExp {$$ = $1;}
    |
    AddExp ADD MulExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::ADD, $1, $3);
    }
    |
    AddExp SUB MulExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::SUB, $1, $3);
    }
    ;
RelExp
    :
    AddExp {$$ = $1;}
    |
    RelExp LESS AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::LESS, $1, $3);
    }
    |
    RelExp MORE AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::MORE, $1, $3);
    }
    |
    RelExp RELGEQ AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::RELGEQ, $1, $3);
    }
    |
    RelExp RELLEQ AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::RELLEQ, $1, $3);
    }
    |
    RelExp EQUOP AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::EQUOP, $1, $3);
    }
    |
    RelExp UEQUOP AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::UEQUOP, $1, $3);
    }
    ;
LAndExp
    :
    RelExp {$$ = $1;}
    |
    LAndExp AND RelExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::AND, $1, $3);
    }
    ;
LOrExp
    :
    LAndExp {$$ = $1;}
    |
    LOrExp OR LAndExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::OR, $1, $3);
    }
    ;
Type
    : INT {
        $$ = TypeSystem::intType;
    }
    | VOID {
        $$ = TypeSystem::voidType;
    }
    | CONST INT  {
        $$ = TypeSystem::constType;
    }
    ;
DeclStmt
    :
    Type IDlist SEMICOLON {
        IDlist *idl = $2;
        std::map<std::string, ExprNode*>::iterator it;
        std::map<std::string, ExprNode*>::iterator ite;
        int i=0;
        int j=0;
        for(it = idl->idnamelist.begin();it != idl->idnamelist.end(); it++){
            j=0;
            for(ite = idl->idnamelist.begin();ite!= idl->idnamelist.end(); ite++){
                if((it->first==ite->first)&&(i!=j)){
                    fprintf(stderr, "在作用域 %d 中\"%s\"重定义\n", identifiers->getLevel(),(char*)it->first.data() );
                    assert(0);
                    }
                j+=1;
            }
            i+=1;
        }
           

        for(it = idl->idnamelist.begin();it != idl->idnamelist.end(); it++)
        {
            SymbolEntry *se;
            se = new IdentifierSymbolEntry($1, it->first, identifiers->getLevel());
            Id *id = new Id(se);
            SymbolEntry *check;
            check = identifiers->lookup(it->first, identifiers->getLevel());
            if(check != nullptr)//判断是否重定义
            {
                fprintf(stderr, "在作用域 %d 中\"%s\"重定义\n", identifiers->getLevel(),(char*)it->first.data() );
                assert(check == nullptr);
            }
            identifiers->install(it->first, se);

            if(it->second)//判断是否又表达式
                idl->idlist[id] = it->second;

            else
                idl->idlist[id] = nullptr;
            
        }
        $$ = new DeclStmt($2);
    }
    ; 

Para
    :
    Para NOD Type ID ASSIGN INTEGER {

        SymbolEntry *se;
        se=new IdentifierSymbolEntry($3,$4,identifiers->getLevel());
        identifiers->install($4, se);
       // ((IdentifierSymbolEntry*)se)->setLabel();
        ((IdentifierSymbolEntry*)se)->setAddr(new Operand(se));
        ParaList* para = $1;
        para->paraNameList.push_back($4);
        para->haveVal.push_back(true);
        para->paramsType.push_back($3);
        para->paramsSy.push_back(se);
        $$ = new ParaList(para->paraNameList, para->haveVal, para->paramsType,para->paramsSy);
    }
    |
    Para NOD Type ID {
        SymbolEntry *se;
        se=new IdentifierSymbolEntry($3,$4,identifiers->getLevel());
        identifiers->install($4, se);
        //((IdentifierSymbolEntry*)se)->setLabel();
        ((IdentifierSymbolEntry*)se)->setAddr(new Operand(se));
        ParaList* para = $1;
        para->paraNameList.push_back($4);
        para->haveVal.push_back(false);
        para->paramsType.push_back($3);
        para->paramsSy.push_back(se);
        $$ = new ParaList(para->paraNameList, para->haveVal, para->paramsType,para->paramsSy);
    }
    |
    Type ID ASSIGN INTEGER {
        std::vector<std::string> para;
        std::vector<bool> haveVal;
        std::vector<Type*> paramsType;
        std::vector<SymbolEntry*>paramsSy;
        SymbolEntry *se;
        se=new IdentifierSymbolEntry($1,$2,identifiers->getLevel());
        identifiers->install($2, se);
        //((IdentifierSymbolEntry*)se)->setLabel();
        ((IdentifierSymbolEntry*)se)->setAddr(new Operand(se));
        
        para.push_back($2);
        haveVal.push_back(true);
        paramsType.push_back($1);
        paramsSy.push_back(se);
        $$ = new ParaList(para, haveVal, paramsType,paramsSy);
        delete []$2;
    }
    |
    Type ID {
        std::vector<std::string> para;
        std::vector<bool> haveVal;
        std::vector<Type*> paramsType;
        std::vector<SymbolEntry*>paramsSy;
        SymbolEntry *se;
        se=new IdentifierSymbolEntry($1,$2,identifiers->getLevel());
        identifiers->install($2, se);
       // ((IdentifierSymbolEntry*)se)->setLabel();
        ((IdentifierSymbolEntry*)se)->setAddr(new Operand(se));
        para.push_back($2);
        haveVal.push_back(false);
        paramsType.push_back($1);
        paramsSy.push_back(se);
        $$ = new ParaList(para, haveVal, paramsType,paramsSy);
        delete []$2;
    }
    ;
CPara
    :
    CPara NOD Exp {
        ParaList* cparalist = $1;
        cparalist->paraexp.push_back($3);
        $$ = new ParaList(cparalist->paraexp);        
    }
    |
    Exp {
        std::vector<ExprNode*> cparalist;
        cparalist.push_back($1);
        $$ = new ParaList(cparalist);        
    }
    ;

FuncDef
    :
    Type ID LPAREN {//无参数类型函数定义
        SymbolEntry *se = nullptr;

        SymbolTable *curr = identifiers;
        std::multimap<std::string, SymbolEntry*>::iterator it;

        while(curr)
        {
            //查找是否存在函数定义
            for(it = curr->symbolTable.begin();it != curr->symbolTable.end();it++)
            {
                Type *t = it->second->getType();
                if(it->first == $2 && t->isFunc())
                {
                    if(t->getPara().size() != 0)
                        continue;
                    
                    se = it->second;
                }
            }
            curr = curr->prev;
        }
        if(se != nullptr)
        {
            fprintf(stderr, "函数\"%s\"重定义\n", (char*)$2);
            delete [](char*)$2;
            assert(se == nullptr);
        }
        Type *funcType;
        funcType = new FunctionType($1,{},{});
        se = new IdentifierSymbolEntry(funcType, $2, identifiers->getLevel());
        identifiers->install($2, se);
        identifiers = new SymbolTable(identifiers);
    }
    RPAREN Stmt
    {
        SymbolEntry *se;
        se = identifiers->lookup($2);
        assert(se != nullptr);
        $$ = new FunctionDef(se, $6);
        SymbolTable *top = identifiers;
        identifiers = identifiers->getPrev();
        delete top;
        delete []$2;
    }
    |
    Type ID LPAREN Para 
    {//有参数类型函数定义
        SymbolEntry *se = nullptr;
        ParaList *para = $4;
        SymbolTable *curr = identifiers;
        std::multimap<std::string, SymbolEntry*>::iterator it;
        while(curr)
        {
            //查找是否存在函数定义
            for(it = curr->symbolTable.begin();it != curr->symbolTable.end();it++)
            {
                Type *t = it->second->getType();
                if(it->first == $2 && t->isFunc())
                {
                    if(t->getPara().size() != para->paramsType.size())
                        continue;
                    
                    else
                    {
                        for(unsigned int i = 0;i < t->getPara().size();i++)
                        {
                            if(t->getPara()[i] != para->paramsType[i])
                                break;
                            se = it->second;
                        }
                    }
                }
            }
            curr = curr->prev;
        }
        if(se != nullptr)
        {
            fprintf(stderr, "函数\"%s\"重定义\n", (char*)$2);
            delete [](char*)$2;
            assert(se == nullptr);
        }
        Type *funcType;
        funcType = new FunctionType($1, para->paramsType,para->paramsSy);
        se = new IdentifierSymbolEntry(funcType, $2, identifiers->getLevel());
        identifiers->install($2, se);
        identifiers = new SymbolTable(identifiers);
        for(unsigned int i = 0;i < para->paraNameList.size();i++)
        {
            SymbolEntry *se;

            se = new IdentifierSymbolEntry(para->paramsType[i], para->paraNameList[i], identifiers->getLevel());
            Id *id = new Id(se);
            identifiers->install(para->paraNameList[i], se);
            para->paraList.push_back(id);
        }
    }
    RPAREN Stmt
    {    
        SymbolEntry *se;
        se = identifiers->lookup($2);
        assert(se != nullptr);
        $$ = new FunctionDef(se, $7, $4);
        SymbolTable *top = identifiers;
        identifiers = identifiers->getPrev();
        delete top;
        delete []$2;
    }
    ;
semicolonStmt
    :
    SEMICOLON {
        $$=new semicolonStmt;
    }
    ;
%%

int yyerror(char const* message)
{
    std::cerr<<message<<std::endl;
    return -1;
}
