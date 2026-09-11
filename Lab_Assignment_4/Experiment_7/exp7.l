%{
#include "y.tab.h"
#include <string.h>
%}

%%
"int"               { return INT; }
"float"             { return FLOAT; }
"char"              { return CHAR; }

[0-9]+\.[0-9]+      { return FLOAT_NUM; }
[0-9]+              { return INT_NUM; }
\'[a-zA-Z0-9]\'     { return CHAR_CONST; }

[a-zA-Z_][a-zA-Z0-9_]* {
    strncpy(yylval.id, yytext, 31);
    yylval.id[31] = '\0';
    return ID;
}

[ \t\r\n]+          { /* ignore whitespace */ }
.                   { return yytext[0]; }
%%

int yywrap() {
    return 1;
}
