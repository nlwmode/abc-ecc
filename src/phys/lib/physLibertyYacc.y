%define api.prefix {Lib_}
%defines "physLibertyYacc.h"
%define parse.error verbose

%union {
    char * s;
    double d;
}

%token <s> LIB_TOK_NAME
%token <s> LIB_TOK_STRING
%token <d> LIB_TOK_NUMBER

/* groups with a fixed meaning in the physical-synthesis flow */
%token LIB_TOK_LIBRARY
%token LIB_TOK_CELL
%token LIB_TOK_PIN
%token LIB_TOK_TIMING
%token LIB_TOK_LU_TABLE_TEMPLATE
%token LIB_TOK_CELL_RISE
%token LIB_TOK_CELL_FALL
%token LIB_TOK_RISE_TRANSITION
%token LIB_TOK_FALL_TRANSITION
%token LIB_TOK_SEQUENTIAL

/* numeric table statements */
%token LIB_TOK_INDEX_1
%token LIB_TOK_INDEX_2
%token LIB_TOK_VALUES

%type <s> attr_values attr_atom arglist argitems argitem

%destructor { free($$); } <s>

%{
/* The semantic actions below call the static helpers defined in
   physLiberty.c, which includes this generated file at its end.  The
   lexer classifies the statement keys of the supported subset (groups
   and numeric table statements) and returns a distinct token for each,
   so the group structure is resolved here rather than by string
   comparison in the actions.  Attributes keep their open-ended
   `name : value ;' shape; unknown groups fall back to the generic
   group productions and are skipped. */
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
      LIB_TOK_LIBRARY '(' arglist ')' '{'
        {
            Lib_PushLibrary( Lib_ParseCtx(), $3 );
            free( $3 );
        }
      stmts '}'
        {
            Lib_ParsePop( Lib_ParseCtx() );
        }
    | LIB_TOK_CELL '(' arglist ')' '{'
        {
            Lib_PushCell( Lib_ParseCtx(), $3 );
            free( $3 );
        }
      stmts '}'
        {
            Lib_ParsePop( Lib_ParseCtx() );
        }
    | LIB_TOK_PIN '(' arglist ')' '{'
        {
            Lib_PushPin( Lib_ParseCtx(), $3 );
            free( $3 );
        }
      stmts '}'
        {
            Lib_ParsePop( Lib_ParseCtx() );
        }
    | LIB_TOK_TIMING '(' arglist ')' '{'
        {
            Lib_PushTiming( Lib_ParseCtx() );
            free( $3 );
        }
      stmts '}'
        {
            Lib_ParsePop( Lib_ParseCtx() );
        }
    | LIB_TOK_LU_TABLE_TEMPLATE '(' arglist ')' '{'
        {
            Lib_PushTemplate( Lib_ParseCtx(), $3 );
            free( $3 );
        }
      stmts '}'
        {
            Lib_ParsePop( Lib_ParseCtx() );
        }
    | LIB_TOK_CELL_RISE '(' arglist ')' '{'
        {
            Lib_PushTable( Lib_ParseCtx(), 0, $3 );
            free( $3 );
        }
      stmts '}'
        {
            Lib_ParsePop( Lib_ParseCtx() );
        }
    | LIB_TOK_CELL_FALL '(' arglist ')' '{'
        {
            Lib_PushTable( Lib_ParseCtx(), 1, $3 );
            free( $3 );
        }
      stmts '}'
        {
            Lib_ParsePop( Lib_ParseCtx() );
        }
    | LIB_TOK_RISE_TRANSITION '(' arglist ')' '{'
        {
            Lib_PushTable( Lib_ParseCtx(), 2, $3 );
            free( $3 );
        }
      stmts '}'
        {
            Lib_ParsePop( Lib_ParseCtx() );
        }
    | LIB_TOK_FALL_TRANSITION '(' arglist ')' '{'
        {
            Lib_PushTable( Lib_ParseCtx(), 3, $3 );
            free( $3 );
        }
      stmts '}'
        {
            Lib_ParsePop( Lib_ParseCtx() );
        }
    | LIB_TOK_SEQUENTIAL '(' arglist ')' '{'
        {
            Lib_PushSequential( Lib_ParseCtx() );
            free( $3 );
        }
      stmts '}'
        {
            Lib_ParsePop( Lib_ParseCtx() );
        }
    | LIB_TOK_INDEX_1 '(' arglist ')' ';'
        {
            Lib_ParseIndex( Lib_ParseCtx(), 0, $3 );
            free( $3 );
        }
    | LIB_TOK_INDEX_2 '(' arglist ')' ';'
        {
            Lib_ParseIndex( Lib_ParseCtx(), 1, $3 );
            free( $3 );
        }
    | LIB_TOK_VALUES '(' arglist ')' ';'
        {
            Lib_ParseValues( Lib_ParseCtx(), $3 );
            free( $3 );
        }
    | LIB_TOK_NAME ':' attr_values ';'
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
            Lib_PushIgnore( Lib_ParseCtx() );
            free( $1 );
            free( $3 );
        }
      stmts '}'
        {
            Lib_ParsePop( Lib_ParseCtx() );
        }
    | LIB_TOK_NAME '{'
        {
            Lib_PushIgnore( Lib_ParseCtx() );
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
    | error ';'
        {
            Lib_ParseRecover( Lib_ParseCtx() );
        }
    ;

/*------------------------------------------------------------------------*/
/* Attribute values and group arguments                                   */
/*------------------------------------------------------------------------*/

attr_values:
      /* empty */             { $$ = Lib_StrDup( "" ); }
    | attr_values attr_atom   { $$ = Lib_StrCat( $1, $2 ); free( $1 ); free( $2 ); }
    ;

attr_atom:
      LIB_TOK_NAME            { $$ = $1; }
    | LIB_TOK_NUMBER          { $$ = Lib_StrNum( $1 ); }
    | LIB_TOK_STRING          { $$ = $1; }
    | '(' attr_values ')'     { $$ = Lib_StrWrap( "(", $2, ")" ); free( $2 ); }
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
    | LIB_TOK_NUMBER          { $$ = Lib_StrNum( $1 ); }
    | LIB_TOK_STRING          { $$ = $1; }
    ;

%%
