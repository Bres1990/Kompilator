%option noyywrap
%option yylineno
%{
#include "bison.tab.h"
int yylex();

%}
%x comment

%%

"{"			BEGIN(comment);
<comment>[^}\n]*        ;
<comment>\n             ;
<comment>"}"        BEGIN(INITIAL);

[ \t\r\n]+	{} 	
VAR				{ return(VAR); }
BEGIN			{ return(START); }
ENDIF			{ return(ENDIF); }
ENDFOR			{ return(ENDFOR); }
ENDWHILE		{ return(ENDWHILE); } 
END				{ return(END); }
":="			{ return(ASSIGN); }
WHILE			{ return(WHILE); }
DOWNTO			{ return(DOWNTO); }
DO				{ return(DO); }
FOR 			{ return(FOR); }
FROM 			{ return(FROM); }
TO 				{ return(TO); }
IF				{ return(IF); }
THEN			{ return(THEN); }
ELSE			{ return(ELSE); }
WRITE			{ return(WRITE); }
READ			{ return(READ); }
SKIP			{ return(SKIP); }
"=="			{ return(EQ); } 
"<"				{ return(LT); }
"<="			{ return(LET); }
">"				{ return(GT); }
">="			{ return(GET); }
"<>"			{ return(NEQ); }
"+"				{ return(PLUS); }
"-"				{ return(MINUS); }
"*"				{ return(MULT); }
"/"				{ return(DIV); }
"%"				{ return(MOD); }
"["				{ return(LBRACKET); }
"]"				{ return(RBRACKET); }
[0-9]+	{    
	     yylval.numu = (char *) strdup(yytext);
				return(NUM);
	}
[_a-z]+	{ 
	     yylval.stru = (char *) strdup(yytext);
				return(PIDENTIFIER);  
	}
";"				{ return(SEMICOLON); }
.	;
%%
