%define api.prefix {Lef_}
%defines "physLefYacc.h"
%define parse.error verbose

%union {
    char * s;
    double d;
}

%token <s> LEF_TOK_NAME
%token <s> LEF_TOK_STRING
%token <d> LEF_TOK_NUMBER

/* group keywords */
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

/* statement keywords */
%token LEF_TOK_CLASS
%token LEF_TOK_ORIGIN
%token LEF_TOK_SIZE
%token LEF_TOK_BY
%token LEF_TOK_DIRECTION
%token LEF_TOK_USE
%token LEF_TOK_RECT
%token LEF_TOK_MASK
%token LEF_TOK_WIDTH
%token LEF_TOK_PITCH
%token LEF_TOK_RESISTANCE
%token LEF_TOK_CAPACITANCE
%token LEF_TOK_EDGECAPACITANCE
%token LEF_TOK_TYPE
%token LEF_TOK_DATABASE
%token LEF_TOK_MANUFACTURINGGRID

%type <s> values atom word end_name
%type <d> number

%destructor { free($$); } <s>

%{
/* The semantic actions call the static helpers in physLef.c, which
   includes this generated file at its end.  The lexer classifies the
   tokens: keywords arrive as distinct terminals, numbers arrive as
   doubles, and everything else as names or quoted strings.  The grammar
   below gives the recognized statements an explicit shape and calls a
   typed handler; statements whose keyword is not part of the supported
   subset reduce to `word values ;' and are skipped. */
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
      stmt
    | macro_group
    | layer_group
    | site_group
    | via_group
    | units_group
    | propdef_group
    | ignore_group
    | LEF_TOK_END end_name opt_semi           { free( $2 ); }
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
      stmt
    | LEF_TOK_SITE word ';'
        {
            Lef_ParseSiteRef( Lef_ParseCtx(), $2 );
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
      stmt
    | LEF_TOK_SITE word ';'                   { free( $2 ); }
    | LEF_TOK_LAYER values ';'                { free( $2 ); }
    | LEF_TOK_VIA values ';'                  { free( $2 ); }
    | port_group
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
      stmt
    | LEF_TOK_SITE word ';'                   { free( $2 ); }
    | LEF_TOK_LAYER values ';'                { free( $2 ); }
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
      stmt
    | LEF_TOK_SITE word ';'                   { free( $2 ); }
    | LEF_TOK_LAYER values ';'                { free( $2 ); }
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
      stmt
    | LEF_TOK_SITE word ';'                   { free( $2 ); }
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
      stmt
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
    | units_items stmt
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
      stmt
    | propdef_kw values ';'                   { free( $2 ); }
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
      stmt
    | LEF_TOK_LAYER LEF_TOK_NAME ';'          { free( $2 ); }
    | LEF_TOK_LAYER LEF_TOK_NAME ignore_items LEF_TOK_END end_name opt_semi
        {
            free( $2 );
            free( $5 );
        }
    | LEF_TOK_SITE word ';'                   { free( $2 ); }
    | LEF_TOK_VIA values ';'                  { free( $2 ); }
    | LEF_TOK_PIN values ';'                  { free( $2 ); }
    | LEF_TOK_MACRO values ';'                { free( $2 ); }
    | LEF_TOK_VIARULE values ';'              { free( $2 ); }
    ;

/*------------------------------------------------------------------------*/
/* recognized statements                                                  */
/*------------------------------------------------------------------------*/

stmt:
      LEF_TOK_NAME values ';'
        {
            free( $1 );
            free( $2 );
        }
    | keyword_stmt
    | error ';'
        {
            Lef_ParseRecover( Lef_ParseCtx() );
        }
    ;

keyword_stmt:
      LEF_TOK_CLASS values ';'
        {
            Lef_ParseClass( Lef_ParseCtx(), $2 );
            free( $2 );
        }
    | LEF_TOK_ORIGIN number number ';'
        {
            Lef_ParseOrigin( Lef_ParseCtx(), $2, $3 );
        }
    | LEF_TOK_SIZE number LEF_TOK_BY number ';'
        {
            Lef_ParseSize( Lef_ParseCtx(), $2, $4 );
        }
    | LEF_TOK_SIZE number number ';'
        {
            Lef_ParseSize( Lef_ParseCtx(), $2, $3 );
        }
    | LEF_TOK_DIRECTION word values ';'
        {
            Lef_ParseDirection( Lef_ParseCtx(), $2 );
            free( $2 );
            free( $3 );
        }
    | LEF_TOK_USE word values ';'
        {
            Lef_ParseUse( Lef_ParseCtx(), $2 );
            free( $2 );
            free( $3 );
        }
    | LEF_TOK_TYPE word ';'
        {
            Lef_ParseLayerType( Lef_ParseCtx(), $2 );
            free( $2 );
        }
    | LEF_TOK_WIDTH number ';'
        {
            Lef_ParseWidth( Lef_ParseCtx(), $2 );
        }
    | LEF_TOK_PITCH number number ';'
        {
            Lef_ParsePitch( Lef_ParseCtx(), $2, $3, 1 );
        }
    | LEF_TOK_PITCH number ';'
        {
            Lef_ParsePitch( Lef_ParseCtx(), $2, $2, 0 );
        }
    | LEF_TOK_RESISTANCE number ';'
        {
            Lef_ParseResistance( Lef_ParseCtx(), NULL, $2 );
        }
    | LEF_TOK_RESISTANCE word number ';'
        {
            Lef_ParseResistance( Lef_ParseCtx(), $2, $3 );
            free( $2 );
        }
    | LEF_TOK_CAPACITANCE number ';'
        {
            Lef_ParseCapacitance( Lef_ParseCtx(), NULL, $2 );
        }
    | LEF_TOK_CAPACITANCE word number ';'
        {
            Lef_ParseCapacitance( Lef_ParseCtx(), $2, $3 );
            free( $2 );
        }
    | LEF_TOK_EDGECAPACITANCE number ';'
        {
            Lef_ParseEdgeCapacitance( Lef_ParseCtx(), $2 );
        }
    | LEF_TOK_EDGECAPACITANCE word number ';'
        {
            Lef_ParseEdgeCapacitance( Lef_ParseCtx(), $3 );
            free( $2 );
        }
    | LEF_TOK_DATABASE word number ';'
        {
            Lef_ParseDatabase( Lef_ParseCtx(), $2, $3 );
            free( $2 );
        }
    | LEF_TOK_MANUFACTURINGGRID number ';'
        {
            Lef_ParseGrid( Lef_ParseCtx(), $2 );
        }
    | LEF_TOK_RECT '(' number number number number ')' ';'
        {
            Lef_ParseRect( Lef_ParseCtx(), $3, $4, $5, $6 );
        }
    | LEF_TOK_RECT '(' number number ')' ';'
        {
            Lef_ParseRectPoint( Lef_ParseCtx(), $3, $4 );
        }
    | LEF_TOK_RECT number number number number ';'
        {
            Lef_ParseRect( Lef_ParseCtx(), $2, $3, $4, $5 );
        }
    | LEF_TOK_RECT number number ';'
        {
            Lef_ParseRectPoint( Lef_ParseCtx(), $2, $3 );
        }
    | LEF_TOK_RECT LEF_TOK_MASK number '(' number number number number ')' ';'
        {
            Lef_ParseRect( Lef_ParseCtx(), $5, $6, $7, $8 );
        }
    | LEF_TOK_RECT LEF_TOK_MASK number '(' number number ')' ';'
        {
            Lef_ParseRectPoint( Lef_ParseCtx(), $5, $6 );
        }
    ;

/*------------------------------------------------------------------------*/
/* values, words, numbers, END names                                      */
/*------------------------------------------------------------------------*/

number:
      LEF_TOK_NUMBER                           { $$ = $1; }
    ;

values:
      /* empty */                             { $$ = Lef_StrDup( "" ); }
    | values atom                             { $$ = Lef_StrCat( $1, $2 ); free( $1 ); free( $2 ); }
    ;

atom:
      word                                    { $$ = $1; }
    | LEF_TOK_STRING                          { $$ = $1; }
    | LEF_TOK_NUMBER                          { $$ = Lef_StrNum( $1 ); }
    | '(' values ')'                          { $$ = Lef_StrWrap( "(", $2, ")" ); free( $2 ); }
    | ','                                     { $$ = Lef_StrDup( "," ); }
    | ':'                                     { $$ = Lef_StrDup( ":" ); }
    | '['                                     { $$ = Lef_StrDup( "[" ); }
    | ']'                                     { $$ = Lef_StrDup( "]" ); }
    ;

word:
      LEF_TOK_NAME                            { $$ = $1; }
    | LEF_TOK_MACRO                           { $$ = Lef_StrDup( "MACRO" ); }
    | LEF_TOK_LAYER                           { $$ = Lef_StrDup( "LAYER" ); }
    | LEF_TOK_SITE                            { $$ = Lef_StrDup( "SITE" ); }
    | LEF_TOK_VIA                             { $$ = Lef_StrDup( "VIA" ); }
    | LEF_TOK_PIN                             { $$ = Lef_StrDup( "PIN" ); }
    | LEF_TOK_PORT                            { $$ = Lef_StrDup( "PORT" ); }
    | LEF_TOK_OBS                             { $$ = Lef_StrDup( "OBS" ); }
    | LEF_TOK_UNITS                           { $$ = Lef_StrDup( "UNITS" ); }
    | LEF_TOK_PROPERTYDEFINITIONS             { $$ = Lef_StrDup( "PROPERTYDEFINITIONS" ); }
    | LEF_TOK_VIARULE                         { $$ = Lef_StrDup( "VIARULE" ); }
    | LEF_TOK_NONDEFAULTRULE                  { $$ = Lef_StrDup( "NONDEFAULTRULE" ); }
    | LEF_TOK_ARRAY                           { $$ = Lef_StrDup( "ARRAY" ); }
    | LEF_TOK_END                             { $$ = Lef_StrDup( "END" ); }
    | LEF_TOK_CLASS                           { $$ = Lef_StrDup( "CLASS" ); }
    | LEF_TOK_ORIGIN                          { $$ = Lef_StrDup( "ORIGIN" ); }
    | LEF_TOK_SIZE                            { $$ = Lef_StrDup( "SIZE" ); }
    | LEF_TOK_BY                              { $$ = Lef_StrDup( "BY" ); }
    | LEF_TOK_DIRECTION                       { $$ = Lef_StrDup( "DIRECTION" ); }
    | LEF_TOK_USE                             { $$ = Lef_StrDup( "USE" ); }
    | LEF_TOK_RECT                            { $$ = Lef_StrDup( "RECT" ); }
    | LEF_TOK_MASK                            { $$ = Lef_StrDup( "MASK" ); }
    | LEF_TOK_WIDTH                           { $$ = Lef_StrDup( "WIDTH" ); }
    | LEF_TOK_PITCH                           { $$ = Lef_StrDup( "PITCH" ); }
    | LEF_TOK_RESISTANCE                      { $$ = Lef_StrDup( "RESISTANCE" ); }
    | LEF_TOK_CAPACITANCE                     { $$ = Lef_StrDup( "CAPACITANCE" ); }
    | LEF_TOK_EDGECAPACITANCE                 { $$ = Lef_StrDup( "EDGECAPACITANCE" ); }
    | LEF_TOK_TYPE                            { $$ = Lef_StrDup( "TYPE" ); }
    | LEF_TOK_DATABASE                        { $$ = Lef_StrDup( "DATABASE" ); }
    | LEF_TOK_MANUFACTURINGGRID               { $$ = Lef_StrDup( "MANUFACTURINGGRID" ); }
    ;

end_name:
      LEF_TOK_NAME                            { $$ = $1; }
    ;

opt_semi:
      /* empty */
    | ';'
    ;

%%
