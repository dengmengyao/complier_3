%option noyywrap
%{
    #define YY_NO_UNPUT
    #define YY_NO_INPUT

    #include "parser.h"
    #include <ostream>
    #include <fstream>
    using namespace std;

    extern FILE *yyin; 
    extern FILE *yyout;
    extern bool dump_tokens;

    void DEBUG_FOR_LAB4(std::string s){
        std::string DEBUG_INFO = "[DEBUG LAB4]: \t" + s + "\n";
        fputs(DEBUG_INFO.c_str(), yyout);
    }
%}

DECIMIAL ([1-9][0-9]*|0)
OCTAL_DECIMIAL (0[1-7][0-7]*|00)
HEX_DECIMAL (0x[1-9,a-f,A-F][0-9,a-f,A-F]*|0X[1-9,a-f,A-F][0-9,a-f,A-F]*|0x0|0X0)
ID [[:alpha:]_][[:alpha:][:digit:]_]*
EOL (\r\n|\n|\r)
WHITE [\t ]
NOTE \/\/.*

commentbegin "/*"
commentelement .|\n
commentend "*/"
%x BLOCKCOMMENT

%%

{NOTE} {}//去掉注释

{commentbegin} {BEGIN BLOCKCOMMENT; }
<BLOCKCOMMENT>{commentelement} {}
<BLOCKCOMMENT>{commentend} {BEGIN INITIAL; }

"int" {
    /*
    * Questions: 
    *   Q1: Why we need to return INT in further labs?
    *   Q2: What is "INT" actually?
    */
    if(dump_tokens)
        DEBUG_FOR_LAB4("INT\tint");
    return INT;
}
"void" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("VOID\tvoid");
    return VOID;
}
"if" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("IF\tif");
    return IF;
};
"else" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("ELSE\telse");
    return ELSE;
};
"return" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("RETURN\treturn");
    return RETURN;
}
"const" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("CONST\tconst");
    return CONST;
}
"for" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("FOR\tfor");
    return FOR;
}
"break" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("BREAK\tbreak");
    return BREAK;
}
"continue" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("CONTINUE\tcontinue");
    return CONTINUE;
}
"while" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("WHILE\twhile");
    return WHILE;
}

"=" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("ASSIGN\t=");
    return ASSIGN;
}
"<" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("LESS\t<");
    return LESS;
}
">" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("MORE\t>");
    return MORE;
}
"+" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("ADD\t+");
    return ADD;
}
"*" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("MUL\tmul");
    return MUL;
}
"-" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("SUB\t-");
    return SUB;
    
}
"%" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("COMOP\t%");
    return COMOP;
    
} 
"/" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("DIV\t/");
    return DIV;
    
} 
"[" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("LB\t[");
        return LB;
    
}
"]" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("RB\t]");
    return RB;
    
}
">=" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("RELGEQ\t>=");
        return RELGEQ;
    
}
"<=" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("RELLEQ\t<=");
    return RELLEQ;
    
}
"==" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("EQUOP\t==");
    return EQUOP;
    
}
"!=" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("UEQUOP\t!=");
    return UEQUOP;
    
}
"&&" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("AND\t&&");
    return AND;
    
}
"||" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("OR\t||");
    return OR;
    
}
"!" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("NOT\t!");
    return NOT;
    
}
";" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("SEMICOLON\t;");
    return SEMICOLON;
}
"(" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("LPAREN\t(");
    return LPAREN;
}
")" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("RPAREN\t)");
    return RPAREN;
}
"{" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("LBRACE\t{");
    return LBRACE;
}
"}" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("RBRACE\t}");
    return RBRACE;
}
"," {
    if(dump_tokens)
        DEBUG_FOR_LAB4("NOD\t}");
    return NOD;
}

{DECIMIAL} {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    yylval.itype = atoi(yytext);
    return INTEGER;
}


{ID} {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    return ID;
}

{OCTAL_DECIMIAL} {//八进制 
    int num=1,sum=0;
    int l=strlen(yytext);
    for(int i=1;i<=l;++i){
        sum=sum+(yytext[l-i]-'0')*num;
        num*=8;
    } 
    const char *u=std::to_string(sum).c_str();
    char *lexeme;
    lexeme = const_cast<char*>(u);
    if(dump_tokens)
        DEBUG_FOR_LAB4(lexeme);
    yylval.itype = atoi(lexeme);
    return INTEGER;
}

{HEX_DECIMAL} {//16进制 
    int num=1,sum=0;
    int l=strlen(yytext);
    for(int i=1;i<=l-2;++i){
        if(yytext[l-i]>='A'&&yytext[l-i]<='F'){
            sum=sum+(yytext[l-i]-55)*num;
            num*=16;
        }
        else if(yytext[l-i]>='a'&&yytext[l-i]<='f'){
            sum=sum+(yytext[l-i]-87)*num;
            num*=16;
        }
        else{
            sum=sum+(yytext[l-i]-'0')*num;
            num*=16;
        }
    } 
    const char *u=std::to_string(sum).c_str();
    char *lexeme;
    lexeme = const_cast<char*>(u);
    if(dump_tokens)
        DEBUG_FOR_LAB4(lexeme);
    yylval.itype = atoi(lexeme);
    return INTEGER;
}

{EOL} yylineno++;
{WHITE}
%%
