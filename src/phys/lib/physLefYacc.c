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
  YYSYMBOL_19_ = 19,                       /* ';'  */
  YYSYMBOL_20_ = 20,                       /* '('  */
  YYSYMBOL_21_ = 21,                       /* ')'  */
  YYSYMBOL_22_ = 22,                       /* ','  */
  YYSYMBOL_23_ = 23,                       /* ':'  */
  YYSYMBOL_24_ = 24,                       /* '+'  */
  YYSYMBOL_25_ = 25,                       /* '-'  */
  YYSYMBOL_26_ = 26,                       /* '['  */
  YYSYMBOL_27_ = 27,                       /* ']'  */
  YYSYMBOL_YYACCEPT = 28,                  /* $accept  */
  YYSYMBOL_lef_file = 29,                  /* lef_file  */
  YYSYMBOL_top_items = 30,                 /* top_items  */
  YYSYMBOL_top_item = 31,                  /* top_item  */
  YYSYMBOL_macro_group = 32,               /* macro_group  */
  YYSYMBOL_33_1 = 33,                      /* $@1  */
  YYSYMBOL_macro_items = 34,               /* macro_items  */
  YYSYMBOL_macro_item = 35,                /* macro_item  */
  YYSYMBOL_pin_group = 36,                 /* pin_group  */
  YYSYMBOL_37_2 = 37,                      /* $@2  */
  YYSYMBOL_pin_items = 38,                 /* pin_items  */
  YYSYMBOL_pin_item = 39,                  /* pin_item  */
  YYSYMBOL_port_group = 40,                /* port_group  */
  YYSYMBOL_41_3 = 41,                      /* $@3  */
  YYSYMBOL_port_items = 42,                /* port_items  */
  YYSYMBOL_port_item = 43,                 /* port_item  */
  YYSYMBOL_obs_group = 44,                 /* obs_group  */
  YYSYMBOL_45_4 = 45,                      /* $@4  */
  YYSYMBOL_obs_items = 46,                 /* obs_items  */
  YYSYMBOL_obs_item = 47,                  /* obs_item  */
  YYSYMBOL_layer_group = 48,               /* layer_group  */
  YYSYMBOL_49_5 = 49,                      /* $@5  */
  YYSYMBOL_layer_items = 50,               /* layer_items  */
  YYSYMBOL_layer_item = 51,                /* layer_item  */
  YYSYMBOL_site_group = 52,                /* site_group  */
  YYSYMBOL_53_6 = 53,                      /* $@6  */
  YYSYMBOL_site_items = 54,                /* site_items  */
  YYSYMBOL_site_item = 55,                 /* site_item  */
  YYSYMBOL_via_group = 56,                 /* via_group  */
  YYSYMBOL_57_7 = 57,                      /* $@7  */
  YYSYMBOL_units_group = 58,               /* units_group  */
  YYSYMBOL_59_8 = 59,                      /* $@8  */
  YYSYMBOL_units_items = 60,               /* units_items  */
  YYSYMBOL_propdef_group = 61,             /* propdef_group  */
  YYSYMBOL_62_9 = 62,                      /* $@9  */
  YYSYMBOL_propdef_items = 63,             /* propdef_items  */
  YYSYMBOL_propdef_item = 64,              /* propdef_item  */
  YYSYMBOL_propdef_kw = 65,                /* propdef_kw  */
  YYSYMBOL_ignore_group = 66,              /* ignore_group  */
  YYSYMBOL_67_10 = 67,                     /* $@10  */
  YYSYMBOL_ignore_kw = 68,                 /* ignore_kw  */
  YYSYMBOL_ignore_items = 69,              /* ignore_items  */
  YYSYMBOL_ignore_item = 70,               /* ignore_item  */
  YYSYMBOL_simple_stmt = 71,               /* simple_stmt  */
  YYSYMBOL_values = 72,                    /* values  */
  YYSYMBOL_atom = 73,                      /* atom  */
  YYSYMBOL_end_name = 74,                  /* end_name  */
  YYSYMBOL_opt_semi = 75                   /* opt_semi  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;


/* Second part of user prologue.  */
#line 31 "physLefYacc.y"

/* The semantic actions call the static helpers in physLef.c, which
   includes this generated file at its end. */

#line 191 "physLefYacc.c"


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
typedef yytype_uint8 yy_state_t;

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
#define YYLAST   888

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  28
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  48
/* YYNRULES -- Number of rules.  */
#define YYNRULES  140
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  230

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   273


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
      20,    21,     2,    24,    22,    25,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    23,    19,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    26,     2,    27,     2,     2,     2,     2,     2,     2,
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
      15,    16,    17,    18
};

#if LEF_DEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    43,    43,    46,    48,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    70,    69,    82,    84,    88,
      89,    94,    95,    96,    97,    98,   103,   102,   115,   117,
     121,   122,   123,   124,   125,   130,   129,   140,   142,   146,
     147,   148,   149,   154,   153,   164,   166,   170,   171,   172,
     173,   182,   181,   194,   196,   200,   201,   202,   207,   206,
     219,   221,   225,   226,   231,   230,   245,   244,   255,   257,
     262,   261,   272,   274,   278,   279,   283,   284,   285,   286,
     287,   288,   289,   290,   295,   294,   308,   309,   310,   313,
     315,   319,   320,   321,   326,   327,   328,   329,   330,   338,
     344,   347,   354,   355,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   383,   384,   385,   386,   387,
     388,   389,   390,   391,   392,   393,   394,   395,   396,   399,
     401
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
  "LEF_TOK_END", "';'", "'('", "')'", "','", "':'", "'+'", "'-'", "'['",
  "']'", "$accept", "lef_file", "top_items", "top_item", "macro_group",
  "$@1", "macro_items", "macro_item", "pin_group", "$@2", "pin_items",
  "pin_item", "port_group", "$@3", "port_items", "port_item", "obs_group",
  "$@4", "obs_items", "obs_item", "layer_group", "$@5", "layer_items",
  "layer_item", "site_group", "$@6", "site_items", "site_item",
  "via_group", "$@7", "units_group", "$@8", "units_items", "propdef_group",
  "$@9", "propdef_items", "propdef_item", "propdef_kw", "ignore_group",
  "$@10", "ignore_kw", "ignore_items", "ignore_item", "simple_stmt",
  "values", "atom", "end_name", "opt_semi", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-102)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-3)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -102,     6,   126,  -102,   -15,  -102,     5,    12,    15,    17,
    -102,  -102,  -102,  -102,  -102,   853,  -102,  -102,  -102,  -102,
    -102,  -102,  -102,  -102,  -102,    22,  -102,  -102,   178,  -102,
    -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,
    -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,
     -10,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,
    -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,
    -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,
      50,   159,  -102,  -102,   211,     4,    23,    26,   788,    20,
    -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,    14,
    -102,  -102,  -102,   807,  -102,  -102,  -102,  -102,    34,  -102,
    -102,   870,  -102,  -102,  -102,  -102,  -102,  -102,   870,  -102,
    -102,  -102,   870,  -102,  -102,  -102,    43,  -102,  -102,  -102,
    -102,   870,  -102,  -102,   -10,   -10,   236,   870,   261,   286,
     311,  -102,  -102,   336,   -10,   361,   386,   -10,   411,   -10,
     436,    28,   461,   486,   511,   536,   -10,  -102,  -102,  -102,
     -10,  -102,  -102,  -102,  -102,    73,  -102,  -102,  -102,  -102,
    -102,  -102,  -102,  -102,  -102,   826,  -102,  -102,  -102,  -102,
    -102,  -102,   102,  -102,  -102,  -102,   -10,  -102,  -102,   870,
    -102,  -102,  -102,  -102,   870,  -102,  -102,  -102,   561,   586,
     611,  -102,   -10,   636,   661,   686,  -102,   -10,  -102,  -102,
    -102,  -102,  -102,  -102,  -102,   839,  -102,  -102,  -102,  -102,
     -10,  -102,  -102,   711,   736,   761,  -102,  -102,  -102,  -102
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     1,     0,   102,     0,     0,     0,     0,
      66,    70,    86,    87,    88,   139,   100,     4,     6,     7,
       8,     9,    10,    11,    12,     0,     5,   101,     0,    15,
      51,    58,    64,    68,    72,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   140,
     139,    14,    84,   104,   106,   105,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,    99,   102,   108,
     109,   110,   111,   112,   113,   103,    17,    53,    60,    89,
       0,     0,    13,    89,     0,     0,     0,     0,     0,     0,
      69,    79,    76,    77,    78,    80,    81,    82,    83,     0,
      73,   102,    74,     0,   107,   102,   102,   102,     0,    43,
     102,     0,    18,    24,    25,    19,   102,   102,     0,    54,
      55,   102,     0,    61,    62,   102,     0,   102,   102,   102,
     102,     0,    90,    91,   139,   139,     0,     0,     0,     0,
       0,    26,    45,     0,   139,     0,     0,   139,     0,   139,
       0,    89,     0,     0,     0,     0,   139,    67,    71,    75,
     139,    21,    20,    22,    28,     0,    23,    16,    56,    57,
      52,    63,    59,    97,    92,     0,    94,    95,    96,    98,
      65,    85,     0,   102,   102,   102,   139,    46,    47,     0,
     102,   102,   102,    35,     0,    29,    31,    30,     0,     0,
       0,    44,   139,     0,     0,     0,    37,   139,    48,    49,
      50,    93,    32,    33,    34,     0,    27,   102,   102,   102,
     139,    38,    39,     0,     0,     0,    36,    40,    41,    42
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
    -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,
    -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,
    -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,
    -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,  -102,
    -102,   -80,  -102,    -2,   -67,  -102,  -101,   -48
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     1,     2,    17,    18,    76,    85,   112,   113,   164,
     182,   195,   196,   206,   215,   221,   114,   142,   165,   187,
      19,    77,    86,   119,    20,    78,    87,   123,    21,    79,
      22,    33,    80,    23,    34,    81,   100,   101,    24,    83,
      25,    88,   132,   133,    28,    75,    50,    51
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      26,    84,    82,   103,    27,     4,     3,     5,    29,    49,
     144,   105,   106,   107,   108,    30,   109,   147,    31,   110,
      32,   149,   111,    16,     4,    52,     5,     4,   135,     5,
     156,   116,   117,   134,   136,   121,   160,   141,   138,   139,
     140,   118,    16,   143,   122,    16,   151,   174,     0,   145,
     146,     4,     0,     5,   148,     0,     0,     0,   150,     0,
     152,   153,   154,   155,     0,     0,     0,     0,    89,    16,
       0,   175,     0,     0,     4,     0,     5,     0,    90,   102,
     183,   184,   185,   115,   120,   124,   157,   158,   202,     0,
       0,   186,    16,   207,     0,     0,   167,     0,     0,   170,
       0,   172,     0,     4,     0,     5,     0,     0,   180,   190,
     191,   192,   181,   193,     0,     0,   198,   199,   200,     0,
     194,    16,     0,   203,   204,   205,    -2,     4,     0,     5,
       0,     0,     6,     7,     8,     9,     0,     0,   201,    10,
      11,    12,    13,    14,    15,    16,     0,     0,     0,     0,
     223,   224,   225,     0,   211,     0,     0,     0,     0,   216,
       4,     0,     5,   188,     0,    91,    92,    93,    94,    95,
       0,     0,   226,     0,    96,    97,    98,    99,    16,     0,
     197,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,     0,    64,    65,    66,     0,    67,    68,     0,
      69,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,     0,   222,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
       0,    68,   104,    69,    70,    71,    72,    73,    74,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,   159,    68,     0,    69,    70,
      71,    72,    73,    74,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
     161,    68,     0,    69,    70,    71,    72,    73,    74,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,   162,    68,     0,    69,    70,
      71,    72,    73,    74,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
     163,    68,     0,    69,    70,    71,    72,    73,    74,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,   166,    68,     0,    69,    70,
      71,    72,    73,    74,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
     168,    68,     0,    69,    70,    71,    72,    73,    74,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,   169,    68,     0,    69,    70,
      71,    72,    73,    74,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
     171,    68,     0,    69,    70,    71,    72,    73,    74,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,   173,    68,     0,    69,    70,
      71,    72,    73,    74,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
     176,    68,     0,    69,    70,    71,    72,    73,    74,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,   177,    68,     0,    69,    70,
      71,    72,    73,    74,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
     178,    68,     0,    69,    70,    71,    72,    73,    74,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,   179,    68,     0,    69,    70,
      71,    72,    73,    74,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
     208,    68,     0,    69,    70,    71,    72,    73,    74,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,   209,    68,     0,    69,    70,
      71,    72,    73,    74,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
     210,    68,     0,    69,    70,    71,    72,    73,    74,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,   212,    68,     0,    69,    70,
      71,    72,    73,    74,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
     213,    68,     0,    69,    70,    71,    72,    73,    74,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,   214,    68,     0,    69,    70,
      71,    72,    73,    74,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
     227,    68,     0,    69,    70,    71,    72,    73,    74,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,   228,    68,     0,    69,    70,
      71,    72,    73,    74,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
     229,    68,     0,    69,    70,    71,    72,    73,    74,     4,
       0,     5,     0,     0,   125,   126,   127,   128,   129,     0,
       0,     0,     0,   130,     0,     0,   131,    16,     4,     0,
       5,     0,     0,   125,   126,   127,   128,   129,     0,     0,
       0,     0,   130,     0,     0,   137,    16,     4,     0,     5,
       0,     0,   125,   126,   127,   128,   129,     0,     0,     0,
       4,   130,     5,     0,   189,    16,   217,   218,   219,     0,
       0,     0,     0,     0,     0,     0,    35,   220,    16,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    35,     0,     0,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48
};

static const yytype_int16 yycheck[] =
{
       2,    68,    50,    83,    19,     1,     0,     3,     3,    19,
     111,     7,     8,     9,    10,     3,    12,   118,     3,    15,
       3,   122,    18,    19,     1,     3,     3,     1,    14,     3,
     131,     8,     9,    13,   101,     9,   137,     3,   105,   106,
     107,    18,    19,   110,    18,    19,     3,    19,    -1,   116,
     117,     1,    -1,     3,   121,    -1,    -1,    -1,   125,    -1,
     127,   128,   129,   130,    -1,    -1,    -1,    -1,    18,    19,
      -1,   151,    -1,    -1,     1,    -1,     3,    -1,    80,    81,
       7,     8,     9,    85,    86,    87,   134,   135,   189,    -1,
      -1,    18,    19,   194,    -1,    -1,   144,    -1,    -1,   147,
      -1,   149,    -1,     1,    -1,     3,    -1,    -1,   156,     7,
       8,     9,   160,    11,    -1,    -1,   183,   184,   185,    -1,
      18,    19,    -1,   190,   191,   192,     0,     1,    -1,     3,
      -1,    -1,     6,     7,     8,     9,    -1,    -1,   186,    13,
      14,    15,    16,    17,    18,    19,    -1,    -1,    -1,    -1,
     217,   218,   219,    -1,   202,    -1,    -1,    -1,    -1,   207,
       1,    -1,     3,   165,    -1,     6,     7,     8,     9,    10,
      -1,    -1,   220,    -1,    15,    16,    17,    18,    19,    -1,
     182,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    -1,    15,    16,    17,    -1,    19,    20,    -1,
      22,    23,    24,    25,    26,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   215,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,    15,    16,    17,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    15,    16,    17,    -1,    19,    20,    -1,    22,    23,
      24,    25,    26,    27,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,    15,    16,    17,    -1,
      19,    20,    -1,    22,    23,    24,    25,    26,    27,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    15,    16,    17,    -1,    19,    20,    -1,    22,    23,
      24,    25,    26,    27,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,    15,    16,    17,    -1,
      19,    20,    -1,    22,    23,    24,    25,    26,    27,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    15,    16,    17,    -1,    19,    20,    -1,    22,    23,
      24,    25,    26,    27,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,    15,    16,    17,    -1,
      19,    20,    -1,    22,    23,    24,    25,    26,    27,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    15,    16,    17,    -1,    19,    20,    -1,    22,    23,
      24,    25,    26,    27,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,    15,    16,    17,    -1,
      19,    20,    -1,    22,    23,    24,    25,    26,    27,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    15,    16,    17,    -1,    19,    20,    -1,    22,    23,
      24,    25,    26,    27,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,    15,    16,    17,    -1,
      19,    20,    -1,    22,    23,    24,    25,    26,    27,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    15,    16,    17,    -1,    19,    20,    -1,    22,    23,
      24,    25,    26,    27,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,    15,    16,    17,    -1,
      19,    20,    -1,    22,    23,    24,    25,    26,    27,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    15,    16,    17,    -1,    19,    20,    -1,    22,    23,
      24,    25,    26,    27,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,    15,    16,    17,    -1,
      19,    20,    -1,    22,    23,    24,    25,    26,    27,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    15,    16,    17,    -1,    19,    20,    -1,    22,    23,
      24,    25,    26,    27,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,    15,    16,    17,    -1,
      19,    20,    -1,    22,    23,    24,    25,    26,    27,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    15,    16,    17,    -1,    19,    20,    -1,    22,    23,
      24,    25,    26,    27,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,    15,    16,    17,    -1,
      19,    20,    -1,    22,    23,    24,    25,    26,    27,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    15,    16,    17,    -1,    19,    20,    -1,    22,    23,
      24,    25,    26,    27,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,    15,    16,    17,    -1,
      19,    20,    -1,    22,    23,    24,    25,    26,    27,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    15,    16,    17,    -1,    19,    20,    -1,    22,    23,
      24,    25,    26,    27,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,    15,    16,    17,    -1,
      19,    20,    -1,    22,    23,    24,    25,    26,    27,     1,
      -1,     3,    -1,    -1,     6,     7,     8,     9,    10,    -1,
      -1,    -1,    -1,    15,    -1,    -1,    18,    19,     1,    -1,
       3,    -1,    -1,     6,     7,     8,     9,    10,    -1,    -1,
      -1,    -1,    15,    -1,    -1,    18,    19,     1,    -1,     3,
      -1,    -1,     6,     7,     8,     9,    10,    -1,    -1,    -1,
       1,    15,     3,    -1,    18,    19,     7,     8,     9,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,    18,    19,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,     3,    -1,    -1,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    29,    30,     0,     1,     3,     6,     7,     8,     9,
      13,    14,    15,    16,    17,    18,    19,    31,    32,    48,
      52,    56,    58,    61,    66,    68,    71,    19,    72,     3,
       3,     3,     3,    59,    62,     3,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      74,    75,     3,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    15,    16,    17,    19,    20,    22,
      23,    24,    25,    26,    27,    73,    33,    49,    53,    57,
      60,    63,    75,    67,    72,    34,    50,    54,    69,    18,
      71,     6,     7,     8,     9,    10,    15,    16,    17,    18,
      64,    65,    71,    69,    21,     7,     8,     9,    10,    12,
      15,    18,    35,    36,    44,    71,     8,     9,    18,    51,
      71,     9,    18,    55,    71,     6,     7,     8,     9,    10,
      15,    18,    70,    71,    13,    14,    72,    18,    72,    72,
      72,     3,    45,    72,    74,    72,    72,    74,    72,    74,
      72,     3,    72,    72,    72,    72,    74,    75,    75,    19,
      74,    19,    19,    19,    37,    46,    19,    75,    19,    19,
      75,    19,    75,    19,    19,    69,    19,    19,    19,    19,
      75,    75,    38,     7,     8,     9,    18,    47,    71,    18,
       7,     8,     9,    11,    18,    39,    40,    71,    72,    72,
      72,    75,    74,    72,    72,    72,    41,    74,    19,    19,
      19,    75,    19,    19,    19,    42,    75,     7,     8,     9,
      18,    43,    71,    72,    72,    72,    75,    19,    19,    19
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    28,    29,    30,    30,    31,    31,    31,    31,    31,
      31,    31,    31,    31,    31,    33,    32,    34,    34,    35,
      35,    35,    35,    35,    35,    35,    37,    36,    38,    38,
      39,    39,    39,    39,    39,    41,    40,    42,    42,    43,
      43,    43,    43,    45,    44,    46,    46,    47,    47,    47,
      47,    49,    48,    50,    50,    51,    51,    51,    53,    52,
      54,    54,    55,    55,    57,    56,    59,    58,    60,    60,
      62,    61,    63,    63,    64,    64,    65,    65,    65,    65,
      65,    65,    65,    65,    67,    66,    68,    68,    68,    69,
      69,    70,    70,    70,    70,    70,    70,    70,    70,    71,
      71,    71,    72,    72,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    74,    74,    74,    74,    74,
      74,    74,    74,    74,    74,    74,    74,    74,    74,    75,
      75
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     2,     0,     7,     0,     2,     1,
       3,     3,     3,     3,     1,     1,     0,     7,     0,     2,
       1,     1,     3,     3,     3,     0,     5,     0,     2,     1,
       3,     3,     3,     0,     5,     0,     2,     1,     3,     3,
       3,     0,     7,     0,     2,     1,     3,     3,     0,     7,
       0,     2,     1,     3,     0,     7,     0,     6,     0,     2,
       0,     6,     0,     2,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     0,     7,     1,     1,     1,     0,
       2,     1,     3,     6,     3,     3,     3,     3,     3,     3,
       1,     2,     0,     2,     1,     1,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       1
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
#line 29 "physLefYacc.y"
            { free(((*yyvaluep).s)); }
#line 1446 "physLefYacc.c"
        break;

    case YYSYMBOL_LEF_TOK_STRING: /* LEF_TOK_STRING  */
#line 29 "physLefYacc.y"
            { free(((*yyvaluep).s)); }
#line 1452 "physLefYacc.c"
        break;

    case YYSYMBOL_LEF_TOK_NUMBER: /* LEF_TOK_NUMBER  */
#line 29 "physLefYacc.y"
            { free(((*yyvaluep).s)); }
#line 1458 "physLefYacc.c"
        break;

    case YYSYMBOL_values: /* values  */
#line 29 "physLefYacc.y"
            { free(((*yyvaluep).s)); }
#line 1464 "physLefYacc.c"
        break;

    case YYSYMBOL_atom: /* atom  */
#line 29 "physLefYacc.y"
            { free(((*yyvaluep).s)); }
#line 1470 "physLefYacc.c"
        break;

    case YYSYMBOL_end_name: /* end_name  */
#line 29 "physLefYacc.y"
            { free(((*yyvaluep).s)); }
#line 1476 "physLefYacc.c"
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
#line 60 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1749 "physLefYacc.c"
    break;

  case 15: /* $@1: %empty  */
#line 70 "physLefYacc.y"
        {
            Lef_ParsePushMacro( Lef_ParseCtx(), (yyvsp[0].s) );
            free( (yyvsp[0].s) );
        }
#line 1758 "physLefYacc.c"
    break;

  case 16: /* macro_group: LEF_TOK_MACRO LEF_TOK_NAME $@1 macro_items LEF_TOK_END end_name opt_semi  */
#line 76 "physLefYacc.y"
        {
            Lef_ParsePopMacro( Lef_ParseCtx(), (yyvsp[-1].s) );
            free( (yyvsp[-1].s) );
        }
#line 1767 "physLefYacc.c"
    break;

  case 20: /* macro_item: LEF_TOK_SITE values ';'  */
#line 90 "physLefYacc.y"
        {
            Lef_ParseSimple( Lef_ParseCtx(), "SITE", (yyvsp[-1].s) );
            free( (yyvsp[-1].s) );
        }
#line 1776 "physLefYacc.c"
    break;

  case 21: /* macro_item: LEF_TOK_LAYER values ';'  */
#line 94 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1782 "physLefYacc.c"
    break;

  case 22: /* macro_item: LEF_TOK_VIA values ';'  */
#line 95 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1788 "physLefYacc.c"
    break;

  case 23: /* macro_item: LEF_TOK_VIARULE values ';'  */
#line 96 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1794 "physLefYacc.c"
    break;

  case 26: /* $@2: %empty  */
#line 103 "physLefYacc.y"
        {
            Lef_ParsePushPin( Lef_ParseCtx(), (yyvsp[0].s) );
            free( (yyvsp[0].s) );
        }
#line 1803 "physLefYacc.c"
    break;

  case 27: /* pin_group: LEF_TOK_PIN LEF_TOK_NAME $@2 pin_items LEF_TOK_END end_name opt_semi  */
#line 109 "physLefYacc.y"
        {
            Lef_ParsePopPin( Lef_ParseCtx(), (yyvsp[-1].s) );
            free( (yyvsp[-1].s) );
        }
#line 1812 "physLefYacc.c"
    break;

  case 32: /* pin_item: LEF_TOK_LAYER values ';'  */
#line 123 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1818 "physLefYacc.c"
    break;

  case 33: /* pin_item: LEF_TOK_SITE values ';'  */
#line 124 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1824 "physLefYacc.c"
    break;

  case 34: /* pin_item: LEF_TOK_VIA values ';'  */
#line 125 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1830 "physLefYacc.c"
    break;

  case 35: /* $@3: %empty  */
#line 130 "physLefYacc.y"
        {
            Lef_ParsePushPort( Lef_ParseCtx() );
        }
#line 1838 "physLefYacc.c"
    break;

  case 36: /* port_group: LEF_TOK_PORT $@3 port_items LEF_TOK_END opt_semi  */
#line 135 "physLefYacc.y"
        {
            Lef_ParsePopPort( Lef_ParseCtx() );
        }
#line 1846 "physLefYacc.c"
    break;

  case 40: /* port_item: LEF_TOK_LAYER values ';'  */
#line 147 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1852 "physLefYacc.c"
    break;

  case 41: /* port_item: LEF_TOK_SITE values ';'  */
#line 148 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1858 "physLefYacc.c"
    break;

  case 42: /* port_item: LEF_TOK_VIA values ';'  */
#line 149 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1864 "physLefYacc.c"
    break;

  case 43: /* $@4: %empty  */
#line 154 "physLefYacc.y"
        {
            Lef_ParsePushObs( Lef_ParseCtx() );
        }
#line 1872 "physLefYacc.c"
    break;

  case 44: /* obs_group: LEF_TOK_OBS $@4 obs_items LEF_TOK_END opt_semi  */
#line 159 "physLefYacc.y"
        {
            Lef_ParsePopObs( Lef_ParseCtx() );
        }
#line 1880 "physLefYacc.c"
    break;

  case 48: /* obs_item: LEF_TOK_LAYER values ';'  */
#line 171 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1886 "physLefYacc.c"
    break;

  case 49: /* obs_item: LEF_TOK_SITE values ';'  */
#line 172 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1892 "physLefYacc.c"
    break;

  case 50: /* obs_item: LEF_TOK_VIA values ';'  */
#line 173 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1898 "physLefYacc.c"
    break;

  case 51: /* $@5: %empty  */
#line 182 "physLefYacc.y"
        {
            Lef_ParsePushLayer( Lef_ParseCtx(), (yyvsp[0].s) );
            free( (yyvsp[0].s) );
        }
#line 1907 "physLefYacc.c"
    break;

  case 52: /* layer_group: LEF_TOK_LAYER LEF_TOK_NAME $@5 layer_items LEF_TOK_END end_name opt_semi  */
#line 188 "physLefYacc.y"
        {
            Lef_ParsePopLayer( Lef_ParseCtx(), (yyvsp[-1].s) );
            free( (yyvsp[-1].s) );
        }
#line 1916 "physLefYacc.c"
    break;

  case 56: /* layer_item: LEF_TOK_SITE values ';'  */
#line 201 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1922 "physLefYacc.c"
    break;

  case 57: /* layer_item: LEF_TOK_VIA values ';'  */
#line 202 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1928 "physLefYacc.c"
    break;

  case 58: /* $@6: %empty  */
#line 207 "physLefYacc.y"
        {
            Lef_ParsePushSite( Lef_ParseCtx(), (yyvsp[0].s) );
            free( (yyvsp[0].s) );
        }
#line 1937 "physLefYacc.c"
    break;

  case 59: /* site_group: LEF_TOK_SITE LEF_TOK_NAME $@6 site_items LEF_TOK_END end_name opt_semi  */
#line 213 "physLefYacc.y"
        {
            Lef_ParsePopSite( Lef_ParseCtx(), (yyvsp[-1].s) );
            free( (yyvsp[-1].s) );
        }
#line 1946 "physLefYacc.c"
    break;

  case 63: /* site_item: LEF_TOK_VIA values ';'  */
#line 226 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 1952 "physLefYacc.c"
    break;

  case 64: /* $@7: %empty  */
#line 231 "physLefYacc.y"
        {
            Lef_ParsePushVia( Lef_ParseCtx(), (yyvsp[0].s) );
            free( (yyvsp[0].s) );
        }
#line 1961 "physLefYacc.c"
    break;

  case 65: /* via_group: LEF_TOK_VIA LEF_TOK_NAME $@7 ignore_items LEF_TOK_END end_name opt_semi  */
#line 237 "physLefYacc.y"
        {
            Lef_ParsePopVia( Lef_ParseCtx(), (yyvsp[-1].s) );
            free( (yyvsp[-1].s) );
        }
#line 1970 "physLefYacc.c"
    break;

  case 66: /* $@8: %empty  */
#line 245 "physLefYacc.y"
        {
            Lef_ParsePushUnits( Lef_ParseCtx() );
        }
#line 1978 "physLefYacc.c"
    break;

  case 67: /* units_group: LEF_TOK_UNITS $@8 units_items LEF_TOK_END LEF_TOK_UNITS opt_semi  */
#line 250 "physLefYacc.y"
        {
            Lef_ParsePopUnits( Lef_ParseCtx() );
        }
#line 1986 "physLefYacc.c"
    break;

  case 70: /* $@9: %empty  */
#line 262 "physLefYacc.y"
        {
            Lef_ParsePushIgnore( Lef_ParseCtx() );
        }
#line 1994 "physLefYacc.c"
    break;

  case 71: /* propdef_group: LEF_TOK_PROPERTYDEFINITIONS $@9 propdef_items LEF_TOK_END LEF_TOK_PROPERTYDEFINITIONS opt_semi  */
#line 267 "physLefYacc.y"
        {
            Lef_ParsePopIgnore( Lef_ParseCtx() );
        }
#line 2002 "physLefYacc.c"
    break;

  case 75: /* propdef_item: propdef_kw values ';'  */
#line 279 "physLefYacc.y"
                                               { free( (yyvsp[-1].s) ); }
#line 2008 "physLefYacc.c"
    break;

  case 84: /* $@10: %empty  */
#line 295 "physLefYacc.y"
        {
            Lef_ParsePushIgnore( Lef_ParseCtx() );
            free( (yyvsp[0].s) );
        }
#line 2017 "physLefYacc.c"
    break;

  case 85: /* ignore_group: ignore_kw LEF_TOK_NAME $@10 ignore_items LEF_TOK_END end_name opt_semi  */
#line 301 "physLefYacc.y"
        {
            Lef_ParsePopIgnore( Lef_ParseCtx() );
            free( (yyvsp[-1].s) );
        }
#line 2026 "physLefYacc.c"
    break;

  case 92: /* ignore_item: LEF_TOK_LAYER LEF_TOK_NAME ';'  */
#line 320 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2032 "physLefYacc.c"
    break;

  case 93: /* ignore_item: LEF_TOK_LAYER LEF_TOK_NAME ignore_items LEF_TOK_END end_name opt_semi  */
#line 322 "physLefYacc.y"
        {
            free( (yyvsp[-4].s) );
            free( (yyvsp[-1].s) );
        }
#line 2041 "physLefYacc.c"
    break;

  case 94: /* ignore_item: LEF_TOK_SITE values ';'  */
#line 326 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2047 "physLefYacc.c"
    break;

  case 95: /* ignore_item: LEF_TOK_VIA values ';'  */
#line 327 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2053 "physLefYacc.c"
    break;

  case 96: /* ignore_item: LEF_TOK_PIN values ';'  */
#line 328 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2059 "physLefYacc.c"
    break;

  case 97: /* ignore_item: LEF_TOK_MACRO values ';'  */
#line 329 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2065 "physLefYacc.c"
    break;

  case 98: /* ignore_item: LEF_TOK_VIARULE values ';'  */
#line 330 "physLefYacc.y"
                                              { free( (yyvsp[-1].s) ); }
#line 2071 "physLefYacc.c"
    break;

  case 99: /* simple_stmt: LEF_TOK_NAME values ';'  */
#line 339 "physLefYacc.y"
        {
            Lef_ParseSimple( Lef_ParseCtx(), (yyvsp[-2].s), (yyvsp[-1].s) );
            free( (yyvsp[-2].s) );
            free( (yyvsp[-1].s) );
        }
#line 2081 "physLefYacc.c"
    break;

  case 100: /* simple_stmt: ';'  */
#line 345 "physLefYacc.y"
        {
        }
#line 2088 "physLefYacc.c"
    break;

  case 101: /* simple_stmt: error ';'  */
#line 348 "physLefYacc.y"
        {
            Lef_ParseRecover( Lef_ParseCtx() );
        }
#line 2096 "physLefYacc.c"
    break;

  case 102: /* values: %empty  */
#line 354 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrDup( "" ); }
#line 2102 "physLefYacc.c"
    break;

  case 103: /* values: values atom  */
#line 355 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrCat( (yyvsp[-1].s), (yyvsp[0].s) ); free( (yyvsp[-1].s) ); free( (yyvsp[0].s) ); }
#line 2108 "physLefYacc.c"
    break;

  case 104: /* atom: LEF_TOK_NAME  */
#line 359 "physLefYacc.y"
                                             { (yyval.s) = (yyvsp[0].s); }
#line 2114 "physLefYacc.c"
    break;

  case 105: /* atom: LEF_TOK_NUMBER  */
#line 360 "physLefYacc.y"
                                             { (yyval.s) = (yyvsp[0].s); }
#line 2120 "physLefYacc.c"
    break;

  case 106: /* atom: LEF_TOK_STRING  */
#line 361 "physLefYacc.y"
                                             { (yyval.s) = (yyvsp[0].s); }
#line 2126 "physLefYacc.c"
    break;

  case 107: /* atom: '(' values ')'  */
#line 362 "physLefYacc.y"
                                              { (yyval.s) = Lef_StrWrap( "(", (yyvsp[-1].s), ")" ); free( (yyvsp[-1].s) ); }
#line 2132 "physLefYacc.c"
    break;

  case 108: /* atom: ','  */
#line 363 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "," ); }
#line 2138 "physLefYacc.c"
    break;

  case 109: /* atom: ':'  */
#line 364 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( ":" ); }
#line 2144 "physLefYacc.c"
    break;

  case 110: /* atom: '+'  */
#line 365 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "+" ); }
#line 2150 "physLefYacc.c"
    break;

  case 111: /* atom: '-'  */
#line 366 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "-" ); }
#line 2156 "physLefYacc.c"
    break;

  case 112: /* atom: '['  */
#line 367 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "[" ); }
#line 2162 "physLefYacc.c"
    break;

  case 113: /* atom: ']'  */
#line 368 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "]" ); }
#line 2168 "physLefYacc.c"
    break;

  case 114: /* atom: LEF_TOK_MACRO  */
#line 369 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "MACRO" ); }
#line 2174 "physLefYacc.c"
    break;

  case 115: /* atom: LEF_TOK_LAYER  */
#line 370 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "LAYER" ); }
#line 2180 "physLefYacc.c"
    break;

  case 116: /* atom: LEF_TOK_SITE  */
#line 371 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "SITE" ); }
#line 2186 "physLefYacc.c"
    break;

  case 117: /* atom: LEF_TOK_VIA  */
#line 372 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "VIA" ); }
#line 2192 "physLefYacc.c"
    break;

  case 118: /* atom: LEF_TOK_PIN  */
#line 373 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "PIN" ); }
#line 2198 "physLefYacc.c"
    break;

  case 119: /* atom: LEF_TOK_PORT  */
#line 374 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "PORT" ); }
#line 2204 "physLefYacc.c"
    break;

  case 120: /* atom: LEF_TOK_OBS  */
#line 375 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "OBS" ); }
#line 2210 "physLefYacc.c"
    break;

  case 121: /* atom: LEF_TOK_UNITS  */
#line 376 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "UNITS" ); }
#line 2216 "physLefYacc.c"
    break;

  case 122: /* atom: LEF_TOK_VIARULE  */
#line 377 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "VIARULE" ); }
#line 2222 "physLefYacc.c"
    break;

  case 123: /* atom: LEF_TOK_NONDEFAULTRULE  */
#line 378 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "NONDEFAULTRULE" ); }
#line 2228 "physLefYacc.c"
    break;

  case 124: /* atom: LEF_TOK_ARRAY  */
#line 379 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "ARRAY" ); }
#line 2234 "physLefYacc.c"
    break;

  case 125: /* end_name: LEF_TOK_NAME  */
#line 383 "physLefYacc.y"
                                             { (yyval.s) = (yyvsp[0].s); }
#line 2240 "physLefYacc.c"
    break;

  case 126: /* end_name: LEF_TOK_MACRO  */
#line 384 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "MACRO" ); }
#line 2246 "physLefYacc.c"
    break;

  case 127: /* end_name: LEF_TOK_LAYER  */
#line 385 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "LAYER" ); }
#line 2252 "physLefYacc.c"
    break;

  case 128: /* end_name: LEF_TOK_SITE  */
#line 386 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "SITE" ); }
#line 2258 "physLefYacc.c"
    break;

  case 129: /* end_name: LEF_TOK_VIA  */
#line 387 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "VIA" ); }
#line 2264 "physLefYacc.c"
    break;

  case 130: /* end_name: LEF_TOK_PIN  */
#line 388 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "PIN" ); }
#line 2270 "physLefYacc.c"
    break;

  case 131: /* end_name: LEF_TOK_PORT  */
#line 389 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "PORT" ); }
#line 2276 "physLefYacc.c"
    break;

  case 132: /* end_name: LEF_TOK_OBS  */
#line 390 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "OBS" ); }
#line 2282 "physLefYacc.c"
    break;

  case 133: /* end_name: LEF_TOK_UNITS  */
#line 391 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "UNITS" ); }
#line 2288 "physLefYacc.c"
    break;

  case 134: /* end_name: LEF_TOK_PROPERTYDEFINITIONS  */
#line 392 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "PROPERTYDEFINITIONS" ); }
#line 2294 "physLefYacc.c"
    break;

  case 135: /* end_name: LEF_TOK_VIARULE  */
#line 393 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "VIARULE" ); }
#line 2300 "physLefYacc.c"
    break;

  case 136: /* end_name: LEF_TOK_NONDEFAULTRULE  */
#line 394 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "NONDEFAULTRULE" ); }
#line 2306 "physLefYacc.c"
    break;

  case 137: /* end_name: LEF_TOK_ARRAY  */
#line 395 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "ARRAY" ); }
#line 2312 "physLefYacc.c"
    break;

  case 138: /* end_name: LEF_TOK_END  */
#line 396 "physLefYacc.y"
                                             { (yyval.s) = Lef_StrDup( "END" ); }
#line 2318 "physLefYacc.c"
    break;


#line 2322 "physLefYacc.c"

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

#line 404 "physLefYacc.y"

