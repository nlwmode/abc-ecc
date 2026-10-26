%option prefix="Lef_"
%option noyywrap nounput noinput nodefault
%option yylineno

%{
#include <stdlib.h>
#include "physLefYacc.h"
%}

%x EXT

%%

\xEF\xBB\xBF                          ;
[ \t\r\n]+                            ;
"\\"\r?\n                             ;
"#"[^\n]*                             ;

"BEGINEXT"                            { BEGIN(EXT); }
<EXT>"ENDEXT"                         { BEGIN(INITIAL); }
<EXT>(.|\n)                           ;

"MACRO"                               { return LEF_TOK_MACRO; }
"LAYER"                               { return LEF_TOK_LAYER; }
"SITE"                                { return LEF_TOK_SITE; }
"VIA"                                 { return LEF_TOK_VIA; }
"PIN"                                 { return LEF_TOK_PIN; }
"PORT"                                { return LEF_TOK_PORT; }
"OBS"                                 { return LEF_TOK_OBS; }
"UNITS"                               { return LEF_TOK_UNITS; }
"PROPERTYDEFINITIONS"                 { return LEF_TOK_PROPERTYDEFINITIONS; }
"VIARULE"                             { return LEF_TOK_VIARULE; }
"NONDEFAULTRULE"                      { return LEF_TOK_NONDEFAULTRULE; }
"ARRAY"                               { return LEF_TOK_ARRAY; }
"END"                                 { return LEF_TOK_END; }

"CLASS"                               { return LEF_TOK_CLASS; }
"ORIGIN"                              { return LEF_TOK_ORIGIN; }
"SIZE"                                { return LEF_TOK_SIZE; }
"BY"                                  { return LEF_TOK_BY; }
"DIRECTION"                           { return LEF_TOK_DIRECTION; }
"USE"                                 { return LEF_TOK_USE; }
"RECT"                                { return LEF_TOK_RECT; }
"MASK"                                { return LEF_TOK_MASK; }
"WIDTH"                               { return LEF_TOK_WIDTH; }
"PITCH"                               { return LEF_TOK_PITCH; }
"RESISTANCE"                          { return LEF_TOK_RESISTANCE; }
"CAPACITANCE"                         { return LEF_TOK_CAPACITANCE; }
"EDGECAPACITANCE"                     { return LEF_TOK_EDGECAPACITANCE; }
"TYPE"                                { return LEF_TOK_TYPE; }
"DATABASE"                            { return LEF_TOK_DATABASE; }
"MANUFACTURINGGRID"                   { return LEF_TOK_MANUFACTURINGGRID; }

\"([^\"\\]|\\[^\n])*\"                { Lef_lval.s = Lef_Unquote( yytext ); return LEF_TOK_STRING; }

[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?  { Lef_lval.d = strtod( yytext, NULL ); return LEF_TOK_NUMBER; }

[A-Za-z_][A-Za-z0-9_.\[\]\\/$]*        { Lef_lval.s = Lef_StrDup( yytext ); return LEF_TOK_NAME; }

[;:(),\[\]]                           { return yytext[0]; }

(.|\n)                                ;

%%

/*------------------------------------------------------------------------*/
/* scanner setup / cleanup                                                */
/*------------------------------------------------------------------------*/

void Lef_SetInput( const char * pData, int nSize )
{
    YY_BUFFER_STATE pBuffer = yy_scan_bytes( (const char *)pData, nSize );
    (void)pBuffer;
}

void Lef_Cleanup( void )
{
    yy_delete_buffer( YY_CURRENT_BUFFER );
    yylex_destroy();
}

int Lef_error( const char * pMsg )
{
    Lef_ParseError( pMsg );
    return 0;
}
