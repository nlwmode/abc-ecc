/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_LIB_PHYSLIBERTYYACC_H_INCLUDED
# define YY_LIB_PHYSLIBERTYYACC_H_INCLUDED
/* Debug traces.  */
#ifndef LIB_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define LIB_DEBUG 1
#  else
#   define LIB_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define LIB_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined LIB_DEBUG */
#if LIB_DEBUG
extern int Lib_debug;
#endif

/* Token kinds.  */
#ifndef LIB_TOKENTYPE
# define LIB_TOKENTYPE
  enum Lib_tokentype
  {
    LIB_EMPTY = -2,
    LIB_EOF = 0,                   /* "end of file"  */
    LIB_error = 256,               /* error  */
    LIB_UNDEF = 257,               /* "invalid token"  */
    LIB_TOK_NAME = 258,            /* LIB_TOK_NAME  */
    LIB_TOK_STRING = 259,          /* LIB_TOK_STRING  */
    LIB_TOK_NUMBER = 260,          /* LIB_TOK_NUMBER  */
    LIB_TOK_LIBRARY = 261,         /* LIB_TOK_LIBRARY  */
    LIB_TOK_CELL = 262,            /* LIB_TOK_CELL  */
    LIB_TOK_PIN = 263,             /* LIB_TOK_PIN  */
    LIB_TOK_TIMING = 264,          /* LIB_TOK_TIMING  */
    LIB_TOK_LU_TABLE_TEMPLATE = 265, /* LIB_TOK_LU_TABLE_TEMPLATE  */
    LIB_TOK_CELL_RISE = 266,       /* LIB_TOK_CELL_RISE  */
    LIB_TOK_CELL_FALL = 267,       /* LIB_TOK_CELL_FALL  */
    LIB_TOK_RISE_TRANSITION = 268, /* LIB_TOK_RISE_TRANSITION  */
    LIB_TOK_FALL_TRANSITION = 269, /* LIB_TOK_FALL_TRANSITION  */
    LIB_TOK_SEQUENTIAL = 270,      /* LIB_TOK_SEQUENTIAL  */
    LIB_TOK_INDEX_1 = 271,         /* LIB_TOK_INDEX_1  */
    LIB_TOK_INDEX_2 = 272,         /* LIB_TOK_INDEX_2  */
    LIB_TOK_VALUES = 273           /* LIB_TOK_VALUES  */
  };
  typedef enum Lib_tokentype Lib_token_kind_t;
#endif

/* Value type.  */
#if ! defined LIB_STYPE && ! defined LIB_STYPE_IS_DECLARED
union LIB_STYPE
{
#line 5 "physLibertyYacc.y"

    char * s;
    double d;

#line 95 "physLibertyYacc.h"

};
typedef union LIB_STYPE LIB_STYPE;
# define LIB_STYPE_IS_TRIVIAL 1
# define LIB_STYPE_IS_DECLARED 1
#endif


extern LIB_STYPE Lib_lval;


int Lib_parse (void);


#endif /* !YY_LIB_PHYSLIBERTYYACC_H_INCLUDED  */
