

/*

FIXIT
1. dość brzydko zrealizowane dzielenie
2. co z adresami liczb w pamieci?
3. poprawic store, load, add, sub w obliczeniach
*/

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

// 0 - akumulator
// 1, 2 - zmienne
// 3 - wynik
// 4 - iterator petli
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
				//printf("value at %d is %d\n", regnum, regValVector.at(regnum));
				return regValVector.at(regnum);
			} else if (regnum == 0) {
				//printf("accumulator is empty\n");
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
			return -1;
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
			return 0; // sukces
		}
		
		int getItemIndex(stri varName) {
			for (int i = 0; i < variableVector.size(); i++) {
				if (varName == variableVector.at(i)) {
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
				} else {
					// sprawdz, czy zmienna jest zainicjalizowana
					if (getValueOfVariable(varVal) != "") {
						valueVector.at(index) = getValueOfVariable(varVal);
					} else {
						if (ERR) printf("podstawiana zmienna jest niezainicjalizowana\n");
						return -1;
					} 
				}
			} else {
				printf("*******przypisanie wartosci do nieistniejacej zmiennej\n");
				return -1;
			}
		}

		stri getValueOfVariable(stri varName) {
			stri varVal = valueVector.at(getItemIndex(varName));
			if (varVal != "") {
				return varVal;
			}
			return "null";
		}

		int setAddressOfVariable(stri varName, int address) {
			if (getItemIndex(varName) != -1) {
				int index = getItemIndex(varName);
				memoryVector.at(index) = address;
			} else {
				printf("\t\t*******tworze zmienna %s\n", varName.c_str());
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


void generateDivision() {
	char temp[50];
	int mod=0;
}

/**
 * ZARZADZANIE PAMIECIA
 */


void binaryNumberToCode(stri bin, stri dec) {
    vec<stri> v;
    int numreg = registerManager.findFreeRegister();
    
    std::ostringstream os;
	os << numreg;
	stri regnum = strdup(os.str().c_str());

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
    v.push_back(zero + "\t ** binaryNumberToCode: " + dec.c_str() +" **");
    

    for (int i = v.size() - 1; i >= 0; i--) {
        addCodeLine(v.at(i));
	}
	addCodeLine("COPY " + regnum + "\t ** copy " + dec.c_str() + " to register 0 **");
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
			//variableManager.setAddressOfVariable(a, registerManager.getAccumulatorValue());
			sprintf(temp, "STORE %d \t** generateP_A(%s) **", AvarIndex, a.c_str());
			addCodeLine(temp);
		}
	}

	return 0;
}

	// LOAD i		r_i <- pr_0
	// STORE i 		pr_0 <- r_i     <=======>    COPY i 	r_0 <- r_i
	// ZERO i 		r_i <- 0
	// ADD i 		r_i <- r_i + pr_0
	// SUB i 		r_i <- r_i - pr_0
 
 /** Funkcja ktora
	1. Sprawdza czy VALUE jest liczba- jesli tak- generuj w rejestrze a
	3. Sprawdza, czy VALUE jest zmienna - laduj zmienna
	Po wykonaniu tej funkcji rejestr pomocniczy op_temp_a zawiera a, op_temp_b zawiera b
	*/
int generateP_AB(stri a, stri b) {
	char temp[50];
	int ret = generateP_A(a);
	if (ret != 0) return ret;
	int reg = registerManager.findFreeRegister();
	variableManager.setAddressOfVariable(a, registerManager.getAccumulatorValue());
	sprintf(temp, "STORE %d \t** umieszczam %s w komorce pamieci %d", reg, a.c_str(), registerManager.getAccumulatorValue());
	addCodeLine(temp); // r_i <- pr_0 
	registerManager.removeLastVariable();
	
	ret = generateP_A(b);
	if (ret != 0) return ret;
	reg = registerManager.findFreeRegister();
	variableManager.setAddressOfVariable(b, registerManager.getAccumulatorValue());
	sprintf(temp, "STORE %d \t** umieszczam %s w komorce pamieci %d", reg, b.c_str(), registerManager.getAccumulatorValue());
	addCodeLine(temp); // r_i = pr_0
	registerManager.removeLastVariable();
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
	if (registerManager.findFreeRegister() != -1) {
		registerManager.populateRegister(varName);
		} else {
			// obsluga zwalniania pamieci zalatwia ten problem
		}
	return result;
}

int generateVariableAssign(stri varName, stri varVal) {
	int varIndex = variableManager.getItemIndex(varName);
	if (varIndex == -1) { // jeśli nie istnieje zmienna
		return 1;
	}
	// istnieje indeks zmiennej
	variableManager.setValueToVariable(varName, varVal);
	if (DEBUG) printf("\tPrzypisuje zmiennej <%s> wartosc <%s>\n", varName.c_str(), variableManager.getValueOfVariable(varName).c_str());
	if (DEBUG) printf("Indeks zmiennej to %d\n", varIndex);
	char temp[50];
	sprintf(temp, "STORE %d \t** // umieszczam %s w komorce pamieci %d // **", varIndex, varName.c_str(), registerManager.getAccumulatorValue());
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

void jumper() {
	if (DEBUG) printf("TEST JUMPERA\n");
	int line = tempCode.size() - 1;
	jumpStack.push_back(line);
	if (DEBUG) printf("Tymczasowy jumper w linii %d\n", line);
}

void labeler() {
	if (DEBUG) addCodeLine("---ETYKIETA---\n");
	int line = tempCode.size() - 1;
	labelStack.push_back(line);
	if (DEBUG) printf("Tymczasowy label w linii %d\n", line);
}

/*
	Zaczynajac od ostatnio dodanych etykiet i jumperow dopasowujemy je
*/

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

	// LOAD i		r_i <- pr_0
	// STORE i 		pr_0 <- r_i     <=======>    COPY i 	r_0 <- r_i
	// ZERO i 		r_i <- 0
	// ADD i 		r_i <- r_i + pr_0
	// SUB i 		r_i <- r_i - pr_0

	if (op == S_PLUS) {

		int reg_of_a = getVariableRegister(a); 
		int reg_of_b = getVariableRegister(b); 
		if (reg_of_a == -5)
			reg_of_a = memoryManager.storeInMemory(a, variableManager.getAddressOfVariable(a));
		if (reg_of_b == -5)
			reg_of_b = memoryManager.storeInMemory(b, variableManager.getAddressOfVariable(b));

		if (a == "1") {
			sprintf(temp, "INC %d \t** DODAWANIE, a = 1 **", reg_of_b);
			addCodeLine(temp);
			sprintf(temp, "COPY %d", reg_of_b);
			addCodeLine(temp);

			registerManager.removeLastVariable();
		} else if (b == "1") {
			sprintf(temp, "INC %d \t** DODAWANIE, b = 1 **", reg_of_a);
			addCodeLine(temp);
			sprintf(temp, "COPY %d", reg_of_a);
			addCodeLine(temp);

			registerManager.removeLastVariable();
		} else {
			sprintf(temp, "STORE %d \t** DODAWANIE (pr_0 = reg_a) **", reg_of_a); 
			addCodeLine(temp); // pr_0 = reg_a
			sprintf(temp, "ADD %d", reg_of_b);
			addCodeLine(temp); // reg_b = reg_b + pr0
			sprintf(temp, "COPY %d", reg_of_b);				
			addCodeLine(temp); // r_0 = reg_b

			registerManager.removeLastVariable();
		}

		return a_val + b_val;

	} else if (op == S_MINUS) {

		int reg_of_a = getVariableRegister(a);
		int reg_of_b = getVariableRegister(b);
		if (reg_of_a == -5) 
			reg_of_a = memoryManager.storeInMemory(a, variableManager.getAddressOfVariable(a));
		
		if (reg_of_b == -5)
			reg_of_b = memoryManager.storeInMemory(b, variableManager.getAddressOfVariable(b));
		

		if (b == "1") {

			sprintf(temp, "DEC %d \t** ODEJMOWANIE, b = 1 **", reg_of_a);
			addCodeLine(temp);
			sprintf(temp, "COPY %d", reg_of_a);
			addCodeLine(temp);

			registerManager.removeLastVariable();
		} else {
			sprintf(temp, "STORE %d \t** ODEJMOWANIE (pr_0 = reg_b) **", reg_of_b); 
			addCodeLine(temp); // pr_0 = reg_b
			sprintf(temp, "SUB %d", reg_of_a);
			addCodeLine(temp); // reg_a = reg_a - pr0
			sprintf(temp, "COPY %d", reg_of_a);				
			addCodeLine(temp); // r_0 = reg_a

			registerManager.removeLastVariable();
		}

		int result = a_val - b_val;

		if (result < 0) 
			return 0;
		else return a_val - b_val;
		return result;

	} else if (op == S_MULT) {

		int reg_of_a = getVariableRegister(a);
		int reg_of_b = getVariableRegister(b);
		if (reg_of_a == -5)
			reg_of_a = memoryManager.storeInMemory(a, variableManager.getAddressOfVariable(a));
		if (reg_of_b == -5) 
			reg_of_b = memoryManager.storeInMemory(b, variableManager.getAddressOfVariable(b));
		
		if (DEBUG) addCodeLine("-----------POCZATEK MNOZENIA");
		if (b == "2") {
			if (DEBUG) addCodeLine("b rowne 2");
			addCodeLine("SHR " + reg_of_a);

			registerManager.removeLastVariable();
			return a_val * 2;
		}
		else if (a == "2") {
			if (DEBUG) addCodeLine("a rowne 2");
			addCodeLine("SHR " + reg_of_b);

			registerManager.removeLastVariable();
			return 2 * b_val;
		}
		else {
			// zwykłe mnożenie

			registerManager.removeLastVariable();
		}	
		if (DEBUG) addCodeLine("-----------KONIEC MNOZENIA");
		return a_val * b_val;
		
	} else if (op == S_DIV) {

		int reg_of_a = getVariableRegister(a);
		int reg_of_b = getVariableRegister(b);
		if (reg_of_a == -5) 
			reg_of_a = memoryManager.storeInMemory(a, variableManager.getAddressOfVariable(a));
		
		if (reg_of_b == -5) 
			reg_of_b = memoryManager.storeInMemory(b, variableManager.getAddressOfVariable(b));
		

		if (b == "2") {
			if (DEBUG) addCodeLine("b rowne 2");
			addCodeLine("SHL " + reg_of_a);

			registerManager.removeLastVariable();
			registerManager.removeLastVariable();
			return floor(a_val / 2);
		}
		else if (a == "2") {
			if (DEBUG) addCodeLine("a rowne 2");
			addCodeLine("SHL " + reg_of_b);

			registerManager.removeLastVariable();
			registerManager.removeLastVariable();
			return floor(2 / b_val);
		}
		else {
			generateDivision();

			registerManager.removeLastVariable();
			registerManager.removeLastVariable();
			return floor(a_val / b_val);
		}	


	} else if (op == S_MOD){

		int reg_of_a = getVariableRegister(a);
		int reg_of_b = getVariableRegister(b);
		if (reg_of_a == -5)
			reg_of_a = memoryManager.storeInMemory(a, variableManager.getAddressOfVariable(a));
		
		if (reg_of_b == -5) 
			reg_of_b = memoryManager.storeInMemory(b, variableManager.getAddressOfVariable(b));
		

		generateDivision();
		//addCodeLine("STORE " + reg_of_b);
		//addCodeLine("LOAD " + reg_of_a);

		registerManager.removeLastVariable();
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

	
	//w tym miejscu w REJ_A jest 'b', pierwszy w p_666
	
	if (op == S_GET) {
		int reg_of_a = getVariableRegister(a);
		int reg_of_b = getVariableRegister(b);
		if (reg_of_a == -5)
			reg_of_a = memoryManager.storeInMemory(a, variableManager.getAddressOfVariable(a));
		
		if (reg_of_b == -5)
			reg_of_b = memoryManager.storeInMemory(b, variableManager.getAddressOfVariable(b));
		

		/*
		addCodeLine("SUB " + op_temp_b); // REJ_A=b-a  -- OK
		addCodeLine("JG"); // if b-a > 0 <=> b > a -> skacz
		*/

		if (a_val >= b_val) {
			return 1;
		} else return 0;

		registerManager.removeLastVariable();
		registerManager.removeLastVariable();

	} else if (op == S_EQ) {
		int reg_of_a = getVariableRegister(a);
		int reg_of_b = getVariableRegister(b);
		if (reg_of_a == -5) 
			reg_of_a = memoryManager.storeInMemory(a, variableManager.getAddressOfVariable(a));
		
		if (reg_of_b == -5) 
			reg_of_b = memoryManager.storeInMemory(b, variableManager.getAddressOfVariable(b));
		

		/*
		addCodeLine("STORE " + op_temp_b);
		addCodeLine("SUB " + op_temp_a); // REJ_A=b-a
		addCodeLine("STORE " + op_temp_c);
		addCodeLine("LOAD " + op_temp_a);
		addCodeLine("SUB " + op_temp_b); // REJ_A=a-b
		addCodeLine("ADD " + op_temp_c); // w Z by sie to skrocilo; dla N wynik > 0 swiadczy o niezerowosci ktoregos ze skladnikow
		addCodeLine("JG"); //skaczemy gdy (b-a) + (a-b) > 0, wtedy sa nierowne
		*/

		if (a_val == b_val) {
			return 1;
		} else return 0;

		registerManager.removeLastVariable();
		registerManager.removeLastVariable();

	} else if (op == S_NEQ) {
		int reg_of_a = getVariableRegister(a);
		int reg_of_b = getVariableRegister(b);
		if (reg_of_a == -5) 
			reg_of_a = memoryManager.storeInMemory(a, variableManager.getAddressOfVariable(a));
		
		if (reg_of_b == -5) 
			reg_of_b = memoryManager.storeInMemory(b, variableManager.getAddressOfVariable(b));
		

		/*
		addCodeLine("STORE " + op_temp_b);
		addCodeLine("SUB " + op_temp_a); // REJ_A=b-a
		addCodeLine("STORE " + op_temp_c);
		addCodeLine("LOAD " + op_temp_a);
		addCodeLine("SUB " + op_temp_b); // REJ_A=a-b
		addCodeLine("ADD " + op_temp_c); // w Z by sie to skrocilo; dla N wynik > 0 swiadczy o niezerowosci ktoregos ze 
		addCodeLine("JZERO");
		*/

		if (a_val != b_val) {
			return 1;
		} else return 0;

		registerManager.removeLastVariable();
		registerManager.removeLastVariable();

	} else if (op == S_GT) {
		int reg_of_a = getVariableRegister(a);
		int reg_of_b = getVariableRegister(b);
		if (reg_of_a == -5)
			reg_of_a = memoryManager.storeInMemory(a, variableManager.getAddressOfVariable(a));
		
		if (reg_of_b == -5)
			reg_of_b = memoryManager.storeInMemory(b, variableManager.getAddressOfVariable(b));
		

		/*
		addCodeLine("STORE " + op_temp_b);
		addCodeLine("SUB " + op_temp_a); // REJ_A = b-a
		addCodeLine("STORE " + op_temp_c);
		addCodeLine("LOAD " + op_temp_a); 
		addCodeLine("SUB " + op_temp_b); // REJ_A = a-b
		addCodeLine("SUB " + op_temp_c);
		addCodeLine("JZERO");
		*/

		if (a_val > b_val) {
			return 1;
		} else return 0;

		registerManager.removeLastVariable();
		registerManager.removeLastVariable();

	} else if (op == S_LET) {
		int reg_of_a = getVariableRegister(a);
		int reg_of_b = getVariableRegister(b);
		if (reg_of_a == -5)
			reg_of_a = memoryManager.storeInMemory(a, variableManager.getAddressOfVariable(a));
		
		if (reg_of_b == -5) 
			reg_of_b = memoryManager.storeInMemory(b, variableManager.getAddressOfVariable(b));
		

		/*
		addCodeLine("STORE " + op_temp_b);
		addCodeLine("SUB " + op_temp_a); // REJ_A = b-a
		addCodeLine("STORE " + op_temp_c);
		addCodeLine("LOAD " + op_temp_a); 
		addCodeLine("SUB " + op_temp_b); // REJ_A = a-b
		addCodeLine("SUB " + op_temp_c);
		addCodeLine("JZERO");
		*/

		if (a_val <= b_val) {
			return 1;
		} else return 0;

		registerManager.removeLastVariable();
		registerManager.removeLastVariable();

	

	} else if (op == S_LT) {
		int reg_of_a = getVariableRegister(a);
		int reg_of_b = getVariableRegister(b);
		if (reg_of_a == -5)
			reg_of_a = memoryManager.storeInMemory(a, variableManager.getAddressOfVariable(a));
		
		if (reg_of_b == -5) 
			reg_of_b = memoryManager.storeInMemory(b, variableManager.getAddressOfVariable(b));
		

		/*
		addCodeLine("STORE " + op_temp_b);
		addCodeLine("SUB " + op_temp_a); // REJ_A = b-a
		addCodeLine("STORE " + op_temp_c);
		addCodeLine("LOAD " + op_temp_a); 
		addCodeLine("SUB " + op_temp_b); // REJ_A = a-b
		addCodeLine("SUB " + op_temp_c);
		addCodeLine("JZERO");
		*/

		if (a_val < b_val) {
			return 1;
		} else return 0;

		registerManager.removeLastVariable();
		registerManager.removeLastVariable();

	} else {
		return -1;
		if (ERR) printf("Nieznany operator %s", op.c_str());
	}
	jumper();
	return 0;
}

void fixIfLabels() { 
	if (DEBUG) printf("Fixin'\n");
	
	for (int i = 0; i < 2; i++) {
		int jumperLine = jumpStack.back();
		if (DEBUG) printf("Skok z %d -> ", jumperLine);
		stri jumper = tempCode.at(jumperLine);
		
		if (labelStack.size() == 0) {
			if (DEBUG) printf("NO LABELS LEFT\n");
			return;
		}
		jumpStack.pop_back();
		
		int labelLine = labelStack.back();
		
		labelStack.pop_back();
		
		char op[10];
		char direction[50];
		char lab[50];
		sscanf(jumper.c_str(), "%s", op);
		if (DEBUG) printf(" ---> %d\n", labelLine + 1);
		sprintf(lab, "%s %d", op, labelLine + 1);
		stri t = lab;
		tempCode.at(jumperLine) = t;	
	}
}

void generateIf() {
	int pl = tempCode.size();
	if (DEBUG) printf("Generuje if w linii %d\n", pl);
}

void generateThen() {
	int pl = tempCode.size();
	if (DEBUG) printf("Generuje then w linii %d\n", pl);
	addCodeLine("JUMP");
	jumper();
	labeler();
}
void generateElse() {
	int pl = tempCode.size();
	if (DEBUG) printf("Generuje else w linii %d\n", pl);
	labeler();
	
	//char temp[30];
	//sprintf(temp, "IKSDE %d", labelStack.size());
	//addCodeLine(temp);
	fixIfLabels();
}

void generateFor() {
	int pl = tempCode.size();
	if (DEBUG) printf("Generuje for w linii %d\n", pl);
}
void generateFrom() {
	int pl = tempCode.size();
	if (DEBUG) printf("Generuje from w linii %d\n", pl);
}
void generateToDo() {
	int pl = tempCode.size();
	if (DEBUG) printf("Generuje to-do w linii %d\n", pl);
}
void generateDowntoDo() {
	int pl = tempCode.size();
	if (DEBUG) printf("Generuje downto-do w linii %d\n", pl);
}

void fixWhileLabels() {
	int label1 = labelStack.back();
	labelStack.pop_back();
	int label2 = labelStack.back();
	labelStack.pop_back();
	labelStack.push_back(label1);
	labelStack.push_back(label2);
	
	if (DEBUG) printf("Zamieniam etykiety %d i %d\n", label1, label2);
	
	for (int i = 0; i < 2; i++) {
		int jumperLine = jumpStack.back();
		if (DEBUG) printf("Skok z %d -> ", jumperLine);
		stri jumper = tempCode.at(jumperLine);
		
		if (labelStack.size() == 0) {
			if (DEBUG) printf("NO LABELS LEFT\n");
			return;
		}
		jumpStack.pop_back();
		
		int labelLine = labelStack.back();
		labelStack.pop_back();
		
		char op[10];
		char direction[50];
		char lab[50];
		sscanf(jumper.c_str(), "%s", op);
		if (DEBUG) printf(" ---> %d\n", labelLine + 1);
		sprintf(lab, "%s %d", op, labelLine + 1);
		stri t = lab;
		tempCode.at(jumperLine) = t;	
	}
}

void generateWhile() {
	int pl = tempCode.size();
	if (DEBUG) printf("Generuje while w linii %d\n", pl);	
	labeler();
}

void generateDo() {
	int pl = tempCode.size();
	if (DEBUG) printf("Generuje do w linii %d\n", pl);
	addCodeLine("JUMP");
	jumper();
	labeler();
	fixWhileLabels();
}

int generateWrite(stri a) {
	if (DEBUG) printf("Generuje write\n");
	// sprawdz czy a jest liczba

	int ret = generateP_A(a);
	if (ret != 0) return ret;
	if (variableManager.getValueOfVariable(a) == "null") return 666;

	// int a_val = 0;
	// if (isNumber(a)) {
	// 	a_val = atoi(a.c_str());
	// } else {
	// 	a_val = atoi(variableManager.getValueOfVariable(a).c_str());
	// }

	int reg_of_a = getVariableRegister(a);
	addCodeLine("LOAD " + reg_of_a);
	addCodeLine("PUT " + reg_of_a);
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
			char temp[50];
			addCodeLine("GET 0");
			sprintf(temp, "STORE 0", AvarIndex);
			variableManager.setAddressOfVariable(a, registerManager.getAccumulatorValue());
			addCodeLine(temp);
			addCodeLine("ZERO 0");
			// zwolnij akumulator
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