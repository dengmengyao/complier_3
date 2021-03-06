/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_INCLUDE_PARSER_H_INCLUDED
# define YY_YY_INCLUDE_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 10 "src/parser.y" /* yacc.c:1909  */

    #include "Ast.h"
    #include "SymbolTable.h"
    #include "Type.h"

#line 50 "include/parser.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ID = 258,
    INTEGER = 259,
    FOR = 260,
    WHILE = 261,
    BREAK = 262,
    CONTINUE = 263,
    IF = 264,
    ELSE = 265,
    NOD = 266,
    INT = 267,
    VOID = 268,
    CONST = 269,
    LPAREN = 270,
    RPAREN = 271,
    LBRACE = 272,
    RBRACE = 273,
    SEMICOLON = 274,
    LB = 275,
    RB = 276,
    MUL = 277,
    DIV = 278,
    COMOP = 279,
    ADD = 280,
    SUB = 281,
    AND = 282,
    OR = 283,
    LESS = 284,
    MORE = 285,
    RELGEQ = 286,
    RELLEQ = 287,
    EQUOP = 288,
    UEQUOP = 289,
    ASSIGN = 290,
    ADDU = 291,
    SUBU = 292,
    NOT = 293,
    RETURN = 294,
    THEN = 295
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 16 "src/parser.y" /* yacc.c:1909  */

    int itype;
    char* strtype;
    StmtNode* stmttype;
    ExprNode* exprtype;
    Type* type;
    IDlist* idlisttype;
    ParaList* Parameterlisttype;
    

#line 114 "include/parser.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_INCLUDE_PARSER_H_INCLUDED  */
