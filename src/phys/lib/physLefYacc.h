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

#ifndef YY_LEF_PHYSLEFYACC_H_INCLUDED
# define YY_LEF_PHYSLEFYACC_H_INCLUDED
/* Debug traces.  */
#ifndef LEF_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define LEF_DEBUG 1
#  else
#   define LEF_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define LEF_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined LEF_DEBUG */
#if LEF_DEBUG
extern int Lef_debug;
#endif

/* Token kinds.  */
#ifndef LEF_TOKENTYPE
# define LEF_TOKENTYPE
  enum Lef_tokentype
  {
    LEF_EMPTY = -2,
    LEF_EOF = 0,                   /* "end of file"  */
    LEF_error = 256,               /* error  */
    LEF_UNDEF = 257,               /* "invalid token"  */
    LEF_TOK_NAME = 258,            /* LEF_TOK_NAME  */
    LEF_TOK_STRING = 259,          /* LEF_TOK_STRING  */
    LEF_TOK_NUMBER = 260,          /* LEF_TOK_NUMBER  */
    LEF_TOK_MACRO = 261,           /* LEF_TOK_MACRO  */
    LEF_TOK_LAYER = 262,           /* LEF_TOK_LAYER  */
    LEF_TOK_SITE = 263,            /* LEF_TOK_SITE  */
    LEF_TOK_VIA = 264,             /* LEF_TOK_VIA  */
    LEF_TOK_PIN = 265,             /* LEF_TOK_PIN  */
    LEF_TOK_PORT = 266,            /* LEF_TOK_PORT  */
    LEF_TOK_OBS = 267,             /* LEF_TOK_OBS  */
    LEF_TOK_UNITS = 268,           /* LEF_TOK_UNITS  */
    LEF_TOK_PROPERTYDEFINITIONS = 269, /* LEF_TOK_PROPERTYDEFINITIONS  */
    LEF_TOK_VIARULE = 270,         /* LEF_TOK_VIARULE  */
    LEF_TOK_NONDEFAULTRULE = 271,  /* LEF_TOK_NONDEFAULTRULE  */
    LEF_TOK_ARRAY = 272,           /* LEF_TOK_ARRAY  */
    LEF_TOK_END = 273,             /* LEF_TOK_END  */
    LEF_TOK_CLASS = 274,           /* LEF_TOK_CLASS  */
    LEF_TOK_ORIGIN = 275,          /* LEF_TOK_ORIGIN  */
    LEF_TOK_SIZE = 276,            /* LEF_TOK_SIZE  */
    LEF_TOK_BY = 277,              /* LEF_TOK_BY  */
    LEF_TOK_DIRECTION = 278,       /* LEF_TOK_DIRECTION  */
    LEF_TOK_USE = 279,             /* LEF_TOK_USE  */
    LEF_TOK_RECT = 280,            /* LEF_TOK_RECT  */
    LEF_TOK_MASK = 281,            /* LEF_TOK_MASK  */
    LEF_TOK_WIDTH = 282,           /* LEF_TOK_WIDTH  */
    LEF_TOK_PITCH = 283,           /* LEF_TOK_PITCH  */
    LEF_TOK_RESISTANCE = 284,      /* LEF_TOK_RESISTANCE  */
    LEF_TOK_CAPACITANCE = 285,     /* LEF_TOK_CAPACITANCE  */
    LEF_TOK_EDGECAPACITANCE = 286, /* LEF_TOK_EDGECAPACITANCE  */
    LEF_TOK_TYPE = 287,            /* LEF_TOK_TYPE  */
    LEF_TOK_DATABASE = 288,        /* LEF_TOK_DATABASE  */
    LEF_TOK_MANUFACTURINGGRID = 289 /* LEF_TOK_MANUFACTURINGGRID  */
  };
  typedef enum Lef_tokentype Lef_token_kind_t;
#endif

/* Value type.  */
#if ! defined LEF_STYPE && ! defined LEF_STYPE_IS_DECLARED
union LEF_STYPE
{
#line 5 "physLefYacc.y"

    char * s;
    double d;

#line 111 "physLefYacc.h"

};
typedef union LEF_STYPE LEF_STYPE;
# define LEF_STYPE_IS_TRIVIAL 1
# define LEF_STYPE_IS_DECLARED 1
#endif


extern LEF_STYPE Lef_lval;


int Lef_parse (void);


#endif /* !YY_LEF_PHYSLEFYACC_H_INCLUDED  */
