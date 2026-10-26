%option prefix="Lib_"
%option noyywrap nounput noinput nodefault
%option yylineno

%{
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

\"(\\(.|\n)|[^\"\\])*\"                { Lib_lval.s = Lib_Unquote( yytext ); return LIB_TOK_STRING; }

[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?  { Lib_lval.s = Lib_StrDup( yytext ); return LIB_TOK_NUMBER; }

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
