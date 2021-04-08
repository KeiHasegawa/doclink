%{
#pragma warning (disable : 4005)
#ifdef _MSC_VER
#define YY_NO_UNISTD_H
#define isatty _isatty
#define fileno _fileno
#define strdup _strdup
extern "C" int isatty(int);
#endif // _MSC_VER

#include "doclink_y.h"

#include "doclink.h"
%}

identifier  {nondigit}({nondigit}|{digit}|:)*
nondigit    [_a-zA-Z]
digit       [0-9]

number      {digit}{digit}*

string      \"{char-seq}?\"
char-seq    {char}+
char        [^\"\\\n\r]

%%

\$def       { return DEF_KW; }
\$decl      { return DECL_KW; }
\$func      { return FUNC_KW; }
\$graph     { return GRAPH_KW; }
\$macro     { return MACRO_KW; }
\$ref       { return REF_KW; }
\$tag       { return TAG_KW; }
\$type      { return TYPE_KW; }
\$var       { return VAR_KW; }

{identifier}  {
                  doclink_lval.m_str = strdup(doclink_text);
                  return IDENTIFIER;
              }
{number}      {
                  doclink_lval.m_ival = strtol(doclink_text,0,0);
                  return NUMBER;
              }
{string}      {
                  doclink_lval.m_str = strdup(doclink_text);
                  return STRING;
              }

\/\/.*      { }
[ \t]       { }
\n|\r\n     { ++doclink::lineno; }
.           { return yytext[0]; }

%%

int yywrap(){ return 1; }
