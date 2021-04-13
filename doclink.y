%{
int doclink_lex();
#include "doclink.h"
%}

%token MACRO_KW DEF_KW REF_KW TAG_KW VAR_KW DECL_KW FUNC_KW GRAPH_KW TYPE_KW
%token IDENTIFIER STRING NUMBER

%union {
  int m_ival;
  char* m_str;
}
%type<m_ival> NUMBER
%type<m_str> IDENTIFIER STRING
%%

statements
  : statement
  | statements statement
  ;

statement
  : macro_stmt
  | tag_stmt
  | var_stmt
  | func_stmt
  | type_stmt
  ;

macro_stmt
  : MACRO_KW DEF_KW IDENTIFIER STRING NUMBER ';'
    { doclink::macro::def($3,$4,$5); }
  | MACRO_KW REF_KW IDENTIFIER STRING NUMBER ';'
    { doclink::macro::ref($3,$4,$5); }
  ;

tag_stmt
  : TAG_KW DECL_KW IDENTIFIER STRING NUMBER ';'
    { doclink::tag::decl($3,$4,$5); }
  | TAG_KW DECL_KW STRING  STRING NUMBER ';'
    { doclink::tag::decl($3,$4,$5); }
  ;

var_stmt
  : VAR_KW DEF_KW IDENTIFIER STRING NUMBER ';'
    { doclink::var::def($3,$4,$5); }
  | VAR_KW DEF_KW STRING STRING NUMBER ';'
    { doclink::var::def($3,$4,$5); }
  | VAR_KW DECL_KW IDENTIFIER STRING NUMBER ';'
    { doclink::var::decl($3,$4,$5); }
  | VAR_KW DECL_KW STRING STRING NUMBER ';'
    { doclink::var::decl($3,$4,$5); }
  | VAR_KW DEF_KW IDENTIFIER STRING NUMBER
    { doclink::var::def($3,$4,$5); } '{' ref_stmts '}'
  | VAR_KW DEF_KW STRING STRING NUMBER
    { doclink::var::def($3,$4,$5); } '{' ref_stmts '}'
  ;

func_stmt
  : FUNC_KW DECL_KW IDENTIFIER STRING NUMBER ';'
    { doclink::func::decl($3,$4,$5); }
  | FUNC_KW DECL_KW STRING STRING NUMBER ';'
    { doclink::func::decl($3,$4,$5); }
  | func_def_header '{' graph '}'
  | func_def_header '{' '}'
  | func_def_header '{' ref_stmts graph '}'
  | func_def_header '{' ref_stmts '}'
  ;

func_def_header
  : FUNC_KW DEF_KW IDENTIFIER STRING NUMBER
    { doclink::func::def($3,$4,$5); }
  | FUNC_KW DEF_KW STRING STRING NUMBER
    { doclink::func::def($3,$4,$5); }
  ;

type_stmt
  : TYPE_KW DEF_KW IDENTIFIER STRING NUMBER ';'
    { doclink::type::def($3,$4,$5); }
  | TYPE_KW DEF_KW STRING STRING NUMBER ';'
    { doclink::type::def($3,$4,$5); }
  | TYPE_KW REF_KW IDENTIFIER STRING NUMBER ';'
    { doclink::type::ref($3,$4,$5); }
  | TYPE_KW REF_KW STRING STRING NUMBER ';'
    { doclink::type::ref($3,$4,$5); }
  ;

ref_stmts
  : ref_stmt
  | ref_stmts ref_stmt
  ;

ref_stmt
  : REF_KW TAG_KW IDENTIFIER STRING NUMBER ';'
    { doclink::tag::ref($3,$4,$5); }
  | REF_KW TAG_KW STRING STRING NUMBER ';'
    { doclink::tag::ref($3,$4,$5); }
  | REF_KW VAR_KW IDENTIFIER STRING NUMBER ';'
    { doclink::var::ref($3,$4,$5); }
  | REF_KW VAR_KW STRING STRING NUMBER ';'
    { doclink::var::ref($3,$4,$5); }
  | REF_KW FUNC_KW IDENTIFIER STRING NUMBER ';'
    { doclink::func::ref($3,$4,$5); }
  | REF_KW FUNC_KW STRING STRING NUMBER ';'
    { doclink::func::ref($3,$4,$5); }
  ;

graph
  : GRAPH_KW '{' '}'
  | GRAPH_KW '{' graph_stmts '}'
  ;

graph_stmts
 : graph_stmt
 | graph_stmts graph_stmt
 ;

graph_stmt
 : vertex
 | edge
 ;

vertex
 : NUMBER STRING NUMBER ';'
   { doclink::func::vertex($1, $2, $3); } 
 ;

edge
 : '(' NUMBER ',' NUMBER ')' ';'
   { doclink::func::edge($2, $4); }
 ;

%%

extern char* doclink_text;

void doclink_error(const char* msg)
{
  using namespace std;
  using namespace doclink;
  cerr << curr_file << ':' << lineno << ':' << '\n';
  cerr << msg << ' ' << '`' << doclink_text << "'" << '\n';
  ++error::counter;
}
