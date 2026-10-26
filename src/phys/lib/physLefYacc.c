/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Substitute the type names.  */
#define YYSTYPE         LEF_STYPE
/* Substitute the variable and function names.  */
#define yyparse         Lef_parse
#define yylex           Lef_lex
#define yyerror         Lef_error
#define yydebug         Lef_debug
#define yynerrs         Lef_nerrs
#define yylval          Lef_lval
#define yychar          Lef_char


# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "physLefYacc.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_LEF_TOK_NAME = 3,               /* LEF_TOK_NAME  */
  YYSYMBOL_LEF_TOK_STRING = 4,             /* LEF_TOK_STRING  */
  YYSYMBOL_LEF_TOK_NUMBER = 5,             /* LEF_TOK_NUMBER  */
  YYSYMBOL_LEF_TOK_MACRO = 6,              /* LEF_TOK_MACRO  */
  YYSYMBOL_LEF_TOK_LAYER = 7,              /* LEF_TOK_LAYER  */
  YYSYMBOL_LEF_TOK_SITE = 8,               /* LEF_TOK_SITE  */
  YYSYMBOL_LEF_TOK_VIA = 9,                /* LEF_TOK_VIA  */
  YYSYMBOL_LEF_TOK_PIN = 10,               /* LEF_TOK_PIN  */
  YYSYMBOL_LEF_TOK_PORT = 11,              /* LEF_TOK_PORT  */
  YYSYMBOL_LEF_TOK_OBS = 12,               /* LEF_TOK_OBS  */
  YYSYMBOL_LEF_TOK_UNITS = 13,             /* LEF_TOK_UNITS  */
  YYSYMBOL_LEF_TOK_PROPERTYDEFINITIONS = 14, /* LEF_TOK_PROPERTYDEFINITIONS  */
  YYSYMBOL_LEF_TOK_VIARULE = 15,           /* LEF_TOK_VIARULE  */
  YYSYMBOL_LEF_TOK_NONDEFAULTRULE = 16,    /* LEF_TOK_NONDEFAULTRULE  */
  YYSYMBOL_LEF_TOK_ARRAY = 17,             /* LEF_TOK_ARRAY  */
  YYSYMBOL_LEF_TOK_END = 18,               /* LEF_TOK_END  */
  YYSYMBOL_LEF_TOK_CLASS = 19,             /* LEF_TOK_CLASS  */
  YYSYMBOL_LEF_TOK_ORIGIN = 20,            /* LEF_TOK_ORIGIN  */
  YYSYMBOL_LEF_TOK_SIZE = 21,              /* LEF_TOK_SIZE  */
  YYSYMBOL_LEF_TOK_BY = 22,                /* LEF_TOK_BY  */
  YYSYMBOL_LEF_TOK_DIRECTION = 23,         /* LEF_TOK_DIRECTION  */
  YYSYMBOL_LEF_TOK_USE = 24,               /* LEF_TOK_USE  */
  YYSYMBOL_LEF_TOK_RECT = 25,              /* LEF_TOK_RECT  */
  YYSYMBOL_LEF_TOK_MASK = 26,              /* LEF_TOK_MASK  */
  YYSYMBOL_LEF_TOK_WIDTH = 27,             /* LEF_TOK_WIDTH  */
  YYSYMBOL_LEF_TOK_PITCH = 28,             /* LEF_TOK_PITCH  */
  YYSYMBOL_LEF_TOK_RESISTANCE = 29,        /* LEF_TOK_RESISTANCE  */
  YYSYMBOL_LEF_TOK_CAPACITANCE = 30,       /* LEF_TOK_CAPACITANCE  */
  YYSYMBOL_LEF_TOK_EDGECAPACITANCE = 31,   /* LEF_TOK_EDGECAPACITANCE  */
  YYSYMBOL_LEF_TOK_TYPE = 32,              /* LEF_TOK_TYPE  */
  YYSYMBOL_LEF_TOK_DATABASE = 33,          /* LEF_TOK_DATABASE  */
  YYSYMBOL_LEF_TOK_MANUFACTURINGGRID = 34, /* LEF_TOK_MANUFACTURINGGRID  */
  YYSYMBOL_35_ = 35,                       /* ';'  */
  YYSYMBOL_36_ = 36,                       /* '('  */
  YYSYMBOL_37_ = 37,                       /* ')'  */
  YYSYMBOL_38_ = 38,                       /* ','  */
  YYSYMBOL_39_ = 39,                       /* ':'  */
  YYSYMBOL_40_ = 40,                       /* '['  */
  YYSYMBOL_41_ = 41,                       /* ']'  */
  YYSYMBOL_YYACCEPT = 42,                  /* $accept  */
  YYSYMBOL_lef_file = 43,                  /* lef_file  */
  YYSYMBOL_top_items = 44,                 /* top_items  */
  YYSYMBOL_top_item = 45,                  /* top_item  */
  YYSYMBOL_macro_group = 46,               /* macro_group  */
  YYSYMBOL_47_1 = 47,                      /* $@1  */
  YYSYMBOL_macro_items = 48,               /* macro_items  */
  YYSYMBOL_macro_item = 49,                /* macro_item  */
  YYSYMBOL_pin_group = 50,                 /* pin_group  */
  YYSYMBOL_51_2 = 51,                      /* $@2  */
  YYSYMBOL_pin_items = 52,                 /* pin_items  */
  YYSYMBOL_pin_item = 53,                  /* pin_item  */
  YYSYMBOL_port_group = 54,                /* port_group  */
  YYSYMBOL_55_3 = 55,                      /* $@3  */
  YYSYMBOL_port_items = 56,                /* port_items  */
  YYSYMBOL_port_item = 57,                 /* port_item  */
  YYSYMBOL_obs_group = 58,                 /* obs_group  */
  YYSYMBOL_59_4 = 59,                      /* $@4  */
  YYSYMBOL_obs_items = 60,                 /* obs_items  */
  YYSYMBOL_obs_item = 61,                  /* obs_item  */
  YYSYMBOL_layer_group = 62,               /* layer_group  */
  YYSYMBOL_63_5 = 63,                      /* $@5  */
  YYSYMBOL_layer_items = 64,               /* layer_items  */
  YYSYMBOL_layer_item = 65,                /* layer_item  */
  YYSYMBOL_site_group = 66,                /* site_group  */
  YYSYMBOL_67_6 = 67,                      /* $@6  */
  YYSYMBOL_site_items = 68,                /* site_items  */
  YYSYMBOL_site_item = 69,                 /* site_item  */
  YYSYMBOL_via_group = 70,                 /* via_group  */
  YYSYMBOL_71_7 = 71,                      /* $@7  */
  YYSYMBOL_units_group = 72,               /* units_group  */
  YYSYMBOL_73_8 = 73,                      /* $@8  */
  YYSYMBOL_units_items = 74,               /* units_items  */
  YYSYMBOL_propdef_group = 75,             /* propdef_group  */
  YYSYMBOL_76_9 = 76,                      /* $@9  */
  YYSYMBOL_propdef_items = 77,             /* propdef_items  */
  YYSYMBOL_propdef_item = 78,              /* propdef_item  */
  YYSYMBOL_propdef_kw = 79,                /* propdef_kw  */
  YYSYMBOL_ignore_group = 80,              /* ignore_group  */
  YYSYMBOL_81_10 = 81,                     /* $@10  */
  YYSYMBOL_ignore_kw = 82,                 /* ignore_kw  */
  YYSYMBOL_ignore_items = 83,              /* ignore_items  */
  YYSYMBOL_ignore_item = 84,               /* ignore_item  */
  YYSYMBOL_stmt = 85,                      /* stmt  */
  YYSYMBOL_keyword_stmt = 86,              /* keyword_stmt  */
  YYSYMBOL_number = 87,                    /* number  */
  YYSYMBOL_values = 88,                    /* values  */
  YYSYMBOL_atom = 89,                      /* atom  */
  YYSYMBOL_word = 90,                      /* word  */
  YYSYMBOL_end_name = 91,                  /* end_name  */
  YYSYMBOL_opt_semi = 92                   /* opt_semi  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;


/* Second part of user prologue.  */
#line 52 "physLefYacc.y"

/* The semantic actions call the static helpers in physLef.c, which
   includes this generated file at its end.  The lexer classifies the
   tokens: keywords arrive as distinct terminals, numbers arrive as
   doubles, and everything else as names or quoted strings.  The grammar
   below gives the recognized statements an explicit shape and calls a
   typed handler; statements whose keyword is not part of the supported
   subset reduce to `word values ;' and are skipped. */

#line 213 "physLefYacc.c"


#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined LEF_STYPE_IS_TRIVIAL && LEF_STYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1491

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  42
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  51
/* YYNRULES -- Number of rules.  */
#define YYNRULES  168
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  319

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   289


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      36,    37,     2,     2,    38,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    39,    35,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    40,     2,    41,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34
};

#if LEF_DEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    69,    69,    72,    74,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    95,    94,   107,   109,   113,   114,
     119,   120,   121,   122,   123,   128,   127,   140,   142,   146,
     147,   148,   149,   150,   155,   154,   165,   167,   171,   172,
     173,   174,   179,   178,   189,   191,   195,   196,   197,   198,
     207,   206,   219,   221,   225,   226,   227,   232,   231,   244,
     246,   250,   251,   256,   255,   270,   269,   280,   282,   287,
     286,   297,   299,   303,   304,   308,   309,   310,   311,   312,
     313,   314,   315,   320,   319,   333,   334,   335,   338,   340,
     344,   345,   346,   351,   352,   353,   354,   355,   363,   368,
     369,   376,   381,   385,   389,   393,   399,   405,   410,   414,
     418,   422,   426,   431,   435,   440,   444,   449,   454,   458,
     462,   466,   470,   474,   478,   489,   493,   494,   498,   499,
     500,   501,   502,   503,   504,   505,   509,   510,   511,   512,
     513,   514,   515,   516,   517,   518,   519,   520,   521,   522,
     523,   524,   525,   526,   527,   528,   529,   530,   531,   532,
     533,   534,   535,   536,   537,   538,   542,   545,   547
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "LEF_TOK_NAME",
  "LEF_TOK_STRING", "LEF_TOK_NUMBER", "LEF_TOK_MACRO", "LEF_TOK_LAYER",
  "LEF_TOK_SITE", "LEF_TOK_VIA", "LEF_TOK_PIN", "LEF_TOK_PORT",
  "LEF_TOK_OBS", "LEF_TOK_UNITS", "LEF_TOK_PROPERTYDEFINITIONS",
  "LEF_TOK_VIARULE", "LEF_TOK_NONDEFAULTRULE", "LEF_TOK_ARRAY",
  "LEF_TOK_END", "LEF_TOK_CLASS", "LEF_TOK_ORIGIN", "LEF_TOK_SIZE",
  "LEF_TOK_BY", "LEF_TOK_DIRECTION", "LEF_TOK_USE", "LEF_TOK_RECT",
  "LEF_TOK_MASK", "LEF_TOK_WIDTH", "LEF_TOK_PITCH", "LEF_TOK_RESISTANCE",
  "LEF_TOK_CAPACITANCE", "LEF_TOK_EDGECAPACITANCE", "LEF_TOK_TYPE",
  "LEF_TOK_DATABASE", "LEF_TOK_MANUFACTURINGGRID", "';'", "'('", "')'",
  "','", "':'", "'['", "']'", "$accept", "lef_file", "top_items",
  "top_item", "macro_group", "$@1", "macro_items", "macro_item",
  "pin_group", "$@2", "pin_items", "pin_item", "port_group", "$@3",
  "port_items", "port_item", "obs_group", "$@4", "obs_items", "obs_item",
  "layer_group", "$@5", "layer_items", "layer_item", "site_group", "$@6",
  "site_items", "site_item", "via_group", "$@7", "units_group", "$@8",
  "units_items", "propdef_group", "$@9", "propdef_items", "propdef_item",
  "propdef_kw", "ignore_group", "$@10", "ignore_kw", "ignore_items",
  "ignore_item", "stmt", "keyword_stmt", "number", "values", "atom",
  "word", "end_name", "opt_semi", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-172)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-3)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -172,     3,   205,  -172,   -28,  -172,     6,     7,    19,    21,
    -172,  -172,  -172,  -172,  -172,    30,  -172,     8,     8,  1457,
    1457,    20,     8,     8,  1425,  1425,  1425,  1457,  1457,     8,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,    37,  -172,
    -172,  -172,   300,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
     -27,   339,  -172,     8,    -1,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,     8,     8,     8,
      12,     9,    13,     8,    14,     8,    16,     8,    22,     8,
      25,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  1393,  1121,  -172,  -172,
    -172,    26,     8,    29,   378,   417,    18,     8,    10,  -172,
    -172,    31,  -172,    32,  -172,    33,  -172,    34,  -172,    38,
    -172,  -172,   456,  1155,   253,  1359,  1189,    28,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,    51,  -172,  -172,
    -172,  -172,    39,  -172,  -172,  -172,     8,     0,  -172,     8,
    -172,  -172,  -172,  -172,  -172,  1223,  -172,  -172,  1457,  -172,
      72,  -172,  -172,    30,  -172,  -172,  -172,  -172,  1457,  -172,
      30,  -172,  -172,  -172,    30,  -172,  -172,  -172,    73,  1457,
    -172,  -172,  -172,    30,  -172,  -172,   -27,   -27,   495,  -172,
       8,    42,     8,    43,    30,   534,    44,   573,  -172,  -172,
     612,   -27,    45,   651,   -27,   690,   -27,   729,    46,    47,
     768,   807,   846,   -27,  -172,  -172,  -172,     1,  -172,    48,
    -172,   -27,  -172,  -172,  -172,  -172,  1291,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  1257,  -172,  -172,  -172,
    -172,  -172,    49,     8,    52,  -172,   128,  -172,  1457,  -172,
     -27,  -172,  -172,    30,  -172,    53,  -172,  -172,  1457,  -172,
    -172,    30,  -172,  -172,  -172,   885,    54,   924,  -172,   -27,
      60,   963,    62,  1002,  -172,   -27,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  1325,  -172,  -172,  1457,  -172,   -27,
    -172,  -172,  1041,    66,  1080,  -172,  -172,  -172,  -172
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     1,     0,   126,     0,     0,     0,     0,
      65,    69,    85,    86,    87,     0,   126,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       4,     6,     7,     8,     9,    10,    11,    12,     0,     5,
      99,   100,     0,    14,    50,    57,    63,    67,    71,   166,
     167,     0,   125,     0,     0,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   126,   126,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    83,   129,   130,    98,   126,   132,   133,   134,   135,
     127,   128,    16,    52,    59,    88,     0,     0,   168,    13,
     101,     0,     0,     0,     0,     0,     0,     0,     0,   108,
     110,     0,   111,     0,   113,     0,   115,     0,   107,     0,
     118,    88,     0,     0,     0,     0,     0,     0,    68,    78,
      75,    76,    77,    79,    80,    81,    82,     0,    72,   126,
      73,   102,     0,   104,   105,   106,     0,     0,   122,     0,
     109,   112,   114,   116,   117,     0,   131,   126,     0,   126,
       0,    42,   126,     0,    17,    23,    24,    18,     0,   126,
       0,    53,    54,   126,     0,    60,    61,   126,     0,     0,
     126,   126,   126,     0,    89,    90,   167,   167,     0,   103,
       0,     0,     0,     0,     0,     0,     0,     0,    25,    44,
       0,   167,     0,     0,   167,     0,   167,     0,    88,     0,
       0,     0,     0,   167,    66,    70,    74,     0,   120,     0,
     121,   167,    20,    19,    21,    27,     0,    22,    15,    55,
      56,    51,    62,    58,    96,    91,     0,    93,    94,    95,
      97,    64,     0,     0,     0,    84,     0,   126,     0,   126,
     167,    45,    46,     0,   124,     0,   119,   126,     0,   126,
      34,     0,    28,    33,    29,     0,     0,     0,    43,   167,
       0,     0,     0,     0,    36,   167,    48,    47,    49,    92,
     123,    31,    30,    32,     0,    26,   126,     0,   126,   167,
      37,    38,     0,     0,     0,    35,    40,    39,    41
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -140,  -172,    -2,  -172,     5,   -14,  -172,    -8,  -151,
    -171
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,     2,    30,    31,   112,   143,   184,   185,   245,
     266,   282,   283,   294,   304,   310,   186,   219,   246,   271,
      32,   113,   144,   191,    33,   114,   145,   195,    34,   115,
      35,    47,   116,    36,    48,   117,   158,   159,    37,   141,
      38,   146,   204,   205,    40,    53,    42,   110,   111,    50,
     119
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      39,   175,    51,     3,    52,    52,    52,    41,   118,    43,
      44,    85,    86,    52,    52,    52,    93,    95,    97,    98,
      99,   122,    45,    54,    46,    52,    89,    90,    91,    92,
      94,    96,   221,    49,   100,   234,   235,   211,   262,   224,
     101,   206,     0,   226,   130,   168,    87,   129,   132,   134,
     248,   136,   233,   251,   166,   253,    88,   138,   121,   123,
     140,   161,   261,   241,   163,   207,   170,   171,   172,   173,
     265,   124,   125,   174,   209,   218,   228,   238,   240,   243,
     249,   255,   257,     0,   274,   264,     0,   276,   256,   297,
     290,   142,   126,   127,   128,   300,   131,   302,   133,   288,
     135,   317,   137,     0,   139,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   148,   160,     0,     0,   299,     0,
       0,     0,   289,     0,   305,     0,     0,   162,     0,     4,
     295,     5,   167,   169,     0,   277,   278,   279,   315,   280,
       0,   187,   192,   196,     0,   208,   281,    16,    17,    18,
       0,    19,    20,    21,     0,    22,    23,    24,    25,    26,
      27,    28,    29,   215,     0,   217,     0,     0,   220,     0,
     216,   210,   212,     0,   213,   223,     0,     0,     0,   225,
     222,     0,     0,   227,     0,     0,   230,   231,   232,     0,
       0,   229,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    -2,     4,     0,     5,     0,
       0,     6,     7,     8,     9,   237,     0,   239,    10,    11,
      12,    13,    14,    15,    16,    17,    18,     0,    19,    20,
      21,     0,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,   263,     0,   272,     0,     0,     0,     0,     0,
       0,     0,     0,   285,     4,   287,     5,     0,     0,     0,
     286,   188,   189,   291,   284,   293,     0,     0,   275,     0,
     292,   190,    16,    17,    18,     0,    19,    20,    21,     0,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,   312,     0,   314,     0,     0,     0,     0,   313,
       0,     0,   311,    55,   102,   103,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,   104,   105,     0,   106,   107,
     108,   109,    55,   102,   103,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,   120,   105,     0,   106,   107,   108,
     109,    55,   102,   103,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,   164,   105,     0,   106,   107,   108,   109,
      55,   102,   103,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,   165,   105,     0,   106,   107,   108,   109,    55,
     102,   103,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,     0,   105,   176,   106,   107,   108,   109,    55,   102,
     103,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
     236,   105,     0,   106,   107,   108,   109,    55,   102,   103,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,   242,
     105,     0,   106,   107,   108,   109,    55,   102,   103,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,   244,   105,
       0,   106,   107,   108,   109,    55,   102,   103,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,   247,   105,     0,
     106,   107,   108,   109,    55,   102,   103,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,   250,   105,     0,   106,
     107,   108,   109,    55,   102,   103,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,   252,   105,     0,   106,   107,
     108,   109,    55,   102,   103,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,   254,   105,     0,   106,   107,   108,
     109,    55,   102,   103,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,   258,   105,     0,   106,   107,   108,   109,
      55,   102,   103,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,   259,   105,     0,   106,   107,   108,   109,    55,
     102,   103,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,   260,   105,     0,   106,   107,   108,   109,    55,   102,
     103,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
     296,   105,     0,   106,   107,   108,   109,    55,   102,   103,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,   298,
     105,     0,   106,   107,   108,   109,    55,   102,   103,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,   301,   105,
       0,   106,   107,   108,   109,    55,   102,   103,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,   303,   105,     0,
     106,   107,   108,   109,    55,   102,   103,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,   316,   105,     0,   106,
     107,   108,   109,    55,   102,   103,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,   318,   105,     0,   106,   107,
     108,   109,     4,     0,     5,     0,     0,   149,   150,   151,
     152,   153,     0,     0,     0,     0,   154,   155,   156,   157,
      16,    17,    18,     0,    19,    20,    21,     0,    22,    23,
      24,    25,    26,    27,    28,    29,     4,     0,     5,     0,
       0,     0,   177,   178,   179,   180,     0,   181,     0,     0,
     182,     0,     0,   183,    16,    17,    18,     0,    19,    20,
      21,     0,    22,    23,    24,    25,    26,    27,    28,    29,
       4,     0,     5,     0,     0,   197,   198,   199,   200,   201,
       0,     0,     0,     0,   202,     0,     0,   203,    16,    17,
      18,     0,    19,    20,    21,     0,    22,    23,    24,    25,
      26,    27,    28,    29,     4,     0,     5,     0,     0,   197,
     198,   199,   200,   201,     0,     0,     0,     0,   202,     0,
       0,   214,    16,    17,    18,     0,    19,    20,    21,     0,
      22,    23,    24,    25,    26,    27,    28,    29,     4,     0,
       5,     0,     0,   197,   198,   199,   200,   201,     0,     0,
       0,     0,   202,     0,     0,   273,    16,    17,    18,     0,
      19,    20,    21,     0,    22,    23,    24,    25,    26,    27,
      28,    29,     4,     0,     5,     0,     0,     0,   267,   268,
     269,     0,     0,     0,     0,     0,     0,     0,     0,   270,
      16,    17,    18,     0,    19,    20,    21,     0,    22,    23,
      24,    25,    26,    27,    28,    29,     4,     0,     5,     0,
       0,     0,   306,   307,   308,     0,     0,     0,     0,     0,
       0,     0,     0,   309,    16,    17,    18,     0,    19,    20,
      21,     0,    22,    23,    24,    25,    26,    27,    28,    29,
       4,     0,     5,     0,     0,     0,     0,     0,   193,     0,
       0,     0,     0,     0,     0,     0,     0,   194,    16,    17,
      18,     0,    19,    20,    21,     0,    22,    23,    24,    25,
      26,    27,    28,    29,     4,     0,     5,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   147,    16,    17,    18,     0,    19,    20,    21,     0,
      22,    23,    24,    25,    26,    27,    28,    29,    55,     0,
      52,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      55,     0,     0,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84
};

static const yytype_int16 yycheck[] =
{
       2,   141,    16,     0,     5,     5,     5,    35,    35,     3,
       3,    19,    20,     5,     5,     5,    24,    25,    26,    27,
      28,    22,     3,    18,     3,     5,    21,    22,    23,    24,
      25,    26,   183,     3,    29,   206,   207,    37,    37,   190,
       3,    13,    -1,   194,    35,    35,    26,    35,    35,    35,
     221,    35,   203,   224,    36,   226,    36,    35,    53,    54,
      35,    35,   233,   214,    35,    14,    35,    35,    35,    35,
     241,    85,    86,    35,    35,     3,     3,    35,    35,    35,
      35,    35,    35,    -1,    35,    37,    -1,    35,   228,    35,
      37,   105,    87,    88,    89,    35,    91,    35,    93,   270,
      95,    35,    97,    -1,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   116,   117,    -1,    -1,   289,    -1,
      -1,    -1,   273,    -1,   295,    -1,    -1,   122,    -1,     1,
     281,     3,   127,   128,    -1,     7,     8,     9,   309,    11,
      -1,   143,   144,   145,    -1,   159,    18,    19,    20,    21,
      -1,    23,    24,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,   177,    -1,   179,    -1,    -1,   182,    -1,
     178,   166,   167,    -1,   169,   189,    -1,    -1,    -1,   193,
     188,    -1,    -1,   197,    -1,    -1,   200,   201,   202,    -1,
      -1,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     0,     1,    -1,     3,    -1,
      -1,     6,     7,     8,     9,   210,    -1,   212,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    -1,    23,    24,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      -1,    -1,   237,    -1,   246,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   267,     1,   269,     3,    -1,    -1,    -1,
     268,     8,     9,   277,   266,   279,    -1,    -1,   263,    -1,
     278,    18,    19,    20,    21,    -1,    23,    24,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    -1,
      -1,    -1,   306,    -1,   308,    -1,    -1,    -1,    -1,   307,
      -1,    -1,   304,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    -1,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    -1,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    -1,    38,    39,    40,    41,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    -1,    38,    39,    40,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    -1,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    -1,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      -1,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    -1,
      38,    39,    40,    41,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    -1,    38,
      39,    40,    41,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    -1,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    -1,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    -1,    38,    39,    40,    41,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    -1,    38,    39,    40,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    -1,    38,    39,    40,    41,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    -1,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    -1,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      -1,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    -1,
      38,    39,    40,    41,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    -1,    38,
      39,    40,    41,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    -1,    38,    39,
      40,    41,     1,    -1,     3,    -1,    -1,     6,     7,     8,
       9,    10,    -1,    -1,    -1,    -1,    15,    16,    17,    18,
      19,    20,    21,    -1,    23,    24,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,     1,    -1,     3,    -1,
      -1,    -1,     7,     8,     9,    10,    -1,    12,    -1,    -1,
      15,    -1,    -1,    18,    19,    20,    21,    -1,    23,    24,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
       1,    -1,     3,    -1,    -1,     6,     7,     8,     9,    10,
      -1,    -1,    -1,    -1,    15,    -1,    -1,    18,    19,    20,
      21,    -1,    23,    24,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,     1,    -1,     3,    -1,    -1,     6,
       7,     8,     9,    10,    -1,    -1,    -1,    -1,    15,    -1,
      -1,    18,    19,    20,    21,    -1,    23,    24,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,     1,    -1,
       3,    -1,    -1,     6,     7,     8,     9,    10,    -1,    -1,
      -1,    -1,    15,    -1,    -1,    18,    19,    20,    21,    -1,
      23,    24,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,     1,    -1,     3,    -1,    -1,    -1,     7,     8,
       9,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    18,
      19,    20,    21,    -1,    23,    24,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,     1,    -1,     3,    -1,
      -1,    -1,     7,     8,     9,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    18,    19,    20,    21,    -1,    23,    24,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
       1,    -1,     3,    -1,    -1,    -1,    -1,    -1,     9,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    18,    19,    20,
      21,    -1,    23,    24,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,     1,    -1,     3,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    18,    19,    20,    21,    -1,    23,    24,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,     3,    -1,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
       3,    -1,    -1,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    43,    44,     0,     1,     3,     6,     7,     8,     9,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    23,
      24,    25,    27,    28,    29,    30,    31,    32,    33,    34,
      45,    46,    62,    66,    70,    72,    75,    80,    82,    85,
      86,    35,    88,     3,     3,     3,     3,    73,    76,     3,
      91,    88,     5,    87,    87,     3,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    90,    90,    26,    36,    87,
      87,    87,    87,    90,    87,    90,    87,    90,    90,    90,
      87,     3,     4,     5,    35,    36,    38,    39,    40,    41,
      89,    90,    47,    63,    67,    71,    74,    77,    35,    92,
      35,    87,    22,    87,    88,    88,    87,    87,    87,    35,
      35,    87,    35,    87,    35,    87,    35,    87,    35,    87,
      35,    81,    88,    48,    64,    68,    83,    18,    85,     6,
       7,     8,     9,    10,    15,    16,    17,    18,    78,    79,
      85,    35,    87,    35,    35,    35,    36,    87,    35,    87,
      35,    35,    35,    35,    35,    83,    37,     7,     8,     9,
      10,    12,    15,    18,    49,    50,    58,    85,     8,     9,
      18,    65,    85,     9,    18,    69,    85,     6,     7,     8,
       9,    10,    15,    18,    84,    85,    13,    14,    88,    35,
      87,    37,    87,    87,    18,    88,    90,    88,     3,    59,
      88,    91,    90,    88,    91,    88,    91,    88,     3,    90,
      88,    88,    88,    91,    92,    92,    35,    87,    35,    87,
      35,    91,    35,    35,    35,    51,    60,    35,    92,    35,
      35,    92,    35,    92,    35,    35,    83,    35,    35,    35,
      35,    92,    37,    87,    37,    92,    52,     7,     8,     9,
      18,    61,    85,    18,    35,    87,    35,     7,     8,     9,
      11,    18,    53,    54,    85,    88,    90,    88,    92,    91,
      37,    88,    90,    88,    55,    91,    35,    35,    35,    92,
      35,    35,    35,    35,    56,    92,     7,     8,     9,    18,
      57,    85,    88,    90,    88,    92,    35,    35,    35
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    42,    43,    44,    44,    45,    45,    45,    45,    45,
      45,    45,    45,    45,    47,    46,    48,    48,    49,    49,
      49,    49,    49,    49,    49,    51,    50,    52,    52,    53,
      53,    53,    53,    53,    55,    54,    56,    56,    57,    57,
      57,    57,    59,    58,    60,    60,    61,    61,    61,    61,
      63,    62,    64,    64,    65,    65,    65,    67,    66,    68,
      68,    69,    69,    71,    70,    73,    72,    74,    74,    76,
      75,    77,    77,    78,    78,    79,    79,    79,    79,    79,
      79,    79,    79,    81,    80,    82,    82,    82,    83,    83,
      84,    84,    84,    84,    84,    84,    84,    84,    85,    85,
      85,    86,    86,    86,    86,    86,    86,    86,    86,    86,
      86,    86,    86,    86,    86,    86,    86,    86,    86,    86,
      86,    86,    86,    86,    86,    87,    88,    88,    89,    89,
      89,    89,    89,    89,    89,    89,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    91,    92,    92
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     0,     7,     0,     2,     1,     3,
       3,     3,     3,     1,     1,     0,     7,     0,     2,     1,
       3,     3,     3,     1,     0,     5,     0,     2,     1,     3,
       3,     3,     0,     5,     0,     2,     1,     3,     3,     3,
       0,     7,     0,     2,     1,     3,     3,     0,     7,     0,
       2,     1,     3,     0,     7,     0,     6,     0,     2,     0,
       6,     0,     2,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     7,     1,     1,     1,     0,     2,
       1,     3,     6,     3,     3,     3,     3,     3,     3,     1,
       2,     3,     4,     5,     4,     4,     4,     3,     3,     4,
       3,     3,     4,     3,     4,     3,     4,     4,     3,     8,
       6,     6,     4,    10,     8,     1,     0,     2,     1,     1,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = LEF_EMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == LEF_EMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use LEF_error or LEF_UNDEF. */
#define YYERRCODE LEF_UNDEF


/* Enable debugging if requested.  */
#if LEF_DEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !LEF_DEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !LEF_DEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_LEF_TOK_NAME: /* LEF_TOK_NAME  */
#line 50 "physLefYacc.y"
            { free(((*yyvaluep).s)); }
#line 1631 "physLefYacc.c"
        break;

    case YYSYMBOL_LEF_TOK_STRING: /* LEF_TOK_STRING  */
#line 50 "physLefYacc.y"
            { free(((*yyvaluep).s)); }
#line 1637 "physLefYacc.c"
        break;

    case YYSYMBOL_values: /* values  */
#line 50 "physLefYacc.y"
            { free(((*yyvaluep).s)); }
#line 1643 "physLefYacc.c"
        break;

    case YYSYMBOL_atom: /* atom  */
#line 50 "physLefYacc.y"
            { free(((*yyvaluep).s)); }
#line 1649 "physLefYacc.c"
        break;

    case YYSYMBOL_word: /* word  */
#line 50 "physLefYacc.y"
            { free(((*yyvaluep).s)); }
#line 1655 "physLefYacc.c"
        break;

    case YYSYMBOL_end_name: /* end_name  */
#line 50 "physLefYacc.y"
            { free(((*yyvaluep).s)); }
#line 1661 "physLefYacc.c"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = LEF_EMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == LEF_EMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= LEF_EOF)
    {
      yychar = LEF_EOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == LEF_error)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = LEF_UNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = LEF_EMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 13: /* top_item: LEF_TOK_END end_name opt_semi  */
#line 86 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1934 "physLefYacc.c"
    break;

  case 14: /* $@1: %empty  */
#line 95 "physLefYacc.y"
        {
            Lef_ParsePushMacro( Lef_ParseCtx(), (yyvsp[0].s) );
            free( (yyvsp[0].s) );
        }
#line 1943 "physLefYacc.c"
    break;

  case 15: /* macro_group: LEF_TOK_MACRO LEF_TOK_NAME $@1 macro_items LEF_TOK_END end_name opt_semi  */
#line 101 "physLefYacc.y"
        {
            Lef_ParsePopMacro( Lef_ParseCtx(), (yyvsp[-1].s) );
            free( (yyvsp[-1].s) );
        }
#line 1952 "physLefYacc.c"
    break;

  case 19: /* macro_item: LEF_TOK_SITE word ';'  */
#line 115 "physLefYacc.y"
        {
            Lef_ParseSiteRef( Lef_ParseCtx(), (yyvsp[-1].s) );
            free( (yyvsp[-1].s) );
        }
#line 1961 "physLefYacc.c"
    break;

  case 20: /* macro_item: LEF_TOK_LAYER values ';'  */
#line 119 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1967 "physLefYacc.c"
    break;

  case 21: /* macro_item: LEF_TOK_VIA values ';'  */
#line 120 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1973 "physLefYacc.c"
    break;

  case 22: /* macro_item: LEF_TOK_VIARULE values ';'  */
#line 121 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1979 "physLefYacc.c"
    break;

  case 25: /* $@2: %empty  */
#line 128 "physLefYacc.y"
        {
            Lef_ParsePushPin( Lef_ParseCtx(), (yyvsp[0].s) );
            free( (yyvsp[0].s) );
        }
#line 1988 "physLefYacc.c"
    break;

  case 26: /* pin_group: LEF_TOK_PIN LEF_TOK_NAME $@2 pin_items LEF_TOK_END end_name opt_semi  */
#line 134 "physLefYacc.y"
        {
            Lef_ParsePopPin( Lef_ParseCtx(), (yyvsp[-1].s) );
            free( (yyvsp[-1].s) );
        }
#line 1997 "physLefYacc.c"
    break;

  case 30: /* pin_item: LEF_TOK_SITE word ';'  */
#line 147 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2003 "physLefYacc.c"
    break;

  case 31: /* pin_item: LEF_TOK_LAYER values ';'  */
#line 148 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2009 "physLefYacc.c"
    break;

  case 32: /* pin_item: LEF_TOK_VIA values ';'  */
#line 149 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2015 "physLefYacc.c"
    break;

  case 34: /* $@3: %empty  */
#line 155 "physLefYacc.y"
        {
            Lef_ParsePushPort( Lef_ParseCtx() );
        }
#line 2023 "physLefYacc.c"
    break;

  case 35: /* port_group: LEF_TOK_PORT $@3 port_items LEF_TOK_END opt_semi  */
#line 160 "physLefYacc.y"
        {
            Lef_ParsePopPort( Lef_ParseCtx() );
        }
#line 2031 "physLefYacc.c"
    break;

  case 39: /* port_item: LEF_TOK_SITE word ';'  */
#line 172 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2037 "physLefYacc.c"
    break;

  case 40: /* port_item: LEF_TOK_LAYER values ';'  */
#line 173 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2043 "physLefYacc.c"
    break;

  case 41: /* port_item: LEF_TOK_VIA values ';'  */
#line 174 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2049 "physLefYacc.c"
    break;

  case 42: /* $@4: %empty  */
#line 179 "physLefYacc.y"
        {
            Lef_ParsePushObs( Lef_ParseCtx() );
        }
#line 2057 "physLefYacc.c"
    break;

  case 43: /* obs_group: LEF_TOK_OBS $@4 obs_items LEF_TOK_END opt_semi  */
#line 184 "physLefYacc.y"
        {
            Lef_ParsePopObs( Lef_ParseCtx() );
        }
#line 2065 "physLefYacc.c"
    break;

  case 47: /* obs_item: LEF_TOK_SITE word ';'  */
#line 196 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2071 "physLefYacc.c"
    break;

  case 48: /* obs_item: LEF_TOK_LAYER values ';'  */
#line 197 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2077 "physLefYacc.c"
    break;

  case 49: /* obs_item: LEF_TOK_VIA values ';'  */
#line 198 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2083 "physLefYacc.c"
    break;

  case 50: /* $@5: %empty  */
#line 207 "physLefYacc.y"
        {
            Lef_ParsePushLayer( Lef_ParseCtx(), (yyvsp[0].s) );
            free( (yyvsp[0].s) );
        }
#line 2092 "physLefYacc.c"
    break;

  case 51: /* layer_group: LEF_TOK_LAYER LEF_TOK_NAME $@5 layer_items LEF_TOK_END end_name opt_semi  */
#line 213 "physLefYacc.y"
        {
            Lef_ParsePopLayer( Lef_ParseCtx(), (yyvsp[-1].s) );
            free( (yyvsp[-1].s) );
        }
#line 2101 "physLefYacc.c"
    break;

  case 55: /* layer_item: LEF_TOK_SITE word ';'  */
#line 226 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2107 "physLefYacc.c"
    break;

  case 56: /* layer_item: LEF_TOK_VIA values ';'  */
#line 227 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2113 "physLefYacc.c"
    break;

  case 57: /* $@6: %empty  */
#line 232 "physLefYacc.y"
        {
            Lef_ParsePushSite( Lef_ParseCtx(), (yyvsp[0].s) );
            free( (yyvsp[0].s) );
        }
#line 2122 "physLefYacc.c"
    break;

  case 58: /* site_group: LEF_TOK_SITE LEF_TOK_NAME $@6 site_items LEF_TOK_END end_name opt_semi  */
#line 238 "physLefYacc.y"
        {
            Lef_ParsePopSite( Lef_ParseCtx(), (yyvsp[-1].s) );
            free( (yyvsp[-1].s) );
        }
#line 2131 "physLefYacc.c"
    break;

  case 62: /* site_item: LEF_TOK_VIA values ';'  */
#line 251 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2137 "physLefYacc.c"
    break;

  case 63: /* $@7: %empty  */
#line 256 "physLefYacc.y"
        {
            Lef_ParsePushVia( Lef_ParseCtx(), (yyvsp[0].s) );
            free( (yyvsp[0].s) );
        }
#line 2146 "physLefYacc.c"
    break;

  case 64: /* via_group: LEF_TOK_VIA LEF_TOK_NAME $@7 ignore_items LEF_TOK_END end_name opt_semi  */
#line 262 "physLefYacc.y"
        {
            Lef_ParsePopVia( Lef_ParseCtx(), (yyvsp[-1].s) );
            free( (yyvsp[-1].s) );
        }
#line 2155 "physLefYacc.c"
    break;

  case 65: /* $@8: %empty  */
#line 270 "physLefYacc.y"
        {
            Lef_ParsePushUnits( Lef_ParseCtx() );
        }
#line 2163 "physLefYacc.c"
    break;

  case 66: /* units_group: LEF_TOK_UNITS $@8 units_items LEF_TOK_END LEF_TOK_UNITS opt_semi  */
#line 275 "physLefYacc.y"
        {
            Lef_ParsePopUnits( Lef_ParseCtx() );
        }
#line 2171 "physLefYacc.c"
    break;

  case 69: /* $@9: %empty  */
#line 287 "physLefYacc.y"
        {
            Lef_ParsePushIgnore( Lef_ParseCtx() );
        }
#line 2179 "physLefYacc.c"
    break;

  case 70: /* propdef_group: LEF_TOK_PROPERTYDEFINITIONS $@9 propdef_items LEF_TOK_END LEF_TOK_PROPERTYDEFINITIONS opt_semi  */
#line 292 "physLefYacc.y"
        {
            Lef_ParsePopIgnore( Lef_ParseCtx() );
        }
#line 2187 "physLefYacc.c"
    break;

  case 74: /* propdef_item: propdef_kw values ';'  */
#line 304 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2193 "physLefYacc.c"
    break;

  case 83: /* $@10: %empty  */
#line 320 "physLefYacc.y"
        {
            Lef_ParsePushIgnore( Lef_ParseCtx() );
            free( (yyvsp[0].s) );
        }
#line 2202 "physLefYacc.c"
    break;

  case 84: /* ignore_group: ignore_kw LEF_TOK_NAME $@10 ignore_items LEF_TOK_END end_name opt_semi  */
#line 326 "physLefYacc.y"
        {
            Lef_ParsePopIgnore( Lef_ParseCtx() );
            free( (yyvsp[-1].s) );
        }
#line 2211 "physLefYacc.c"
    break;

  case 91: /* ignore_item: LEF_TOK_LAYER LEF_TOK_NAME ';'  */
#line 345 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2217 "physLefYacc.c"
    break;

  case 92: /* ignore_item: LEF_TOK_LAYER LEF_TOK_NAME ignore_items LEF_TOK_END end_name opt_semi  */
#line 347 "physLefYacc.y"
        {
            free( (yyvsp[-4].s) );
            free( (yyvsp[-1].s) );
        }
#line 2226 "physLefYacc.c"
    break;

  case 93: /* ignore_item: LEF_TOK_SITE word ';'  */
#line 351 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2232 "physLefYacc.c"
    break;

  case 94: /* ignore_item: LEF_TOK_VIA values ';'  */
#line 352 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2238 "physLefYacc.c"
    break;

  case 95: /* ignore_item: LEF_TOK_PIN values ';'  */
#line 353 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2244 "physLefYacc.c"
    break;

  case 96: /* ignore_item: LEF_TOK_MACRO values ';'  */
#line 354 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2250 "physLefYacc.c"
    break;

  case 97: /* ignore_item: LEF_TOK_VIARULE values ';'  */
#line 355 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2256 "physLefYacc.c"
    break;

  case 98: /* stmt: LEF_TOK_NAME values ';'  */
#line 364 "physLefYacc.y"
        {
            free( (yyvsp[-2].s) );
            free( (yyvsp[-1].s) );
        }
#line 2265 "physLefYacc.c"
    break;

  case 100: /* stmt: error ';'  */
#line 370 "physLefYacc.y"
        {
            Lef_ParseRecover( Lef_ParseCtx() );
        }
#line 2273 "physLefYacc.c"
    break;

  case 101: /* keyword_stmt: LEF_TOK_CLASS values ';'  */
#line 377 "physLefYacc.y"
        {
            Lef_ParseClass( Lef_ParseCtx(), (yyvsp[-1].s) );
            free( (yyvsp[-1].s) );
        }
#line 2282 "physLefYacc.c"
    break;

  case 102: /* keyword_stmt: LEF_TOK_ORIGIN number number ';'  */
#line 382 "physLefYacc.y"
        {
            Lef_ParseOrigin( Lef_ParseCtx(), (yyvsp[-2].d), (yyvsp[-1].d) );
        }
#line 2290 "physLefYacc.c"
    break;

  case 103: /* keyword_stmt: LEF_TOK_SIZE number LEF_TOK_BY number ';'  */
#line 386 "physLefYacc.y"
        {
            Lef_ParseSize( Lef_ParseCtx(), (yyvsp[-3].d), (yyvsp[-1].d) );
        }
#line 2298 "physLefYacc.c"
    break;

  case 104: /* keyword_stmt: LEF_TOK_SIZE number number ';'  */
#line 390 "physLefYacc.y"
        {
            Lef_ParseSize( Lef_ParseCtx(), (yyvsp[-2].d), (yyvsp[-1].d) );
        }
#line 2306 "physLefYacc.c"
    break;

  case 105: /* keyword_stmt: LEF_TOK_DIRECTION word values ';'  */
#line 394 "physLefYacc.y"
        {
            Lef_ParseDirection( Lef_ParseCtx(), (yyvsp[-2].s) );
            free( (yyvsp[-2].s) );
            free( (yyvsp[-1].s) );
        }
#line 2316 "physLefYacc.c"
    break;

  case 106: /* keyword_stmt: LEF_TOK_USE word values ';'  */
#line 400 "physLefYacc.y"
        {
            Lef_ParseUse( Lef_ParseCtx(), (yyvsp[-2].s) );
            free( (yyvsp[-2].s) );
            free( (yyvsp[-1].s) );
        }
#line 2326 "physLefYacc.c"
    break;

  case 107: /* keyword_stmt: LEF_TOK_TYPE word ';'  */
#line 406 "physLefYacc.y"
        {
            Lef_ParseLayerType( Lef_ParseCtx(), (yyvsp[-1].s) );
            free( (yyvsp[-1].s) );
        }
#line 2335 "physLefYacc.c"
    break;

  case 108: /* keyword_stmt: LEF_TOK_WIDTH number ';'  */
#line 411 "physLefYacc.y"
        {
            Lef_ParseWidth( Lef_ParseCtx(), (yyvsp[-1].d) );
        }
#line 2343 "physLefYacc.c"
    break;

  case 109: /* keyword_stmt: LEF_TOK_PITCH number number ';'  */
#line 415 "physLefYacc.y"
        {
            Lef_ParsePitch( Lef_ParseCtx(), (yyvsp[-2].d), (yyvsp[-1].d), 1 );
        }
#line 2351 "physLefYacc.c"
    break;

  case 110: /* keyword_stmt: LEF_TOK_PITCH number ';'  */
#line 419 "physLefYacc.y"
        {
            Lef_ParsePitch( Lef_ParseCtx(), (yyvsp[-1].d), (yyvsp[-1].d), 0 );
        }
#line 2359 "physLefYacc.c"
    break;

  case 111: /* keyword_stmt: LEF_TOK_RESISTANCE number ';'  */
#line 423 "physLefYacc.y"
        {
            Lef_ParseResistance( Lef_ParseCtx(), NULL, (yyvsp[-1].d) );
        }
#line 2367 "physLefYacc.c"
    break;

  case 112: /* keyword_stmt: LEF_TOK_RESISTANCE word number ';'  */
#line 427 "physLefYacc.y"
        {
            Lef_ParseResistance( Lef_ParseCtx(), (yyvsp[-2].s), (yyvsp[-1].d) );
            free( (yyvsp[-2].s) );
        }
#line 2376 "physLefYacc.c"
    break;

  case 113: /* keyword_stmt: LEF_TOK_CAPACITANCE number ';'  */
#line 432 "physLefYacc.y"
        {
            Lef_ParseCapacitance( Lef_ParseCtx(), NULL, (yyvsp[-1].d) );
        }
#line 2384 "physLefYacc.c"
    break;

  case 114: /* keyword_stmt: LEF_TOK_CAPACITANCE word number ';'  */
#line 436 "physLefYacc.y"
        {
            Lef_ParseCapacitance( Lef_ParseCtx(), (yyvsp[-2].s), (yyvsp[-1].d) );
            free( (yyvsp[-2].s) );
        }
#line 2393 "physLefYacc.c"
    break;

  case 115: /* keyword_stmt: LEF_TOK_EDGECAPACITANCE number ';'  */
#line 441 "physLefYacc.y"
        {
            Lef_ParseEdgeCapacitance( Lef_ParseCtx(), (yyvsp[-1].d) );
        }
#line 2401 "physLefYacc.c"
    break;

  case 116: /* keyword_stmt: LEF_TOK_EDGECAPACITANCE word number ';'  */
#line 445 "physLefYacc.y"
        {
            Lef_ParseEdgeCapacitance( Lef_ParseCtx(), (yyvsp[-1].d) );
            free( (yyvsp[-2].s) );
        }
#line 2410 "physLefYacc.c"
    break;

  case 117: /* keyword_stmt: LEF_TOK_DATABASE word number ';'  */
#line 450 "physLefYacc.y"
        {
            Lef_ParseDatabase( Lef_ParseCtx(), (yyvsp[-2].s), (yyvsp[-1].d) );
            free( (yyvsp[-2].s) );
        }
#line 2419 "physLefYacc.c"
    break;

  case 118: /* keyword_stmt: LEF_TOK_MANUFACTURINGGRID number ';'  */
#line 455 "physLefYacc.y"
        {
            Lef_ParseGrid( Lef_ParseCtx(), (yyvsp[-1].d) );
        }
#line 2427 "physLefYacc.c"
    break;

  case 119: /* keyword_stmt: LEF_TOK_RECT '(' number number number number ')' ';'  */
#line 459 "physLefYacc.y"
        {
            Lef_ParseRect( Lef_ParseCtx(), (yyvsp[-5].d), (yyvsp[-4].d), (yyvsp[-3].d), (yyvsp[-2].d) );
        }
#line 2435 "physLefYacc.c"
    break;

  case 120: /* keyword_stmt: LEF_TOK_RECT '(' number number ')' ';'  */
#line 463 "physLefYacc.y"
        {
            Lef_ParseRectPoint( Lef_ParseCtx(), (yyvsp[-3].d), (yyvsp[-2].d) );
        }
#line 2443 "physLefYacc.c"
    break;

  case 121: /* keyword_stmt: LEF_TOK_RECT number number number number ';'  */
#line 467 "physLefYacc.y"
        {
            Lef_ParseRect( Lef_ParseCtx(), (yyvsp[-4].d), (yyvsp[-3].d), (yyvsp[-2].d), (yyvsp[-1].d) );
        }
#line 2451 "physLefYacc.c"
    break;

  case 122: /* keyword_stmt: LEF_TOK_RECT number number ';'  */
#line 471 "physLefYacc.y"
        {
            Lef_ParseRectPoint( Lef_ParseCtx(), (yyvsp[-2].d), (yyvsp[-1].d) );
        }
#line 2459 "physLefYacc.c"
    break;

  case 123: /* keyword_stmt: LEF_TOK_RECT LEF_TOK_MASK number '(' number number number number ')' ';'  */
#line 475 "physLefYacc.y"
        {
            Lef_ParseRect( Lef_ParseCtx(), (yyvsp[-5].d), (yyvsp[-4].d), (yyvsp[-3].d), (yyvsp[-2].d) );
        }
#line 2467 "physLefYacc.c"
    break;

  case 124: /* keyword_stmt: LEF_TOK_RECT LEF_TOK_MASK number '(' number number ')' ';'  */
#line 479 "physLefYacc.y"
        {
            Lef_ParseRectPoint( Lef_ParseCtx(), (yyvsp[-3].d), (yyvsp[-2].d) );
        }
#line 2475 "physLefYacc.c"
    break;

  case 125: /* number: LEF_TOK_NUMBER  */
#line 489 "physLefYacc.y"
                                               { (yyval.d) = (yyvsp[0].d); }
#line 2481 "physLefYacc.c"
    break;

  case 126: /* values: %empty  */
#line 493 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "" ); }
#line 2487 "physLefYacc.c"
    break;

  case 127: /* values: values atom  */
#line 494 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrCat( (yyvsp[-1].s), (yyvsp[0].s) ); free( (yyvsp[-1].s) ); free( (yyvsp[0].s) ); }
#line 2493 "physLefYacc.c"
    break;

  case 128: /* atom: word  */
#line 498 "physLefYacc.y"
                                              { (yyval.s) = (yyvsp[0].s); }
#line 2499 "physLefYacc.c"
    break;

  case 129: /* atom: LEF_TOK_STRING  */
#line 499 "physLefYacc.y"
                                              { (yyval.s) = (yyvsp[0].s); }
#line 2505 "physLefYacc.c"
    break;

  case 130: /* atom: LEF_TOK_NUMBER  */
#line 500 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrNum( (yyvsp[0].d) ); }
#line 2511 "physLefYacc.c"
    break;

  case 131: /* atom: '(' values ')'  */
#line 501 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrWrap( "(", (yyvsp[-1].s), ")" ); free( (yyvsp[-1].s) ); }
#line 2517 "physLefYacc.c"
    break;

  case 132: /* atom: ','  */
#line 502 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "," ); }
#line 2523 "physLefYacc.c"
    break;

  case 133: /* atom: ':'  */
#line 503 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( ":" ); }
#line 2529 "physLefYacc.c"
    break;

  case 134: /* atom: '['  */
#line 504 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "[" ); }
#line 2535 "physLefYacc.c"
    break;

  case 135: /* atom: ']'  */
#line 505 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "]" ); }
#line 2541 "physLefYacc.c"
    break;

  case 136: /* word: LEF_TOK_NAME  */
#line 509 "physLefYacc.y"
                                              { (yyval.s) = (yyvsp[0].s); }
#line 2547 "physLefYacc.c"
    break;

  case 137: /* word: LEF_TOK_MACRO  */
#line 510 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "MACRO" ); }
#line 2553 "physLefYacc.c"
    break;

  case 138: /* word: LEF_TOK_LAYER  */
#line 511 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "LAYER" ); }
#line 2559 "physLefYacc.c"
    break;

  case 139: /* word: LEF_TOK_SITE  */
#line 512 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "SITE" ); }
#line 2565 "physLefYacc.c"
    break;

  case 140: /* word: LEF_TOK_VIA  */
#line 513 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "VIA" ); }
#line 2571 "physLefYacc.c"
    break;

  case 141: /* word: LEF_TOK_PIN  */
#line 514 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "PIN" ); }
#line 2577 "physLefYacc.c"
    break;

  case 142: /* word: LEF_TOK_PORT  */
#line 515 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "PORT" ); }
#line 2583 "physLefYacc.c"
    break;

  case 143: /* word: LEF_TOK_OBS  */
#line 516 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "OBS" ); }
#line 2589 "physLefYacc.c"
    break;

  case 144: /* word: LEF_TOK_UNITS  */
#line 517 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "UNITS" ); }
#line 2595 "physLefYacc.c"
    break;

  case 145: /* word: LEF_TOK_PROPERTYDEFINITIONS  */
#line 518 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "PROPERTYDEFINITIONS" ); }
#line 2601 "physLefYacc.c"
    break;

  case 146: /* word: LEF_TOK_VIARULE  */
#line 519 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "VIARULE" ); }
#line 2607 "physLefYacc.c"
    break;

  case 147: /* word: LEF_TOK_NONDEFAULTRULE  */
#line 520 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "NONDEFAULTRULE" ); }
#line 2613 "physLefYacc.c"
    break;

  case 148: /* word: LEF_TOK_ARRAY  */
#line 521 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "ARRAY" ); }
#line 2619 "physLefYacc.c"
    break;

  case 149: /* word: LEF_TOK_END  */
#line 522 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "END" ); }
#line 2625 "physLefYacc.c"
    break;

  case 150: /* word: LEF_TOK_CLASS  */
#line 523 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "CLASS" ); }
#line 2631 "physLefYacc.c"
    break;

  case 151: /* word: LEF_TOK_ORIGIN  */
#line 524 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "ORIGIN" ); }
#line 2637 "physLefYacc.c"
    break;

  case 152: /* word: LEF_TOK_SIZE  */
#line 525 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "SIZE" ); }
#line 2643 "physLefYacc.c"
    break;

  case 153: /* word: LEF_TOK_BY  */
#line 526 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "BY" ); }
#line 2649 "physLefYacc.c"
    break;

  case 154: /* word: LEF_TOK_DIRECTION  */
#line 527 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "DIRECTION" ); }
#line 2655 "physLefYacc.c"
    break;

  case 155: /* word: LEF_TOK_USE  */
#line 528 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "USE" ); }
#line 2661 "physLefYacc.c"
    break;

  case 156: /* word: LEF_TOK_RECT  */
#line 529 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "RECT" ); }
#line 2667 "physLefYacc.c"
    break;

  case 157: /* word: LEF_TOK_MASK  */
#line 530 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "MASK" ); }
#line 2673 "physLefYacc.c"
    break;

  case 158: /* word: LEF_TOK_WIDTH  */
#line 531 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "WIDTH" ); }
#line 2679 "physLefYacc.c"
    break;

  case 159: /* word: LEF_TOK_PITCH  */
#line 532 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "PITCH" ); }
#line 2685 "physLefYacc.c"
    break;

  case 160: /* word: LEF_TOK_RESISTANCE  */
#line 533 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "RESISTANCE" ); }
#line 2691 "physLefYacc.c"
    break;

  case 161: /* word: LEF_TOK_CAPACITANCE  */
#line 534 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "CAPACITANCE" ); }
#line 2697 "physLefYacc.c"
    break;

  case 162: /* word: LEF_TOK_EDGECAPACITANCE  */
#line 535 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "EDGECAPACITANCE" ); }
#line 2703 "physLefYacc.c"
    break;

  case 163: /* word: LEF_TOK_TYPE  */
#line 536 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "TYPE" ); }
#line 2709 "physLefYacc.c"
    break;

  case 164: /* word: LEF_TOK_DATABASE  */
#line 537 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "DATABASE" ); }
#line 2715 "physLefYacc.c"
    break;

  case 165: /* word: LEF_TOK_MANUFACTURINGGRID  */
#line 538 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "MANUFACTURINGGRID" ); }
#line 2721 "physLefYacc.c"
    break;

  case 166: /* end_name: LEF_TOK_NAME  */
#line 542 "physLefYacc.y"
                                              { (yyval.s) = (yyvsp[0].s); }
#line 2727 "physLefYacc.c"
    break;


#line 2731 "physLefYacc.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == LEF_EMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= LEF_EOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == LEF_EOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = LEF_EMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != LEF_EMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 550 "physLefYacc.y"

