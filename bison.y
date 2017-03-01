%{

#include <stdio.h>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream> 

int yylex(void);
extern int yylineno; 

int yyerror(const char *error) 
{ 
    printf("ERROR: Line %d - %s\n", yylineno, error ); 
}   

void catch_error(int line, const char* err);     
void err(int line, int no); 

#include "translate.c" 

int forPlaceholder = -1;
int forIterator = -1;
int iteratorRegister = -1; // register of a protected variable

%}
 
%union{ char *stru; char *numu; } 
%token <stru> VAR /* blok deklaracji zmiennych */
%token <stru> START END /* blok kodu */
%token <stru> LBRACKET RBRACKET  
%token <stru> LCOMMENT RCOMMENT  
%token <stru> IF THEN ELSE ENDIF  
%token <stru> WHILE DO ENDWHILE
%token <stru> FOR FROM DOWNTO TO ENDFOR       
%token <stru> READ WRITE
%token <stru> SKIP 
%token <stru> ASSIGN  
%token <stru> PLUS MINUS MULT DIV MOD
%token <stru> EQ LT LET GT GET NEQ  
%token <numu> NUM 
%token <stru> PIDENTIFIER IDENTIFIER
%token <stru> SEMICOLON

%type<numu> expression condition   

%%
program : 	| 
		VAR vdeclarations START commands END     
		{ 
		    endOfProgram();  
		} 
;

vdeclarations : vdeclarations PIDENTIFIER   
		{
		    int result = declareVariable($<stru>2); 
			switch (result) {      
				case 1: 
					{  
					stri err = "Redeklaracja zmiennej ";
					err += $<stru>2;
					catch_error(yylineno, err.c_str());
					}
					break;       
				case 0: 
					if (DEBUG) printf("\tUdana deklaracja zmiennej\n");
					if (DEBUG) printf("\tZmienna przypisana do rejestru %d\n", registerManager.findFreeRegister()-1);
					break;
				case -1:
					stri err = "Zmienna "; 
					err += $<stru>2;
					err += " musi miec nazwe\n"; 
					catch_error(yylineno, err.c_str());
					break;
			}
		}  
		| vdeclarations PIDENTIFIER LBRACKET NUM RBRACKET  
		{  
		    int result = arrayManager.declareArray($<stru>2, $<stru>4);  
			switch (result) {
				case 1: 
					{
					stri err = "Redeklaracja tablicy ";
					err += $<stru>2; 
					err += $<stru>3;
					err += $<numu>4;
					err += $<stru>5;
					catch_error(yylineno, err.c_str());   
					} 
					break; 
				case 0:
					if (DEBUG) printf("\tUdana deklaracja tablicy\n");  
					break; 
				case -1: 
					{
					stri err = "Tablica ";
					err += $<stru>2;
					err += " musi miec nazwe\n";
					catch_error(yylineno, err.c_str());
					}
					break;
			}
		}   
		| { if (DEBUG) printf("\n\n****** BLOK DEKLARACJI ZMIENNYCH ******\n\n\n"); }
;

commands : commands command 
      	| { if (DEBUG) printf("\n\n****** BLOK KOMENDY ******\n\n\n"); }
; 
 
command : PIDENTIFIER LBRACKET PIDENTIFIER RBRACKET ASSIGN expression SEMICOLON
	{  
		if (arrayManager.findArray($<stru>1) == -1) { // check if array exists
			stri err = "Niezadeklarowana tablica";
			err += $<stru>1;  
			err += $<stru>2;
			err += $<stru>3;
			err += $<stru>4;
			catch_error(yylineno, err.c_str());
		} else { 	// assign value to array index
			arrayManager.setArrayIndexValue($<stru>1, $<stru>3, $<stru>6);
			if (DEBUG) printf("\tUdane przypisanie do indeksu tablicy");
		}
	}       

	| PIDENTIFIER LBRACKET NUM RBRACKET ASSIGN expression SEMICOLON
	{
		if (arrayManager.findArray($<stru>1) == -1) { // check if array exists
			stri err = "Niezadeklarowana tablica";
			err += $<stru>1; 
			err += $<stru>2;
			err += $<stru>3;
			err += $<stru>4;
			catch_error(yylineno, err.c_str());
		} else { 	// assign value to array index
			if (DEBUG)
			arrayManager.setArrayIndexValue($<stru>1, $<stru>3, $<stru>6);
			if (DEBUG) printf("\tUdane przypisanie do indeksu tablicy");
		}
	}

	| PIDENTIFIER ASSIGN expression SEMICOLON
	{
	    int result = generateVariableAssign($<stru>1, $<stru>3);  

		switch (result) {
			case 2: 
				{
					stri err = "Użycie niezainicjalizowanej zmiennej ";
					err += $<stru>3;
					catch_error(yylineno, err.c_str());
				}
				break;
			case 1:
				{
					stri err = "NIEZADEKLAROWANA zmienna ";
					err += $<stru>1;
					catch_error(yylineno, err.c_str());
				}
				break;
			case 0:
				if (DEBUG) printf("\tUdane przypisanie do zmiennej\n"); 
				break; 
		}  
	}
  
    | IF condition
	{ 
		registerManager.setValueToRegister($<stru>2, 0);
		generateIf();
		if (DEBUG) printf("Obsluga if \n");
	} 
	THEN commands
	{        
		generateThen(); 
		if (DEBUG) printf("Obsluga then \n");    
	} 
	ELSE commands 
	{
		generateElse();
		if (DEBUG) printf("Obsluga else \n");
	} ENDIF {} 
 
    | WHILE condition      
	{ 
		registerManager.setValueToRegister($<stru>2, 0);
	    generateWhile();  
		if (DEBUG)printf("Obsluga while\n");   
	}	
	DO commands  
	{	
	    if (DEBUG)printf("Condition do while\n");  
	} ENDWHILE {} 

	| FOR PIDENTIFIER 
	{ 
		if (variableManager.getItemIndex($<stru>2) == -1) {
			declareVariable($<stru>2);  
			registerManager.populateRegister($<stru>2); /////     
			//printf("iterator variableManager index: %d\n", variableManager.getItemIndex($<stru>2));
			//printf("iterator register: %d\n", getVariableRegister($<stru>2)); 
			stri var = ""; 
			var =+ $<stru>2;        
			if (DEBUG) printf("FOR: Variable %s declared\n", var.c_str());
			iteratorRegister = getVariableRegister($<stru>2);
		} else {
			stri err = "Uzycie zmiennej globalnej o tej samej nazwie co iterator w zakresie petli: <";
			err += $<stru>2;
			err += ">";
			catch_error(yylineno, err.c_str());
		} 
	}
	FROM VALUE DOWNTO VALUE {                    
		stri val1 = "";
		val1 += $<stru>5;
		stri val2 = "";
		val2 += $<stru>7;

		//if (DEBUG) printf("values: %s %s\n", val1.c_str(), val2.c_str());

		int result = variableManager.getItemIndex($<stru>2);
		switch(result) {
			case -1: 
			{
				stri err = "Niezadeklarowana zmienna ";
				err += $<stru>2;
				catch_error(yylineno, err.c_str());
				break;
			}
			default:
			{
				if (DEBUG) printf("Obsluga for\n");
				forPlaceholder = generateFor($<stru>2, $<stru>5, $<stru>7, false); //
				if (DEBUG) printf("Skonczylem obsluge for\n");
				break;
			}
		}
	} 
	DO commands
	{ 
		generateDowntoDo(forPlaceholder);
		if (DEBUG) printf("Obsluga downto-do\n");
	} ENDFOR { registerManager.removeFromRegister(iteratorRegister+1); variableManager.deleteVariable($<stru>2); iteratorRegister = -1; }

	| FOR PIDENTIFIER 
	{   
		if (variableManager.getItemIndex($<stru>2) == -1) {
			declareVariable($<stru>2);
			printf("iterator variableManager index: %d\n", variableManager.getItemIndex($<stru>2));
			//printf("iterator register: %d\n", getVariableRegister($<stru>2)); 
			stri var = ""; 
			var =+ $<stru>2;
			if (DEBUG) printf("FOR: Variable %s declared\n", var.c_str());
			iteratorRegister = getVariableRegister($<stru>2);
		} else {
			stri err = "Uzycie zmiennej globalnej o tej samej nazwie co iterator w zakresie petli: <";
			err += $<stru>2;
			err += ">";
			catch_error(yylineno, err.c_str());
		} 

	} 
	FROM VALUE TO VALUE 
	{
		int result = variableManager.getItemIndex($<stru>2);

		stri val1 = "";
		val1 += $<stru>5;
		stri val2 = "";
		val2 += $<stru>7;  

		//if (DEBUG) printf("values: %s %s\n", val1.c_str(), val2.c_str());
		switch(result) {
			case -1:
			{
				stri err = "Niezadeklarowana zmienna ";
				err += $<stru>2;
				catch_error(yylineno, err.c_str()); 
				break;
			}
			default:
			{
				if (DEBUG) printf("Obsluga for\n");
				forPlaceholder = generateFor($<stru>2, $<stru>5, $<stru>7, true);
				break;
			}
		}
	}  
	DO commands        
	{
		generateToDo(forPlaceholder);
		if (DEBUG) printf("Obsluga to-do\n");
	} ENDFOR { registerManager.removeFromRegister(iteratorRegister); variableManager.deleteVariable($<stru>2); iteratorRegister = -1; }


    | READ PIDENTIFIER LBRACKET PIDENTIFIER RBRACKET SEMICOLON
	{
	    int result = generateRead($<stru>2);
		if (result != 0) {  
			err(yylineno, result);  
		}
	}

	| READ PIDENTIFIER LBRACKET NUM RBRACKET SEMICOLON 
	{ 
	    int result = generateRead($<stru>2);
		if (result != 0) {   
			err(yylineno, result);
		} 
	}    

	| READ PIDENTIFIER SEMICOLON
	{
	    int result = generateRead($<stru>2);
		if (result != 0) {
			err(yylineno, result);      
		}
	}

    | WRITE VALUE SEMICOLON  
	{ 
	    int result = generateWrite($<stru>2);
		switch(result) { 
		case 666: 
					{
					stri err = "Niezainicjalizowana zmienna ";
					err += $<stru>2;
					catch_error(yylineno, err.c_str());
					}
					break;
		case 0: 
					if (DEBUG) printf("\tUdane wypisanie zmiennej\n");
					break;     
		}  
	}

	| SKIP SEMICOLON {}  
;
	 
expression: VALUE
			{	
				if (DEBUG) printf("Generuje wartosc %s\n", $<stru>1);     
				
				int result = generateP_A($<stru>1);
				if (result) { err(yylineno, result); }
			}
			| VALUE PLUS VALUE       
			{
				if (DEBUG) printf("Operacja arytmetyczna %s + %s\n", $<stru>1, $<stru>3);
				int result = generateAddition($<stru>1, $<stru>3); 
				if (result == -1) { err(yylineno, result); } 
 
				std::ostringstream os;
				os << result;
				$$ = strdup(os.str().c_str());
			}
			| VALUE MINUS VALUE  
			{
				if (DEBUG) printf("Operacja arytmetyczna %s - %s\n", $<stru>1, $<stru>3);
				int result = generateSubtraction($<stru>1, $<stru>3);
				if (result == -1) { err(yylineno, result); }

				std::ostringstream os;
				os << result;
				$$ = strdup(os.str().c_str());
			}
			| VALUE MULT VALUE  
			{
				if (DEBUG) printf("Operacja arytmetyczna %s * %s\n", $<stru>1, $<stru>3);
				int result = generateMultiplication($<stru>1, $<stru>3);
				if (result == -1) { err(yylineno, result); } 

				std::ostringstream os;
				os << result;
				$$ = strdup(os.str().c_str());
			}
			| VALUE DIV VALUE
			{
				if (DEBUG) printf("Operacja arytmetyczna %s / %s\n", $<stru>1, $<stru>3);
				int result = generateDivision($<stru>1, $<stru>3);
				if (result == -1) { err(yylineno, result); }

				std::ostringstream os;
				os << result; 
				$$ = strdup(os.str().c_str());
			}
			| VALUE MOD VALUE 
			{
				if (DEBUG) printf("Operacja arytmetyczna %s % %s\n", $<stru>1, $<stru>3);
				int result = generateModulo($<stru>1, $<stru>3);
				if (result == -1) { err(yylineno, result); }

				std::ostringstream os;
				os << result; 
				$$ = strdup(os.str().c_str());
			} 
			
	;


	
condition :  VALUE
	{    
		int result = generateP_A($<stru>1); 
		if (result) { err(yylineno, result); }     
	}
	| VALUE EQ VALUE
	{
		if (DEBUG) printf("Operacja boolowska %s == %s\n", $<stru>1, $<stru>3);

		if (variableManager.getValueOfVariable($<stru>1) == "")
		{
			stri err = "Niezainicjalizowana zmienna ";
			err += $<stru>1; 
			catch_error(yylineno, err.c_str());
		}

		if (variableManager.getValueOfVariable($<stru>3) == "")
		{
			stri err = "Niezainicjalizowana zmienna ";
			err += $<stru>3;
			catch_error(yylineno, err.c_str());
		}
		
		int result = generateBoolOp(S_EQ, $<stru>1, $<stru>3);     
		if (result == -1) { err(yylineno, result); }       
 
		std::ostringstream os; 
		os << result;  
		$$ = strdup(os.str().c_str());
	}	  
	| VALUE NEQ VALUE
	{
		if (DEBUG) printf("Operacja boolowska %s <> %s\n", $<stru>1, $<stru>3);

		if (variableManager.getValueOfVariable($<stru>1) == "")
		{ 
			stri err = "Niezainicjalizowana zmienna ";
			err += $<stru>1; 
			catch_error(yylineno, err.c_str()); 
		} 

		if (variableManager.getValueOfVariable($<stru>3) == "")
		{
			stri err = "Niezainicjalizowana zmienna ";
			err += $<stru>3;
			catch_error(yylineno, err.c_str());
		} 

		int result = generateBoolOp(S_NEQ, $<stru>1, $<stru>3);
		if (result == -1) { err(yylineno, result); }

		std::ostringstream os;
		os << result; 
		$$ = strdup(os.str().c_str());    
	}  
	| VALUE LT VALUE  
	{ 
		if (DEBUG) printf("Operacja boolowska %s < %s\n", $<stru>1, $<stru>3);

		if (variableManager.getValueOfVariable($<stru>1) == "")
		{
			stri err = "Niezainicjalizowana zmienna ";
			err += $<stru>1; 
			catch_error(yylineno, err.c_str());
		}

		if (variableManager.getValueOfVariable($<stru>3) == "")
		{
			stri err = "Niezainicjalizowana zmienna ";
			err += $<stru>3;
			catch_error(yylineno, err.c_str());
		}

		int result = generateBoolOp(S_GT, $<stru>3, $<stru>1);
		if (result == -1) { err(yylineno, result); }       
 
		std::ostringstream os;  
		os << result;  
		$$ = strdup(os.str().c_str()); 
	} 
	| VALUE GT VALUE 
	{
		if (DEBUG) printf("Operacja boolowska %s > %s\n", $<stru>1, $<stru>3);

		if (variableManager.getValueOfVariable($<stru>1) == "")
		{
			stri err = "Niezainicjalizowana zmienna ";
			err += $<stru>1; 
			catch_error(yylineno, err.c_str());
		}

		if (variableManager.getValueOfVariable($<stru>3) == "")
		{
			stri err = "Niezainicjalizowana zmienna ";
			err += $<stru>3;
			catch_error(yylineno, err.c_str());
		}

		int result = generateBoolOp(S_GT, $<stru>1, $<stru>3);
		if (result == -1) { err(yylineno, result); }

		std::ostringstream os; 
		os << result;
		$$ = strdup(os.str().c_str());
	}  
	| VALUE GET VALUE 
	{	
		if (DEBUG) printf("Operacja boolowska %s >= %s\n", $<stru>1, $<stru>3);

		if (variableManager.getValueOfVariable($<stru>1) == "")
		{
			stri err = "Niezainicjalizowana zmienna ";
			err += $<stru>1; 
			catch_error(yylineno, err.c_str());
		}

		if (variableManager.getValueOfVariable($<stru>3) == "")
		{
			stri err = "Niezainicjalizowana zmienna ";
			err += $<stru>3;
			catch_error(yylineno, err.c_str());
		}
 
		int result = generateBoolOp(S_GET, $<stru>1, $<stru>3);
		if (result == -1) { err(yylineno, result); }

		std::ostringstream os;
		os << result;  
		$$ = strdup(os.str().c_str());       
	}  
	| VALUE LET VALUE 
	{  
		if (DEBUG) printf("Operacja boolowska %s <= %s\n", $<stru>1, $<stru>3);

		if (variableManager.getValueOfVariable($<stru>1) == "")
		{
			stri err = "Niezainicjalizowana zmienna ";
			err += $<stru>1; 
			catch_error(yylineno, err.c_str());
		}

		if (variableManager.getValueOfVariable($<stru>3) == "")
		{
			stri err = "Niezainicjalizowana zmienna ";
			err += $<stru>3;
			catch_error(yylineno, err.c_str());
		}

		int result = generateBoolOp(S_GET, $<stru>3, $<stru>1); //odwracamy argumenty, bo (a>=b) <=> (b<=a)
		if (result == -1) { err(yylineno, result); }      
 
		std::ostringstream os;  
		os << result; 
		$$ = strdup(os.str().c_str()); 
	}
	
;

VALUE : NUM
	| PIDENTIFIER LBRACKET PIDENTIFIER RBRACKET    
	| PIDENTIFIER LBRACKET NUM RBRACKET   
	| PIDENTIFIER        
; 

%%
 
void err(int line, int no) { 
	char txt[50];
	switch(no) {
		case(-1):
			sprintf(txt, "Niezadeklarowana zmienna");
			break;
		case(-2):
			sprintf(txt, "Wczytywany obiekt nie jest zmienna");
			break;
		case(-5):
			sprintf(txt, "Niezainicjalizowana zmienna"); 
			break;
		default:   
			sprintf(txt, "BLAD NUMER %d", no); 
			break;
	}
	
	
	catch_error(line, txt);  
}

void catch_error(int linia, const char* err) {
	printf("BLAD: [Linia %d] %s\n", linia, err);
	exit(0); //kompiluje do pierwszego napotkanego bledu
}



int main()
{ 
	registerManager.initializeAccumulator();
	yyparse(); 

	return 0; 
}