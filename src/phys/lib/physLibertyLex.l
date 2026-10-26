%option prefix="Lib_"
%option noyywrap nounput noinput nodefault
%option yylineno

%{
#include <stdlib.h>
#include "physLibertyYacc.h"
%}

%x COMMENT
%x LINE_COMMENT

%%

\xEF\xBB\xBF                          ;
[ \t\r\n]+                            ;
"\\"\r?\n                             ;
"/*"                                  { BEGIN(COMMENT); }
<COMMENT>"*/"                         { BEGIN(INITIAL); }
<COMMENT>(.|\n)                       ;
"//"                                  { BEGIN(LINE_COMMENT); }
<LINE_COMMENT>\r?\n                   { BEGIN(INITIAL); }
<LINE_COMMENT>(.|\n)                  ;

"library"                             { return LIB_TOK_LIBRARY; }
"cell"                                { return LIB_TOK_CELL; }
"pin"                                 { return LIB_TOK_PIN; }
"timing"                              { return LIB_TOK_TIMING; }
"lu_table_template"                   { return LIB_TOK_LU_TABLE_TEMPLATE; }
"cell_rise"                           { return LIB_TOK_CELL_RISE; }
"cell_fall"                           { return LIB_TOK_CELL_FALL; }
"rise_transition"                     { return LIB_TOK_RISE_TRANSITION; }
"fall_transition"                     { return LIB_TOK_FALL_TRANSITION; }
"index_1"                             { return LIB_TOK_INDEX_1; }
"index_2"                             { return LIB_TOK_INDEX_2; }
"values"                              { return LIB_TOK_VALUES; }
"ff"                                  { return LIB_TOK_SEQUENTIAL; }
"ff_bank"                             { return LIB_TOK_SEQUENTIAL; }
"latch"                               { return LIB_TOK_SEQUENTIAL; }
"latch_bank"                          { return LIB_TOK_SEQUENTIAL; }
"statetable"                          { return LIB_TOK_SEQUENTIAL; }

\"(\\(.|\n)|[^\"\\])*\"                { Lib_lval.s = Lib_Unquote( yytext ); return LIB_TOK_STRING; }

[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?  { Lib_lval.d = strtod( yytext, NULL ); return LIB_TOK_NUMBER; }

[A-Za-z_\\$][A-Za-z0-9_.\[\]\\$]*      { Lib_lval.s = Lib_StrDup( yytext ); return LIB_TOK_NAME; }

[(){},;:]                             { return yytext[0]; }

(.|\n)                                ;

%%

/*------------------------------------------------------------------------*/
/* scanner setup / cleanup                                                */
/*------------------------------------------------------------------------*/

void Lib_SetInput( const char * pData, int nSize )
{
    YY_BUFFER_STATE pBuffer = yy_scan_bytes( (const char *)pData, nSize );
    (void)pBuffer;
}

void Lib_Cleanup( void )
{
    yy_delete_buffer( YY_CURRENT_BUFFER );
    yylex_destroy();
}

int Lib_error( const char * pMsg )
{
    Lib_ParseError( pMsg );
    return 0;
}
