#ifndef TRANSLATE_GUARD
#define TRANSLATE_GUARD

#define DEBUG 1
#define ERR 0
#define LINECODE 0

#define stri std::string
#define vec std::vector
#define longlong unsigned long long int

#include <math.h>
#include <string.h>

vec<stri> outputSource;

vec<int> jumpStack; //przechowuje miejsca skaczace
vec<int> labelStack; //przechowuje miejsca do ktorych beda odbywac sie skoki


int tempLines;
vec<stri> tempCode;

const stri op_temp_a = "0";
const stri op_temp_b = "1";
const stri op_temp_c = "2";
const stri op_temp_d = "3";
const stri op_temp_e = "4";
const int reserved_registers_number = 5;

const stri S_PLUS("+");
const stri S_MINUS("-");
const stri S_MULT("*");
const stri S_DIV("/");
const stri S_MOD("%");

const stri S_GET(">=");
const stri S_EQ("==");
const stri S_NEQ("<>");
const stri S_GT(">");
const stri S_LET("<=");
const stri S_LT("<");



/** POMOCNICZE */
int isNumber(stri name) {
   const char* buf = name.c_str();
   while(*buf){
      if (!isdigit(*buf))
         return 0;
      else
         ++buf;
   }
   return 1;
}

class RegisterManager {

	private:
		vec<stri> registerVector;
		vec<int> regValVector;
	public:
		int initializeAccumulator() {
			registerVector.push_back("");
			regValVector.push_back(-1);
		}

		int getValueFromRegister(int regnum) {
			if (regValVector.at(regnum) != -1) {
				return regValVector.at(regnum);
			} return -1;
		}

		int setValueToRegister(stri value, int regnum) {
			regValVector.at(regnum) = atoi(value.c_str());
		}

		int findFreeRegister() {
			// zwroc indeks pierwszego pustego rejestru
			int fullRegisters = registerVector.size();
			if (fullRegisters == 5) {
				return -1;
			} else {
				return fullRegisters;
			}
		}

		int populateRegister(stri variable) {
			registerVector.push_back(variable);
			regValVector.push_back(-1);
		}

		int getRegisterOfVariable(stri varName) {
			searchRegisterVector(varName);
			if (findFreeRegister() != -1) {
				populateRegister(varName);
				int result = searchRegisterVector(varName);
				return result;
			} else {
				return -5; // zrob STORE w pamieci zamiast COPY w rejestrze
			}
		}

		int removeLastVariable() { 
			registerVector.pop_back();
			regValVector.pop_back();
		}

		int getAccumulatorValue() {
			return getValueFromRegister(0);
		}

		int searchRegisterVector(stri varName) {
			for (int i = 0; i < registerVector.size(); i++) {
				if (varName == registerVector.at(i)) {
					return i;
				}
			} return -1;
		}

		void removeFromRegister(int regnum) {
			if (DEBUG) printf ("Removed variable from register %d", regnum);
			registerVector.at(regnum) = "";
			regValVector.at(regnum) = -1;
		}

};


class VariableManager {
	private:
		vec<stri> variableVector;
		vec<stri> valueVector;
		vec<int> memoryVector;
	public:
		int addVariable(stri varName, int valInAcc) {
			if (getItemIndex(varName) > -1) {
				if (DEBUG) printf("*******ZMIENNA %s JUZ ISTNIEJE", varName.c_str());
				return 1; // juz istnieje
			}
			variableVector.push_back(varName);
			valueVector.push_back("");

			int memoryAddress = valInAcc;
			memoryVector.push_back(memoryAddress);
			if (DEBUG) printf("Dodaje zmienna %s o adresie %d\n", varName.c_str(), valInAcc);
			return 0; // sukces
		}

		void deleteVariable(stri varName) {
			if (getItemIndex(varName) != -1) {
				int varIndex = getItemIndex(varName);
				variableVector.at(varIndex) = "";
				valueVector.at(varIndex) = "";
				memoryVector.at(varIndex) = -1;
			}
		}
		
		int getItemIndex(stri varName) {
			for (int i = 0; i < variableVector.size(); i++) {
				if (varName == variableVector.at(i)) 
				{
					if (varName == "i")
					return i;
				}
			}
			if (ERR) printf("*******nie ma takiej zmiennej\n");
			return -1;
		}

		int getAddressIndex(int address) {
			for (int i = 0; i < memoryVector.size(); i++) {
				if (address == memoryVector.at(i)) {
					return i;
				}
			}
		}

		stri getItem(int itemIndex) {
			return variableVector.at(itemIndex);
		}

		int setValueToVariable(stri varName, stri varVal) {
			if (getItemIndex(varName) != -1) {
				int index = getItemIndex(varName);

				if (isNumber(varVal)) {
					valueVector.at(index) = varVal;
					if (DEBUG) printf("%s = %s", varName.c_str(), varVal.c_str());
				} else {
					// sprawdz, czy zmienna jest zainicjalizowana
					if (getValueOfVariable(varVal) != "") {
						valueVector.at(index) = getValueOfVariable(varVal);
					} else {
						if (DEBUG) printf("podstawiana zmienna jest niezainicjalizowana\n");
						return -2;
					} 
				}
			} else {
				if (DEBUG) printf("*******przypisanie wartosci do nieistniejacej zmiennej\n");
				return -1;
			}
		}

		stri getValueOfVariable(stri varName) {
			if (isNumber(varName)) {
				return varName;
			}
			stri varVal = valueVector.at(getItemIndex(varName));

			return varVal;
		}

		int setAddressOfVariable(stri varName, int address) {
			if (getItemIndex(varName) != -1) {
				int index = getItemIndex(varName);
				memoryVector.at(index) = address;
			} else {
				if (DEBUG) printf("\t\t*******tworze zmienna %s\n", varName.c_str());
				addVariable(varName, address);
			}
		}

		int getAddressOfVariable(stri varName) { 
			int varAddress = memoryVector.at(getItemIndex(varName));
			if (varAddress != -1) {
				return varAddress;
			}
			return -1;
		}

		stri getVariableAtAddress(int address) {
			return variableVector.at(getAddressIndex(address));
		}
};

/*
*	Register 0 memory manager
*/
class MemoryManager {
	private:
		vec<stri> memoryVariables;
		vec<int> memoryValues;
	public:
		// pr_0 <- r_i
		int storeInMemory(stri variable, int address) {
			memoryVariables.push_back(variable);
			memoryValues.push_back(address);
			return memoryVariables.size() - 1 + reserved_registers_number; 
		}
};

VariableManager	variableManager;
RegisterManager registerManager;
MemoryManager memoryManager;

void addCodeLine(stri line) {
	tempCode.push_back(line);
}


int generateDivision(int a, int b, bool modulo) {


 // 	int a=42;
 //    int b=7;
 //    int result = 0;
 //    int k=0;
    /** ustawiamy liczby jak w dzieleniu pisemnym
    *
    *   ____            
    *   1100                
    *   110                  
    *
    *   k = 1 : jedno przesuniecie bylo potrzebne, zeby ustawic liczby dobrze
    *
    *   ______
    *   101010
    *    110
    *  
    *   k = 2 : dwa przsuniecia byly potrzebne, zeby ustawic liczby dobrze
    */
  //   while(b<=a){
  //     b = b<<1;
  //     k++;
  //   }
  //   while(k>0){   
  //     k--;
  //     b=b>>1;
  //     result = result<<1;
  //     if(a>=b) {      
  //         result=result+1;
  //         a=a-b;
  //     }
  //   }
  //   System.out.println("Result "+result+" remainder " + a);
  // }

	char temp[50];
	stri k = "k";
	int k_val = 0;

	std::stringstream ss;
	ss << k_val;
	declareVariable(k);
	variableManager.setValueToVariable(k, ss.str());
	int k_reg = getVariableRegister(k);

	int pl = tempCode.size();
	int a_reg = getVariableRegister(a);
	int b_reg = getVariableRegister(b);

	// poczatek pierwszego while'a

	generateBoolOp(S_LET, b, a);
	int pl2 = tempCode.size();
	sprintf(temp, "JZERO 0 %d", pl2+5); // jesli warunek niespelniony, jump_poza_petle (WHILE)
	addCodeLine(temp);
	sprintf(temp, "SHL %d", b_reg);		// w p.p. wykonaj cialo petli
	addCodeLine(temp);
	k_val++;
	ss << k_val;
	sprintf(temp, "INC %d", k_reg);
	addCodeLine(temp);
	variableManager.setValueToVariable(k, ss.str());
	sprintf(temp, "JUMP %d", pl);			// jump_do_sprawdzenia_warunku
	addCodeLine(temp);

	// koniec pierwszego while'a
	// poczatek drugiego while'a

	int pl3 = tempCode.size();
	generateBoolOp(S_GT, k, ss.str());
	sprintf(temp, "JZERO 0 %d", pl4);
	addCodeLine(temp);
	sprintf(temp, "SHR %d", b_reg);
	k_val--;
	ss << k_val;
	sprintf(temp, "DEC %d", k_reg);
	addCodeLine(temp);
	variableManager.setValueToVariable(k, ss.str());

	// poczatek IF
	generateBoolOp(S_GET, a, b);
	sprintf(temp, "JZERO 0 %d", pl4); 	// koniec IF
	addCodeLine(temp);
	generateArithOp(S_MINUS, a, b);		// THEN
	generateVariableAssign(a, registerManager.getAccumulatorValue());
	sprintf(temp, "JUMP %d", pl4); 
	addCodeLine(temp);
	// koniec IF

	sprintf(temp, "JUMP %d", pl3); 		// jump_do_sprawdzenia_warunku
	addCodeLine(temp);
	int pl4 = tempCode.size();
	// koniec drugiego while'a
}

void generateMultiplication(int a, int b) {
	char temp[50];
 //    int a=10;
 //    int b=3;
 //    int result = 0;
 //    while(b>0){      
 //      if(b%2==1) {     
 //        result = result+a;
 //        System.out.println(b + " is odd");
 //      }
 //      a = a<<1;
 //      b = b>>1;   
 //      System.out.println("New b is "+b);
 //    }     
 //    System.out.println(result);


}

/**
 * ZARZADZANIE PAMIECIA
 */


void binaryNumberToCode(stri bin, stri dec) {
    vec<stri> v;
    int numreg = registerManager.findFreeRegister(); // -1
    
    std::ostringstream os;
	os << numreg;
	stri regnum = strdup(os.str().c_str());
	if (DEBUG) printf("Rejestr liczby %s to %s\n", dec.c_str(), regnum.c_str());

	stri zero = "ZERO " + regnum;
	stri inc = "INC " + regnum;
	stri shl = "SHL " + regnum;

    while (bin != "0") {
        char l = bin.at(bin.size() - 1);
        if (l == '1') {
            v.push_back(inc);
            bin.at(bin.size() - 1) = '0';
        } else if (l == '0') {
            v.push_back(shl); 
            bin.erase(bin.size() - 1, 1);
        }
    }
    v.push_back(zero);
    

    for (int i = v.size() - 1; i >= 0; i--) {
        addCodeLine(v.at(i));
	}
	addCodeLine("COPY " + regnum);
	registerManager.setValueToRegister(dec, 0);

}

stri divideByTwo(stri dec) {
    int rem = 0;
    int m = dec.size();
    for (int i = 0; i < m; i++) {
        int a = dec.at(i) - 48; //cyfra
        dec.at(i) = (rem + a) / 2 + 48;
        if (a % 2) { rem=10; } else {rem = 0;}
    }
    return dec;
}

stri strDecToStrBin(stri dec) {
    stri bin = "";
    int m = dec.size();
    int i = 0;
    while (dec.size()) {
        char l = dec.at(dec.size() - 1);
        if (l == '0' || l == '2' || l == '4' || l == '6' || l == '8') { //parzysta
            bin = '0' + bin;
        } else {
            bin = '1' + bin;
        }
        dec = divideByTwo(dec);
        if (dec.at(0) == '0') { dec.erase(0,1); }
    }
    return bin;
}

void generateValue(stri a) {
    stri binary = strDecToStrBin(a);
    binaryNumberToCode(binary, a);
}


int generateP_A(stri a) {
	if (isNumber(a)) {
		generateValue(a);
	} else {
		int AvarIndex = variableManager.getItemIndex(a);
		if (AvarIndex == -1) { 
			if (ERR) printf("*******ARITH OP NIE ZNALEZIONO");
				return -1;
		} else { // jezeli istnieje zmienna
			char temp[50];
			int reg = AvarIndex+1;
			sprintf(temp, "LOAD %d", reg);
			addCodeLine(temp);
			registerManager.setValueToRegister(a, reg);
		}
	}

	return 0;
}
 
 /** Funkcja ktora
	1. Sprawdza czy VALUE jest liczba- jesli tak- generuj w rejestrze a
	3. Sprawdza, czy VALUE jest zmienna - laduj zmienna
	Po wykonaniu tej funkcji rejestr pomocniczy op_temp_a zawiera a, op_temp_b zawiera b
	*/
int generateP_AB(stri a, stri b) {
	char temp[50];
	int reg;
	int ret = generateP_A(a);
	if (ret != 0) return ret;

	if (isNumber(a)) {
		reg = registerManager.findFreeRegister();
		variableManager.setAddressOfVariable(a, registerManager.getAccumulatorValue());
		sprintf(temp, "STORE %d", reg);
		addCodeLine(temp); // r_i <- pr_0 
		registerManager.removeLastVariable();
	}
	
	ret = generateP_A(b);
	if (ret != 0) return ret;

	if (isNumber(b)) {
		reg = registerManager.findFreeRegister();
		variableManager.setAddressOfVariable(b, registerManager.getAccumulatorValue());
		sprintf(temp, "STORE %d", reg);
		addCodeLine(temp); // r_i = pr_0
		registerManager.removeLastVariable();
	}
	return 0;
}

int declareVariable(stri varName) {
	std::stringstream ss;

	if (DEBUG) printf("\tDeklaracja zmiennej <%s>\n", varName.c_str());
	if (varName == "") return -1;
	int valInAcc = registerManager.getAccumulatorValue();
	ss << variableManager.addVariable(varName, valInAcc);
	stri addingVariable = ss.str();
	int result = atoi(addingVariable.c_str());
	int freeReg = registerManager.findFreeRegister();
	printf("Wolny rejestr nr %d dla zmiennej %s\n", freeReg, varName.c_str());
	if (freeReg != -1)
		registerManager.populateRegister(varName);

	return result;
}

int generateVariableAssign(stri varName, stri varVal) { 
	int varIndex = variableManager.getItemIndex(varName);
	if (varIndex == -1) { // jeśli nie istnieje zmienna
		return 1;
	}
	// istnieje indeks zmiennej
	if (variableManager.getValueOfVariable(varVal) == "") {
		return 2;
	}
	variableManager.setValueToVariable(varName, varVal);
	if (DEBUG) printf("\tPrzypisuje zmiennej <%s> wartosc <%s>\n", varName.c_str(), variableManager.getValueOfVariable(varName).c_str());
	if (DEBUG) printf("Indeks zmiennej to %d\n", varIndex);
	char temp[50];
	sprintf(temp, "STORE %d", varIndex);
	variableManager.setAddressOfVariable(varName, registerManager.getAccumulatorValue());
	addCodeLine(temp);

	return 0;
}

int getVariableRegister(stri variable) {
	if (registerManager.getRegisterOfVariable(variable) == -1) 
	{
		return -5;
	} else { 
		return registerManager.getRegisterOfVariable(variable); 
	}
}

int generateArithOp(stri op, stri a, stri b) {
	generateP_AB(a, b);

	int a_val, b_val = 0;
	if (isNumber(a)) {
		a_val = atoi(a.c_str());
	} else {
		a_val = atoi(variableManager.getValueOfVariable(a).c_str());
	}
	if (isNumber(b)) {
		b_val = atoi(b.c_str());
	} else {
		b_val = atoi(variableManager.getValueOfVariable(b).c_str());
	}

	char temp[50];

	int reg_of_a = getVariableRegister(a);
		int reg_of_b = getVariableRegister(b);
		if (reg_of_a == -5)
			reg_of_a = memoryManager.storeInMemory(a, variableManager.getAddressOfVariable(a));
		if (reg_of_b == -5) 
			reg_of_b = memoryManager.storeInMemory(b, variableManager.getAddressOfVariable(b));

	if (op == S_PLUS) {


		if (a == "1") {
			sprintf(temp, "INC %d", reg_of_b);
			addCodeLine(temp);
			sprintf(temp, "COPY %d", reg_of_b);
			addCodeLine(temp);
		} else if (b == "1") {
			sprintf(temp, "INC %d", reg_of_a);
			addCodeLine(temp);
			sprintf(temp, "COPY %d", reg_of_a);
			addCodeLine(temp);
		} else {
			sprintf(temp, "STORE %d", reg_of_a); 
			addCodeLine(temp); // pr_0 = reg_a
			sprintf(temp, "ADD %d", reg_of_b);
			addCodeLine(temp); // reg_b = reg_b + pr0
			sprintf(temp, "COPY %d", reg_of_b);				
			addCodeLine(temp); // r_0 = reg_b
		}

		registerManager.removeLastVariable();

		return a_val + b_val;

	} else if (op == S_MINUS) {
		

		if (b == "1") {
			sprintf(temp, "DEC %d", reg_of_a);
			addCodeLine(temp);
			sprintf(temp, "COPY %d", reg_of_a);
			addCodeLine(temp);
		} else {
			sprintf(temp, "STORE %d", reg_of_b); 
			addCodeLine(temp); // pr_0 = reg_b
			sprintf(temp, "SUB %d", reg_of_a);
			addCodeLine(temp); // reg_a = reg_a - pr0
			sprintf(temp, "COPY %d", reg_of_a);				
			addCodeLine(temp); // r_0 = reg_a
		}

		registerManager.removeLastVariable();
		int result = a_val - b_val;

		if (result < 0) 
			return 0;
		else return a_val - b_val;
		return result;

	} else if (op == S_MULT) {


		if (a == "0" || b == "0") {
			sprintf(temp, "ZERO 0");
			addCodeLine(temp);

			registerManager.removeLastVariable();
			return 0;
		}
		
		if (b == "2") {
			sprintf(temp, "SHR %d", reg_of_a);
			addCodeLine(temp);
			sprintf(temp, "COPY %d", reg_of_a);				
			addCodeLine(temp); // r_0 = reg_a

			registerManager.removeLastVariable();
			return a_val * 2;
		}
		else if (a == "2") {
			sprintf(temp, "SHR %d", reg_of_b);
			addCodeLine(temp);
			sprintf(temp, "COPY %d", reg_of_b);				
			addCodeLine(temp); // r_0 = reg_b

			registerManager.removeLastVariable();
			return 2 * b_val;
		}
		else {
			generateMultiplication(a, b);

			registerManager.removeLastVariable();
			return a_val * b_val;
		}
		
	} else if (op == S_DIV) {

		
		if (a == "0" || b == "0") {
			sprintf(temp, "ZERO 0");
			addCodeLine(temp);

			registerManager.removeLastVariable();
			return 0;
		}

		if (b == "2") {
			sprintf(temp, "SHL %d", reg_of_a);
			addCodeLine(temp);
			sprintf(temp, "COPY %d", reg_of_a);				
			addCodeLine(temp); // r_0 = reg_a

			registerManager.removeLastVariable();
			return floor(a_val / 2);
		}
		else if (a == "2") {
			sprintf(temp, "SHL %d", reg_of_b);
			addCodeLine(temp);
			sprintf(temp, "COPY %d", reg_of_b);				
			addCodeLine(temp); // r_0 = reg_b

			registerManager.removeLastVariable();
			return floor(2 / b_val);
		}
		else {
			generateDivision(a, b, false);

			registerManager.removeLastVariable();
			return floor(a_val / b_val);
		}	


	} else if (op == S_MOD) {		
		generateDivision(a, b, true);
		
		registerManager.removeLastVariable();
		return a_val % b_val;
	}
	else { 
		if (ERR) printf("Nieznany operator %s\n", op.c_str());
	}

	return 0;
}

int generateBoolOp(stri op, stri a, stri b) {
	generateP_AB(a, b);

	int a_val, b_val = 0;
	if (isNumber(a)) {
		a_val = atoi(a.c_str());
	} else {
		a_val = atoi(variableManager.getValueOfVariable(a).c_str());
	}
	if (isNumber(b)) {
		b_val = atoi(b.c_str());
	} else {
		b_val = atoi(variableManager.getValueOfVariable(b).c_str());
	}

	char temp[50];

	int reg_of_a = getVariableRegister(a);
	int reg_of_b = getVariableRegister(b);
	if (reg_of_a == -5)
		reg_of_a = memoryManager.storeInMemory(a, variableManager.getAddressOfVariable(a));
	
	if (reg_of_b == -5)
		reg_of_b = memoryManager.storeInMemory(b, variableManager.getAddressOfVariable(b));

	
	if (op == S_GET) { // a >= b
		
		// a - b = 0  ==> return 1;
		// a - b > 0  ==> return 0;
		generateArithOp(S_MINUS, b, a);
		int currentLine = tempCode.size() - 1;
		sprintf(temp, "JZERO 0 %d", currentLine+1);	
		addCodeLine(temp);

		registerManager.removeLastVariable();
		if (a_val >= b_val) {
			return 1;
		} else return 0;


	} else if (op == S_EQ) { // a == b
		
		int reg_of_c = registerManager.findFreeRegister();

		sprintf(temp, "STORE %d", reg_of_b);
		addCodeLine(temp);
		sprintf(temp, "SUB %d", reg_of_a);
		addCodeLine(temp);
		sprintf(temp, "STORE %d", reg_of_c);
		addCodeLine(temp);
		sprintf(temp, "LOAD %d", reg_of_a);
		addCodeLine(temp);
		sprintf(temp, "SUB %d", reg_of_b);
		addCodeLine(temp);
		sprintf(temp, "ADD %d", reg_of_c);
		addCodeLine(temp);

		registerManager.removeLastVariable();

		int result1 = generateArithOp(S_MINUS, a, b);
		int result2 = generateArithOp(S_MINUS, b, a);
		int main_result; 
		
		if (result1 == result2) {
			main_result = 1;
		} else main_result = 0;
		printf("%d == %d ---> %d\n", a_val, b_val, main_result);
		return main_result;

	} else if (op == S_NEQ) { // a <> b
		
		int reg_of_c = registerManager.findFreeRegister();

		sprintf(temp, "STORE %d", reg_of_b);
		addCodeLine(temp);
		sprintf(temp, "SUB %d", reg_of_a);
		addCodeLine(temp);
		sprintf(temp, "STORE %d", reg_of_c);
		addCodeLine(temp);
		sprintf(temp, "LOAD %d", reg_of_a);
		addCodeLine(temp);
		sprintf(temp, "SUB %d", reg_of_b);
		addCodeLine(temp);
		sprintf(temp, "ADD %d", reg_of_c);
		addCodeLine(temp);

		registerManager.removeLastVariable();

		int result1 = generateArithOp(S_MINUS, a, b);
		int result2 = generateArithOp(S_MINUS, b, a);
		int main_result; 
		
		if (result1 == result2) {
			main_result = 0;
		} else main_result = 1;
		if (DEBUG) printf("%d == %d ---> %d\n", a_val, b_val, main_result);
		return main_result;

	} else if (op == S_GT) { // a > b
		
		generateArithOp(S_MINUS, a, b);
		int currentLine = tempCode.size() - 1;
		addCodeLine("ZERO 0");
		sprintf(temp, "JZERO 0 %d", currentLine+3);
		addCodeLine(temp);
		sprintf(temp, "JUMP %d", currentLine+5);
		addCodeLine(temp);
		sprintf(temp, "INC 0");
		addCodeLine(temp);

		registerManager.removeLastVariable();
		if (a_val > b_val) {
			return 1;
		} else return 0;

	} else if (op == S_LET) { // a <= b
		
		generateArithOp(S_MINUS, a, b);
		int currentLine = tempCode.size() - 1;
		sprintf(temp, "JZERO 0 %d", currentLine+1);	
		addCodeLine(temp);

		registerManager.removeLastVariable();
		if (a_val <= b_val) {
			return 1;
		} else return 0;


	} else if (op == S_LT) { // a < b
		
		generateArithOp(S_MINUS, b, a);
		int currentLine = tempCode.size() - 1;
		addCodeLine("ZERO 0");
		sprintf(temp, "JZERO 0 %d", currentLine+3);
		addCodeLine(temp);
		sprintf(temp, "JUMP %d", currentLine+5);
		addCodeLine(temp);
		sprintf(temp, "INC 0");
		addCodeLine(temp);

		registerManager.removeLastVariable();
		if (a_val < b_val) {
			return 1;
		} else return 0; 

	} else {
		return -1;
		if (ERR) printf("Nieznany operator %s", op.c_str());
	}
	return 0;
}

int generateElse() {
	int pl = tempCode.size();
	if (DEBUG) printf("Generuje else w linii %d\n", pl);

	return pl;
}

int generateThen() {
	char temp[50];
	int pl = tempCode.size();
	if (DEBUG) printf("Generuje then w linii %d\n", pl);
	sprintf(temp, "JUMP %d", generateElse()+1);
	addCodeLine(temp);

	return pl;
}

void generateIf() {
	char temp[50];
	int pl = tempCode.size();
	if (DEBUG) printf("Generuje if w linii %d\n", pl);
	sprintf(temp, "JZERO 0 %d", generateThen()+1); // goto ELSE
	addCodeLine(temp);
}

int generateFor(stri pid, stri from, stri to, bool mode) {
	char temp[50];
	int pl = tempCode.size();
	if (DEBUG) printf("Generuje for w linii %d\n", pl);

	int regOfIterator = variableManager.getItemIndex(pid)+1; 
	printf("regOfIterator %s: %d\n", pid.c_str(), regOfIterator);

	if (isNumber(from)) {
		std::stringstream ss;
		ss << atoi(from.c_str());
		registerManager.setValueToRegister(ss.str(), regOfIterator); 
		if (DEBUG) printf("Value %s set to register %d\n", from.c_str(), regOfIterator);
	} else {
		registerManager.setValueToRegister(variableManager.getValueOfVariable(from).c_str(), regOfIterator);
		if (DEBUG) printf("Value %s set to register %d\n", variableManager.getValueOfVariable(from).c_str(), regOfIterator);
	}

	sprintf(temp, "LOAD %d", regOfIterator);
	addCodeLine(temp);

	int result = generateBoolOp(S_GET, pid, to);
	sprintf(temp, "JODD 0 %d", pl+4); // warunek spelniony, wykonaj cialo petli
	addCodeLine(temp);
	if (mode == true) {
		sprintf(temp, "INC %d", regOfIterator); // i++
		addCodeLine(temp);
		std::stringstream ss;
		ss << registerManager.getValueFromRegister(regOfIterator)+1;

		registerManager.setValueToRegister(ss.str(), regOfIterator);
	} else {
		sprintf(temp, "DEC %d", regOfIterator); // i--
		addCodeLine(temp);
		std::stringstream ss;
		ss << registerManager.getValueFromRegister(regOfIterator)+1;

		registerManager.setValueToRegister(ss.str(), regOfIterator);
	}
	if (DEBUG) printf("iterator: %d\n", registerManager.getValueFromRegister(regOfIterator));
	sprintf(temp, "JUMP %d", pl+5); // koniec petli for
	addCodeLine(temp);

	if (DEBUG) printf("Koniec generateFor()\n");
	return pl+1;
}

void generateToDo(int placeholder) {
	char temp[50];
	int pl = tempCode.size();
	if (DEBUG) printf("Generuje to-do w linii %d\n", pl);

	sprintf(temp, "JUMP %d", placeholder);// jump_do_sprawdzenia_warunku
	addCodeLine(temp);
}

void generateDowntoDo(int placeholder) { 
	char temp[50];
	int pl = tempCode.size();
	if (DEBUG) printf("Generuje downto-do w linii %d\n", pl);

	sprintf(temp, "JUMP %d", placeholder);// jump_do_sprawdzenia_warunku
	addCodeLine(temp);
}

int generateDo(int placeholder) {
	char temp[50];
	int pl = tempCode.size();
	if (DEBUG) printf("Generuje do w linii %d\n", pl);
	sprintf(temp, "JUMP %d", placeholder); // jump_do_sprawdzenia_warunku
	addCodeLine(temp);

	return pl;
}

void generateWhile() {
	char temp[50];
	int pl = tempCode.size();
	if (DEBUG) printf("Generuje while w linii %d\n", pl);	
	sprintf(temp, "JZERO 0 %d", generateDo(pl+1)); // jump_poza_petle
	addCodeLine(temp);
}

int generateWrite(stri a) {
	if (DEBUG) printf("Generuje write\n");
	char temp[50];
	int ret = generateP_A(a);
	if (ret != 0) return ret;

	if (isNumber(a)) { // liczba
		addCodeLine("PUT 0");
	} else { // zmienna
		if (variableManager.getValueOfVariable(a) == "") return 666;
		int a_val = atoi(variableManager.getValueOfVariable(a).c_str());

		int AvarIndex = variableManager.getItemIndex(a);
		int reg = AvarIndex+1;
		int value = registerManager.getValueFromRegister(reg);
 		sprintf(temp, "PUT %d", reg);
		if (DEBUG) printf("%s = %d\n", a.c_str(), value);
		addCodeLine(temp);
	}

	return 0;

}

int generateRead(stri a) {
	if (DEBUG) printf("Generuje read\n");
	if (isNumber(a)) {
		return -2;
	} else {
		int AvarIndex = variableManager.getItemIndex(a); 
		if (AvarIndex == -1) { 
			return -1;
		} else { // jezeli istnieje zmienna
			std::stringstream ss;
			ss << atoi(a.c_str());
			registerManager.setValueToRegister(ss.str(), AvarIndex+1);
			ss << registerManager.getValueFromRegister(AvarIndex+1);
			variableManager.setValueToVariable(a, ss.str());
			char temp[50];
			int reg = registerManager.getRegisterOfVariable(a);
			sprintf(temp, "GET %d", reg);
			addCodeLine(temp);
			sprintf(temp, "STORE %d", reg);
			addCodeLine(temp);
			variableManager.setAddressOfVariable(a, registerManager.getAccumulatorValue());
		}
	}
	return 0;
}


void endOfProgram() {
	if (DEBUG) printf("Koniec programu, zwracam HALT\n\n\n******** KOD ********\n\n\n");
	addCodeLine("HALT");
	for (int i = 0; i < tempCode.size(); i++) { 
		if (DEBUG || LINECODE) printf("%d. ", i);
		printf("%s\n", tempCode.at(i).c_str());
	}
}

#endif