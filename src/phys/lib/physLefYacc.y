%define api.prefix {Lef_}
%defines "physLefYacc.h"
%define parse.error verbose

%union {
    char * s;
}

%token <s> LEF_TOK_NAME
%token <s> LEF_TOK_STRING
%token <s> LEF_TOK_NUMBER

%token LEF_TOK_MACRO
%token LEF_TOK_LAYER
%token LEF_TOK_SITE
%token LEF_TOK_VIA
%token LEF_TOK_PIN
%token LEF_TOK_PORT
%token LEF_TOK_OBS
%token LEF_TOK_UNITS
%token LEF_TOK_PROPERTYDEFINITIONS
%token LEF_TOK_VIARULE
%token LEF_TOK_NONDEFAULTRULE
%token LEF_TOK_ARRAY
%token LEF_TOK_END

%type <s> values atom end_name

%destructor { free($$); } <s>

%{
/* The semantic actions call the static helpers in physLef.c, which
   includes this generated file at its end. */
%}

%%

/*------------------------------------------------------------------------*/
/* top level                                                              */
/*------------------------------------------------------------------------*/

lef_file:
      top_items
    ;

top_items:
      /* empty */
    | top_items top_item
    ;

top_item:
      simple_stmt
    | macro_group
    | layer_group
    | site_group
    | via_group
    | units_group
    | propdef_group
    | ignore_group
    | LEF_TOK_END end_name opt_semi           { free( $2 ); }
    | LEF_TOK_END opt_semi
    ;

/*------------------------------------------------------------------------*/
/* macro / pin / port / obs                                               */
/*------------------------------------------------------------------------*/

macro_group:
      LEF_TOK_MACRO LEF_TOK_NAME
        {
            Lef_ParsePushMacro( Lef_ParseCtx(), $2 );
            free( $2 );
        }
      macro_items
      LEF_TOK_END end_name opt_semi
        {
            Lef_ParsePopMacro( Lef_ParseCtx(), $6 );
            free( $6 );
        }
    ;

macro_items:
      /* empty */
    | macro_items macro_item
    ;

macro_item:
      simple_stmt
    | LEF_TOK_SITE values ';'
        {
            Lef_ParseSimple( Lef_ParseCtx(), "SITE", $2 );
            free( $2 );
        }
    | LEF_TOK_LAYER values ';'                { free( $2 ); }
    | LEF_TOK_VIA values ';'                  { free( $2 ); }
    | LEF_TOK_VIARULE values ';'              { free( $2 ); }
    | pin_group
    | obs_group
    ;

pin_group:
      LEF_TOK_PIN LEF_TOK_NAME
        {
            Lef_ParsePushPin( Lef_ParseCtx(), $2 );
            free( $2 );
        }
      pin_items
      LEF_TOK_END end_name opt_semi
        {
            Lef_ParsePopPin( Lef_ParseCtx(), $6 );
            free( $6 );
        }
    ;

pin_items:
      /* empty */
    | pin_items pin_item
    ;

pin_item:
      simple_stmt
    | port_group
    | LEF_TOK_LAYER values ';'                { free( $2 ); }
    | LEF_TOK_SITE values ';'                 { free( $2 ); }
    | LEF_TOK_VIA values ';'                  { free( $2 ); }
    ;

port_group:
      LEF_TOK_PORT
        {
            Lef_ParsePushPort( Lef_ParseCtx() );
        }
      port_items
      LEF_TOK_END opt_semi
        {
            Lef_ParsePopPort( Lef_ParseCtx() );
        }
    ;

port_items:
      /* empty */
    | port_items port_item
    ;

port_item:
      simple_stmt
    | LEF_TOK_LAYER values ';'                { free( $2 ); }
    | LEF_TOK_SITE values ';'                 { free( $2 ); }
    | LEF_TOK_VIA values ';'                  { free( $2 ); }
    ;

obs_group:
      LEF_TOK_OBS
        {
            Lef_ParsePushObs( Lef_ParseCtx() );
        }
      obs_items
      LEF_TOK_END opt_semi
        {
            Lef_ParsePopObs( Lef_ParseCtx() );
        }
    ;

obs_items:
      /* empty */
    | obs_items obs_item
    ;

obs_item:
      simple_stmt
    | LEF_TOK_LAYER values ';'                { free( $2 ); }
    | LEF_TOK_SITE values ';'                 { free( $2 ); }
    | LEF_TOK_VIA values ';'                  { free( $2 ); }
    ;

/*------------------------------------------------------------------------*/
/* technology groups                                                      */
/*------------------------------------------------------------------------*/

layer_group:
      LEF_TOK_LAYER LEF_TOK_NAME
        {
            Lef_ParsePushLayer( Lef_ParseCtx(), $2 );
            free( $2 );
        }
      layer_items
      LEF_TOK_END end_name opt_semi
        {
            Lef_ParsePopLayer( Lef_ParseCtx(), $6 );
            free( $6 );
        }
    ;

layer_items:
      /* empty */
    | layer_items layer_item
    ;

layer_item:
      simple_stmt
    | LEF_TOK_SITE values ';'                 { free( $2 ); }
    | LEF_TOK_VIA values ';'                  { free( $2 ); }
    ;

site_group:
      LEF_TOK_SITE LEF_TOK_NAME
        {
            Lef_ParsePushSite( Lef_ParseCtx(), $2 );
            free( $2 );
        }
      site_items
      LEF_TOK_END end_name opt_semi
        {
            Lef_ParsePopSite( Lef_ParseCtx(), $6 );
            free( $6 );
        }
    ;

site_items:
      /* empty */
    | site_items site_item
    ;

site_item:
      simple_stmt
    | LEF_TOK_VIA values ';'                  { free( $2 ); }
    ;

via_group:
      LEF_TOK_VIA LEF_TOK_NAME
        {
            Lef_ParsePushVia( Lef_ParseCtx(), $2 );
            free( $2 );
        }
      ignore_items
      LEF_TOK_END end_name opt_semi
        {
            Lef_ParsePopVia( Lef_ParseCtx(), $6 );
            free( $6 );
        }
    ;

units_group:
      LEF_TOK_UNITS
        {
            Lef_ParsePushUnits( Lef_ParseCtx() );
        }
      units_items
      LEF_TOK_END LEF_TOK_UNITS opt_semi
        {
            Lef_ParsePopUnits( Lef_ParseCtx() );
        }
    ;

units_items:
      /* empty */
    | units_items simple_stmt
    ;

propdef_group:
      LEF_TOK_PROPERTYDEFINITIONS
        {
            Lef_ParsePushIgnore( Lef_ParseCtx() );
        }
      propdef_items
      LEF_TOK_END LEF_TOK_PROPERTYDEFINITIONS opt_semi
        {
            Lef_ParsePopIgnore( Lef_ParseCtx() );
        }
    ;

propdef_items:
      /* empty */
    | propdef_items propdef_item
    ;

propdef_item:
      simple_stmt
    | propdef_kw values ';'                    { free( $2 ); }
    ;

propdef_kw:
      LEF_TOK_LAYER
    | LEF_TOK_SITE
    | LEF_TOK_VIA
    | LEF_TOK_MACRO
    | LEF_TOK_PIN
    | LEF_TOK_VIARULE
    | LEF_TOK_NONDEFAULTRULE
    | LEF_TOK_ARRAY
    ;

ignore_group:
      ignore_kw LEF_TOK_NAME
        {
            Lef_ParsePushIgnore( Lef_ParseCtx() );
            free( $2 );
        }
      ignore_items
      LEF_TOK_END end_name opt_semi
        {
            Lef_ParsePopIgnore( Lef_ParseCtx() );
            free( $6 );
        }
    ;

ignore_kw:
      LEF_TOK_VIARULE
    | LEF_TOK_NONDEFAULTRULE
    | LEF_TOK_ARRAY
    ;

ignore_items:
      /* empty */
    | ignore_items ignore_item
    ;

ignore_item:
      simple_stmt
    | LEF_TOK_LAYER LEF_TOK_NAME ';'          { free( $2 ); }
    | LEF_TOK_LAYER LEF_TOK_NAME ignore_items LEF_TOK_END end_name opt_semi
        {
            free( $2 );
            free( $5 );
        }
    | LEF_TOK_SITE values ';'                 { free( $2 ); }
    | LEF_TOK_VIA values ';'                  { free( $2 ); }
    | LEF_TOK_PIN values ';'                  { free( $2 ); }
    | LEF_TOK_MACRO values ';'                { free( $2 ); }
    | LEF_TOK_VIARULE values ';'              { free( $2 ); }
    ;

/*------------------------------------------------------------------------*/
/* statements, values, END names                                          */
/*------------------------------------------------------------------------*/

simple_stmt:
      LEF_TOK_NAME values ';'
        {
            Lef_ParseSimple( Lef_ParseCtx(), $1, $2 );
            free( $1 );
            free( $2 );
        }
    | ';'
        {
        }
    | error ';'
        {
            Lef_ParseRecover( Lef_ParseCtx() );
        }
    ;

values:
      /* empty */                             { $$ = Lef_StrDup( "" ); }
    | values atom                             { $$ = Lef_StrCat( $1, $2 ); free( $1 ); free( $2 ); }
    ;

atom:
      LEF_TOK_NAME                           { $$ = $1; }
    | LEF_TOK_NUMBER                         { $$ = $1; }
    | LEF_TOK_STRING                         { $$ = $1; }
    | '(' values ')'                          { $$ = Lef_StrWrap( "(", $2, ")" ); free( $2 ); }
    | ','                                    { $$ = Lef_StrDup( "," ); }
    | ':'                                    { $$ = Lef_StrDup( ":" ); }
    | '+'                                    { $$ = Lef_StrDup( "+" ); }
    | '-'                                    { $$ = Lef_StrDup( "-" ); }
    | '['                                    { $$ = Lef_StrDup( "[" ); }
    | ']'                                    { $$ = Lef_StrDup( "]" ); }
    | LEF_TOK_MACRO                          { $$ = Lef_StrDup( "MACRO" ); }
    | LEF_TOK_LAYER                          { $$ = Lef_StrDup( "LAYER" ); }
    | LEF_TOK_SITE                           { $$ = Lef_StrDup( "SITE" ); }
    | LEF_TOK_VIA                            { $$ = Lef_StrDup( "VIA" ); }
    | LEF_TOK_PIN                            { $$ = Lef_StrDup( "PIN" ); }
    | LEF_TOK_PORT                           { $$ = Lef_StrDup( "PORT" ); }
    | LEF_TOK_OBS                            { $$ = Lef_StrDup( "OBS" ); }
    | LEF_TOK_UNITS                          { $$ = Lef_StrDup( "UNITS" ); }
    | LEF_TOK_VIARULE                        { $$ = Lef_StrDup( "VIARULE" ); }
    | LEF_TOK_NONDEFAULTRULE                 { $$ = Lef_StrDup( "NONDEFAULTRULE" ); }
    | LEF_TOK_ARRAY                          { $$ = Lef_StrDup( "ARRAY" ); }
    ;

end_name:
      LEF_TOK_NAME                           { $$ = $1; }
    | LEF_TOK_MACRO                          { $$ = Lef_StrDup( "MACRO" ); }
    | LEF_TOK_LAYER                          { $$ = Lef_StrDup( "LAYER" ); }
    | LEF_TOK_SITE                           { $$ = Lef_StrDup( "SITE" ); }
    | LEF_TOK_VIA                            { $$ = Lef_StrDup( "VIA" ); }
    | LEF_TOK_PIN                            { $$ = Lef_StrDup( "PIN" ); }
    | LEF_TOK_PORT                           { $$ = Lef_StrDup( "PORT" ); }
    | LEF_TOK_OBS                            { $$ = Lef_StrDup( "OBS" ); }
    | LEF_TOK_UNITS                          { $$ = Lef_StrDup( "UNITS" ); }
    | LEF_TOK_PROPERTYDEFINITIONS            { $$ = Lef_StrDup( "PROPERTYDEFINITIONS" ); }
    | LEF_TOK_VIARULE                        { $$ = Lef_StrDup( "VIARULE" ); }
    | LEF_TOK_NONDEFAULTRULE                 { $$ = Lef_StrDup( "NONDEFAULTRULE" ); }
    | LEF_TOK_ARRAY                          { $$ = Lef_StrDup( "ARRAY" ); }
    | LEF_TOK_END                            { $$ = Lef_StrDup( "END" ); }
    ;

opt_semi:
      /* empty */
    | ';'
    ;

%%
