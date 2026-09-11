%{
#include "y.tab.h"
%}

%%
"("     { return '('; }
")"     { return ')'; }
"["     { return '['; }
"]"     { return ']'; }
"{"     { return '{'; }
"}"     { return '}'; }
\n      { return '\n'; }
[ \t]+  { /* ignore spaces and tabs */ }
.       { /* ignore variables, identifiers, numbers, operators */ }
%%

int yywrap() {
    return 1;
}
