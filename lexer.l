%option noyywrap
%option nounput
%option noinput

%{
#include <iostream>
#include <cstdlib>
using namespace std;
#include "ast.hpp"
#include "parser.tab.hpp"
%}

%%
def                     {return def_token;}
extern                  {return extern_token;}
if                      {return if_token;}
then                    {return then_token;}
else                    {return else_token;}
for                     {return for_token;}
in                      {return in_token;}
var                     {return var_token;}
[#].*                   { }
[a-zA-Z_][a-zA-Z_0-9]*  {
    yylval.s = new string(yytext);
    return id_token;
}
[0-9]+(\.[0-9]*)?       {
    yylval.d = atof(yytext);
    return num_token;
}
"=="                    {return poredjenje_token;}
[()+*/;,:<>=!|-]               {return *yytext;}
[ \n\t]                 { }
.                       {
    cerr << "Leksicka greska: " << yytext << endl;
    exit(EXIT_FAILURE);
}
%%
