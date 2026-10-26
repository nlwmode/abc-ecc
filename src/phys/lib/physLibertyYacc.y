%define api.prefix {Lib_}
%defines "physLibertyYacc.h"
%define parse.error verbose

%union {
    char * s;
}

%token <s> LIB_TOK_NAME
%token <s> LIB_TOK_STRING
%token <s> LIB_TOK_NUMBER

%type <s> values value arglist argitems argitem

%destructor { free($$); } <s>

%{
/* The semantic actions below call the static helpers defined in
   physLiberty.c, which includes this generated file at its end. */
%}

%%

/*------------------------------------------------------------------------*/
/* Liberty statement grammar                                              */
/*------------------------------------------------------------------------*/

stmts:
      /* empty */
    | stmts stmt
    ;

stmt:
      LIB_TOK_NAME ':' values ';'
        {
            Lib_ParseAttr( Lib_ParseCtx(), $1, $3 );
            free( $1 );
            free( $3 );
        }
    | LIB_TOK_NAME '(' arglist ')' ';'
        {
            Lib_ParseCall( Lib_ParseCtx(), $1, $3 );
            free( $1 );
            free( $3 );
        }
    | LIB_TOK_NAME '(' arglist ')' '{'
        {
            Lib_ParsePush( Lib_ParseCtx(), $1, $3 );
            free( $1 );
            free( $3 );
        }
      stmts '}'
        {
            Lib_ParsePop( Lib_ParseCtx() );
        }
    | LIB_TOK_NAME '{'
        {
            Lib_ParsePush( Lib_ParseCtx(), $1, "" );
            free( $1 );
        }
      stmts '}'
        {
            Lib_ParsePop( Lib_ParseCtx() );
        }
    | LIB_TOK_NAME ';'
        {
            free( $1 );
        }
    | ';'
    ;

/*------------------------------------------------------------------------*/
/* Attribute values and group arguments                                   */
/*------------------------------------------------------------------------*/

values:
      /* empty */             { $$ = Lib_StrDup( "" ); }
    | values value            { $$ = Lib_StrCat( $1, $2 ); free( $1 ); free( $2 ); }
    ;

value:
      LIB_TOK_NAME            { $$ = $1; }
    | LIB_TOK_NUMBER          { $$ = $1; }
    | LIB_TOK_STRING          { $$ = $1; }
    | '(' values ')'          { $$ = Lib_StrWrap( "(", $2, ")" ); free( $2 ); }
    | ','                     { $$ = Lib_StrDup( "," ); }
    ;

arglist:
      /* empty */             { $$ = Lib_StrDup( "" ); }
    | argitems                { $$ = $1; }
    ;

argitems:
      argitem                 { $$ = $1; }
    | argitems ',' argitem   { $$ = Lib_StrJoin( $1, $3 ); free( $1 ); free( $3 ); }
    ;

argitem:
      LIB_TOK_NAME            { $$ = $1; }
    | LIB_TOK_NUMBER          { $$ = $1; }
    | LIB_TOK_STRING          { $$ = $1; }
    ;

%%
