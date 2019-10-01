
%{

#include <iostream>
#include <cstdlib>

using namespace std;

%}

%%
  /*
    Pattern definitions for all tokens
  */
func                       { return 1; }
int                        { return 2; }
package                    { return 3; }
extern                     { return 4; }
void                       { return 5; }
string                     { return 6; }
var                        { return 7; }
\=                         { return 8; }
if                         { return 9; }
return 					   { return 10; }
else                       { return 11; }
\;						   { return 12; }
\{				   		   { return 13; }
\}				           { return 14; }
\(				           { return 15; }
\)				           { return 16; }
\'[ -&(-[\]-~]\'		   { return 17; }
\'\\[abntrvf\\'"]\'        { return 18; } 
\"([\x7-\x9\xB-\xD\x20\x21\x23-\x5B\x5D-\x7E]*([\\][nrtvfab\'\"\\])*)*\"  { return 19; }
\-						   { return 20; }
([0-9]+|[0]+[xX][0-8]+[a-fA-F]*)  { return 21; }
[a-zA-Z\_][a-zA-Z\_0-9]*   { return 22; }
\=\=  					   { return 23; }
\%						   { return 24; }
\,						   { return 25; }
\/\/[\x7-\x9\xB-\xD\x20-\x7E]*\n   { return 26; }
[\t\r\a\v\b\f\n ]+		   { return 27; }
\n                         { return 28; }
.                          { cerr << "Error: unexpected character in input" << endl; return -1; }

%%

int main () {
  int token;
  string lexeme;
  string concatenatedWhitespaces; 
  while ((token = yylex())) {
    if (token > 0) {
      lexeme.assign(yytext);
      switch(token) {
        case 1: cout << "T_FUNC " << lexeme << endl; break;
        case 2: cout << "T_INTTYPE " << lexeme << endl; break;
        case 3: cout << "T_PACKAGE " << lexeme << endl; break;
        case 4: cout << "T_EXTERN " << lexeme << endl; break;
        case 5: cout << "T_VOID " << lexeme << endl; break;
        case 6: cout << "T_STRINGTYPE " << lexeme << endl; break;
        case 7: cout << "T_VAR " << lexeme << endl; break;
        case 8: cout << "T_ASSIGN " << lexeme << endl; break;
        case 9: cout << "T_IF " << lexeme << endl; break;
        case 11: cout << "T_ELSE " << lexeme << endl; break;
        case 17: cout << "T_CHARCONSTANT " << lexeme << endl; break;
        case 18: cout << "T_CHARCONSTANT " << lexeme << endl; break; 
        case 19: cout << "T_STRINGCONSTANT " << lexeme << endl; break;
        case 12: cout << "T_SEMICOLON " << lexeme << endl; break; 
        case 13: cout << "T_LCB " << lexeme << endl; break; 
        case 14: cout << "T_RCB " << lexeme << endl; break; 
        case 15: cout << "T_LPAREN " << lexeme << endl; break; 
        case 16: cout << "T_RPAREN " << lexeme << endl; break; 
        case 10: cout << "T_RETURN " << lexeme << endl; break; 
        case 20: cout << "T_MINUS " << lexeme << endl; break; 
        case 21: cout << "T_INTCONSTANT " << lexeme << endl; break; 
        case 22: cout << "T_ID " << lexeme << endl; break; 
        case 23: cout << "T_EQ " << lexeme << endl; break; 
        case 24: cout << "T_MOD " << lexeme << endl; break; 
        case 25: cout << "T_COMMA " << lexeme << endl; break; 
        case 26: cout << "T_COMMENT " ;
              lexeme.replace(lexeme.find('\n'),1,"\\n");
              cout << lexeme << endl; break; 
        case 27: cout << "T_WHITESPACE " ;
        			for(int i=0; i < lexeme.length(); i++){
        				if(lexeme[i]== '\n') cout << "\\n";
        				else cout << lexeme[i];	
        			} 
        		 cout << endl; break; 
        case 28: cout << "T_WHITESPACE \\n " << endl; break;		 
        default: exit(EXIT_FAILURE);
      }
    } else {
      if (token < 0) {
        exit(EXIT_FAILURE);
      }
    }
  }
  exit(EXIT_SUCCESS);
}
