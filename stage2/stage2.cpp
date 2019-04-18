#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <stack>
#include <cstdlib>
#include <fstream>
#include <ctime>
#include <sstream>

using namespace std;

const int MAX_SYMBOL_TABLE_SIZE = 256;
enum storeType {INTEGER, BOOLEAN, PROG_NAME};
enum allocation {YES,NO};
enum modes {VARIABLE, CONSTANT};
const string storeTypeStrings[] = {"INTEGER", "BOOLEAN", "PROG_NAME"};
const string allocationStrings[] = {"YES","NO"};
const string modesStrings[] = {"VARIABLE","CONSTANT"};

struct entry //define symbol table entry format
{
	string internalName;
	string externalName;
	storeType dataType;
	modes mode;
	string value;
	allocation alloc;
	int units;
};
vector<entry> symbolTable;
ifstream sourceFile;
ofstream listingFile,objectFile;
string token;
char charac;
const char END_OF_FILE = '$'; //arbitrary choice
int lineNum = 0;
int boolCount = 0;
int intCount = 0;
int beginDepth = 0;
string GetTemp();
void FreeTemp();
//paragraph says it wants this in the parser function
int tempCount;//initialize to -1 to reflect the fact we have no temporaries yet
int maxTempCount;
int loopCount = 0;
string areg = "";
const string rammTab = "      ";
const string rTbFv = "     ";
bool hold = false;
bool FALS = false;
bool ZERO = false;
bool TRU = false;

stack<string> operators;
stack<string> operands;

//Declare functions
void CreateListingHeader();
void Parser();
void CreateListingTrailer(int numErr);
void PrintSymbolTable();
void printError(string error);
void Prog();
void ProgStmt();
void Consts();
void Vars();
void BeginEndStmt();
void ConstStmts();
void VarStmts();
string Ids();
string GenInternalName(storeType inType);
vector<string> parseNames(string names);
void Insert(string externalName,storeType inType, modes inMode, string inValue,allocation inAlloc, int inUnits, string forcedIntName = "");
storeType WhichType(string name);
string WhichValue(string name);
string NextToken();
char NextChar();
bool operator==(entry lhs, string rhs);
bool IsBool(string str);
bool IsInt(string str);
bool IsKeyWord(string str);
bool IsNonKey(string str);
void ExecStmt();
void ExecStmts();
void AssignStmt();
void ReadStmt();
void WriteStmt();
void Express();
void Term();
void Expresses();
void Terms();
void Factor();
void Factors();
void Part();
void IfStmt();
void ElsePt();
void WhileStmt();
void RepeatStmt();
void NullStmt();
bool RelOp();
bool AddLevelOp();
bool MultLevelOp();

void Code(string op,string operand1="",string operand2="");
void pushOperator(string op);
string popOperator();
void pushOperand(string operand);
string popOperand();

void EmitStartCode();
void EmitEndCode();
void EmitReadCode(string operand1);
void EmitWriteCode(string operand1);
void EmitAdditionCode(string operand1,string operand2);
void EmitSubtractionCode(string operand1, string operand2);
void EmitNegationCode(string operand);
void EmitNotCode(string operand);
void EmitLessEqualCode(string operand1, string operand2);
void EmitLessThanCode(string operand1, string operand2);
void EmitGreaterEqualCode(string operand1, string operand2);
void EmitGreaterThanCode(string operand1, string operand2);
void EmitMultiplicationCode(string operand1,string operand2);
void EmitDivisionCode(string operand1,string operand2);
void EmitModCode(string operand1, string operand2);
void EmitAndCode(string operand1,string operand2);
void EmitOrCode(string operand1,string operand2);
void EmitAssignCode(string operand1,string operand2);
void EmitNotEqualsCode(string operand1,string operand2);
void EmitEqualsCode(string operand1,string operand2);
void EmitThenCode(string operand);
void EmitElseCode(string operand);
void EmitPostIfCode(string operand);
void EmitWhileCode();
void EmitDoCode(string operand);
void EmitPostWhileCode(string operand1,string operand2);
void EmitRepeatCode();
void EmitUntilCode(string operand1, string operand2);

string getIntName(string extName);

//this program is the stage0 compiler for Pascallite. It will accept
//input from argv[1], generating a listing to argv[2], and object code to
//argv[3]
int main(int argc, char **argv)
{
    sourceFile.open(argv[1]);
    listingFile.open(argv[2]);
    objectFile.open(argv[3]);
    
	CreateListingHeader();
	Parser();
	CreateListingTrailer(0);
	
	listingFile.close();
	//PrintSymbolTable();
	objectFile.close();
	
	sourceFile.close();
	
	return 0;
}

void CreateListingHeader()
{	
	lineNum++;
    time_t now = time (NULL);
    if(listingFile.is_open())
    {
        listingFile << "STAGE0:  " << "Anna Porter, Jacob Hallenberger\t" << ctime(&now);
        listingFile << setw(23) << left << "\nLINE NO." << "SOURCE STATEMENT\n\n" << setw(5) << right << lineNum << "|";
    }

	//line numbers and source statements should be aligned under the headings
}

void Parser()
{
    //initialize to -1 to reflect the fact we have no temporaries yet
    tempCount = -1;
    maxTempCount = -1;
	NextChar();
	    //charac must be initialized to the first character of the source file
	if(NextToken() != "program")
	{    
	    printError("keyword \"program\" expected");//process err
	}    	//a call to NextToken() has two effects
          	// (1) the variable, token, is assigned the value of the next token
          	// (2) the next token is read from the source file in order to make
        	// the assignment. The value returned by NextToken() is also
        	// the next token.
	Prog();
	    //parser implements the grammar rules, calling first rule
}

void CreateListingTrailer(int numErr)
{
    if(numErr == 0)
    {
        listingFile << "\n\n";
    }
	listingFile << "COMPILATION TERMINATED      " << numErr << " ERRORS ENCOUNTERED\n";
	//print "COMPILATION TERMINATED", "# ERRORS ENCOUNTERED";
}

void PrintSymbolTable()
{
	entry entr;
	ostringstream out;
	//Print header
	time_t now = time (NULL);
	out << "STAGE0:  " << "Anna Porter, Jacob Hallenberger\t" << ctime(&now) << "\nSymbol Table\n\n";
	objectFile << out.str();
	out.str("");
	for(unsigned int i=0; i<symbolTable.size(); ++i)
	{
		entr = symbolTable[i];
		out << setw(15) << left;
		if(entr.externalName.length() > 15)
		{
			out << entr.externalName.substr(0,15);
		}
		else
		{
			out << entr.externalName;
		}
		out << "  " << setw(4) << left << entr.internalName;
		out << "  " << setw(9) << right << storeTypeStrings[entr.dataType];
		out << "  " << setw(8) << right << modesStrings[entr.mode];
		out << "  " << setw(15) << right;
		if(entr.dataType == BOOLEAN)
		{
			if(entr.value == "true")
			{
				out << "1";
			}
			else if(entr.value == "false")
			{
				out << "0";
			}
			else
			{
				out << "";
			}
		}
		else
		{
			if(entr.value.length() > 15)
			{
				out << entr.value.substr(0,15);
			}
			else
			{
				out << entr.value;
			}
		}
		out << "  " << setw(3) << right << allocationStrings[entr.alloc];
		out << "  " << setw(1) << entr.units;
		objectFile << out.str() << "\n";
		out.str("");
	}
}

//we made this!
void printError(string error)
{
    if(listingFile.is_open())
    {
        listingFile << "\nError: Line " << lineNum << ": " << error << "\n\n";
        CreateListingTrailer(1);
        listingFile.close();
        //PrintSymbolTable();
        objectFile.close();
    }

	//sourceFile.close();
}

//i think this may be all we need to do
    //I think it's finished except the last if statement
void Prog() //token should be "program"
{
    if (token != "program")
    {
        printError("keyword \"program\" expected");//process err
    }
    ProgStmt();
    EmitStartCode();
    if (token == "const")
    {
        Consts();//not sure if finished (It should be done now, along with ConstStmnts, but there is an if at the end I'm iffy about)
    }
    if (token == "var")
    {
        Vars();//not sure if finished
    }
    if (token != "begin")
    {
        printError("keyword \"begin\" expected");//process err
    } 
    BeginEndStmt();
    if (charac != END_OF_FILE)//CHANGED THIS TO CHARAC FROM TOKEN
    {
        printError("no text may follow \"end\""); //process err
    }    
    EmitEndCode();
}

void ProgStmt() //token should be "program"
{
    string x;
    if (token != "program")
    {
       printError("keyword \"program\" expected"); //process err
    }
    x = NextToken();
    if (!IsNonKey(token))
    {    
        printError("program name expected"); //process err
    }
    if (NextToken() != ";")
    {
        //cout << "XX:" << "PROGSTMT" << endl;
        printError("semicolon expected"); //process err
    }
    NextToken();
    Insert(x,PROG_NAME,CONSTANT,x,NO,0);
}

void Consts() //token should be "const"
{
    if (token != "const")
    {
        printError("keyword \"const\" expected"); //process err
    }
    if (!IsNonKey(NextToken()))
    {
        printError("non-keyword identifier must follow \"const\""); //process err
    }
    ConstStmts();
}

void Vars() //token should be "var"
{
    if (token != "var")
    {
        printError("keyword \"var\" expected"); //process err 
    }
    if (!IsNonKey(NextToken()))
    {
        printError("non-keyword identifier must follow \"var\""); //process err
    }
    VarStmts();
}

void BeginEndStmt() //token should be "begin"
{
    if (token != "begin")
    {
        printError("keyword \"begin\" expected"); //process err 
    }
    ++beginDepth;
    //Does this need to call something here?
    NextToken();
	ExecStmts();
    if (token != "end")
    {
        printError("keyword \"end\" expected"); //process err
    }
    --beginDepth;
    NextToken();
    if (token != "." && token != ";")
    {
        printError("semicolon or period expected"); //process err
    }
    if(token == "." && beginDepth>0)
    {
        printError("mismatched begin-end statements or missing semicolon");
    }
    Code("end",token);
    if(token != ";"){
       NextToken(); 
    }
}


void ConstStmts() //token should be NON_KEY_ID
{
    //strings for stuff
    string x,y;
    //If token isn't a NON_KEY_ID, then trigger error
    if (!IsNonKey(token))
    {
        printError("non-keyword identifier expected");//process err
    }
    x = token;
    //The next token must be equals
    if (NextToken() != "=")
    {
        printError("\"=\" expected");//process err 
    }
    y = NextToken();
    //The next token (y) must be one of the following symbols/keywords
    if (y != "+" && !IsNonKey(y) && y != "-" && y != "not" && y != "true" && y!= "false" && !IsInt(y))
    {
        printError("token to right of \"=\" illegal");//process err
    }
    //If y is a + or - then it must be followed by an integer
    if (y == "+" || y == "-")
    {
        if(!IsInt(NextToken()))
        {
            printError("integer expected after sign");//process err 
        }
        y = y + token;
    }
    //If y is "not", it must be followed by a BOOLEAN or Non-Key ID
    if (y == "not")
    {
        NextToken();
        if(!IsBool(token) && !IsNonKey(token))
        {
            printError("boolean or non keyword identifier expected after not");//process err
        }
        //Invert the BOOLEAN that follows y
        if(token == "true" || WhichValue(token) == "true")
        {
            y = "false";
        }
        else
        {
            y = "true";
        }
    }
    //The statement must be followed by a ;
    if (NextToken() != ";")
    {
        //cout << "XX:" << "CONSTSTMTS" << endl;
        printError("semicolon expected");//process err 
    }
    //Insert the newly defined CONSTANT into the symbol table
    Insert(x,WhichType(y),CONSTANT,WhichValue(y),YES,1);
    
    //The next token must signal the beginning of the variable section, the code section
        //or define another CONSTANT
    
    x = NextToken();    //Reuse x to avoid calling NextToken() more times than needed (and because I'm too lazy to declare another variable for this)
    //cout << "Token: " << token << endl;
    if (x != "begin" && x != "var" && !IsNonKey(x))
    {
        printError("non-keyword identifier,\"begin\", or \"var\" expected");//process err
    }
    
    if (IsNonKey(x)) //THIS MIGHT NEED TO BE CHANGED TO IsNonKey(x). I'm not sure if getting the next token here will break stuff, I think it might though,according to the syntax chart
    {
        ConstStmts();
    }
}

void VarStmts() //token should be NON_KEY_ID
{
    string x,y,tkn;
    //Token must be a NON_KEY_ID
    if (!IsNonKey(token))
    {
        printError("non-keyword identifier expected");//process err
    }
    
    x = Ids();
    
    if (token != ":")
    {
        printError("\":\" expected");//process err 
    }
    
    tkn = NextToken();
    
    if(tkn != "integer" && tkn != "boolean")
    {
        printError("illegal type follows \":\"");//process err 
    }
    
    y = token;
    
    if(NextToken() != ";")
    {
        //cout << "XX:" << "VARSTMTS" << endl;
        printError("semicolon expected");//process err
    }
    
    Insert(x,WhichType(y),VARIABLE,"",YES,1);
    
    tkn = NextToken();
    if (tkn != "begin" && !IsNonKey(tkn))
    {
        printError("non-keyword identifier or \"begin\" expected");//process err
    }
    
    if (IsNonKey(token))   //I THINK THIS MIGHT BE A BUG I'm fairly sure that this if should say if(IsNonKey(token)) instead of not, but this is how the pseudocode had it, so I'm leaving it for now
    {
        VarStmts();
    }
}

void ExecStmts()
{
    ExecStmt();
    //If token is not end, then it must be another exec stmt, so continue
	if(token != "end" && token != "until" && token != ")")
	{
		ExecStmts();
	}
}

//Reads in exec statements
void ExecStmt()
{
    //cout << "XX:" << "ExecStmt: " << token << endl;
    /*if(token == ";")
    {
        NextToken();
    }*/
	//token must be a non key ID, "read", or "write" in order to be an exec statement, it can also be end if the program has looped
	if(!IsNonKey(token) && token != "read" && token != "write" && token != "end" && token != "if" && token != "while" && token != "repeat" && token != ";" && token != "begin"  && token != "until")
	{
		printError("non-keyword identifier, \"read\", \"write\", \"if\", \"while\", \"repeat\", \";\", or \"begin\" expected");
	}
	
	if(token == "read")
	{
		ReadStmt();
	}
	else if(token == "write")
	{
		WriteStmt();
	}
    else if(token == "if")
    {
        IfStmt();
    }
    else if(token == "while")
    {
        WhileStmt();
    }
    else if(token == "repeat")
    {
        RepeatStmt();
    }
    else if(token == ";")
    {
        NullStmt();
    }
    else if(token == "begin")
    {
        BeginEndStmt();
    }
	else if(token != ";" && token != "end" && IsNonKey(token))
	{
		AssignStmt();
	}
    //MIGHT WANT TO REMOVE THIS
    else if(token != "end")
    {
        printError("non-keyword identifier, \"read\", \"write\", \"if\", \"while\", \"repeat\", \";\", or \"begin\" expected");
    }
}

//This is the fun one
void AssignStmt()
{
    //cout << "XX:" << "AssignStmt: " << token << endl;
    //Assignments must begin with a non key ID
	if(!IsNonKey(token))
    {
        printError("non-keyword identifier expected");
    }
    pushOperand(token);
    //Assignments must have ":=" after the non key ID
    if(NextToken() != ":=")
    {
        printError("\":=\" expected");
    }
    pushOperator(":=");
    //Next production is express
    NextToken();
    Express();
    
    //Assignments must end with a semicolon
    if(token != ";")
    {
        //cout << "XX:" << "ASSIGNSTMT" << endl;
        printError("semicolon expected");
    }
    //Express the assignment statement
    Code(popOperator(),popOperand(),popOperand());
    //NextToken();
}

//Reads in data/variables
void ReadStmt()
{
    //cout << "XX:" << "ReadStmt: " << token << endl;
    //READ_LIST
    //If the token isn't "read", you shouldn't be here
	if(token != "read")
    {
        printError("keyword \"read\" expected");
    }
    //The next token must be a left parenthesis
    if(NextToken() != "(")
    {
        printError("\"(\" expected");
    }
    NextToken();
    string x = Ids();
    //cout << "XX:" << "Read IDS: " << x << endl;
    //token should be ")"
    if(token != ")")
    {
        printError("\",\" or \")\" expected");
    }
    
    Code("read",x,"");
    
    //End of READ_STMT must be a semicolon
    if(NextToken() != ";")
    {
        //cout << "XX:" << "READSTMT" << endl;
        printError("semicolon expected");
    }
    NextToken();
}

//writes out data
void WriteStmt()
{
    //cout << "XX:" << "WriteStmt: " << token << endl;
	//WRITE_LIST
    //If the token isn't "write", you shouldn't be here
	if(token != "write")
    {
        printError("keyword \"write\" expected");
    }
    //The next token must be a left parenthesis
    if(NextToken() != "(")
    {
        printError("\"(\" expected");
    }
    
    NextToken();
    string x = Ids();
    //cout << "XX:" << "WriteIds: " << x << " | " << token << endl;
    //token should be ")"
    if(token != ")")
    {
        printError("\",\" or \")\" expected");
    }
    
    Code("write",x);
    //NextToken();
    //End of WRITE_STMT must be a semicolon
    if(NextToken() != ";")
    {
        //cout << "XX:" << "BOB" << endl;
        printError("semicolon expected");
    }
    //cout << "XX:" << "WriteStmtEnd " << token << endl;
    NextToken();
}

void Express()
{
    //NextToken();
    Term();
    Expresses();
}

void Expresses()
{
    //cout << "XX:" << "Expresses: " << token << endl;
    //NextToken();
    //////cout << "XX:" << "ExpressesOut: " << token << endl;
    //If token is a REL_OP then code is needed
    if(RelOp())
    {
        //Push the operator onto the stack
        pushOperator(token);
        //Call Term
        NextToken();
        Term();
        //Express code to implement the operator
        Code(popOperator(),popOperand(),popOperand());
        //Call Expresses again
        Expresses();    //THIS MIGHT BE A BUG, THE SYNTAX CHART AND COMPILER STRUCTURE CONTRADICT EACH OTHER
    }
    //else token must be ")", ",", or ";"
    else if(token != ")" && token != "," && token != ";" && token != "do" && token != "then")
    {
        printError("\")\", comma, or semicolon expected");
    }
}

void Term()
{
    //NextToken();
    Factor();
    Terms();
}

void Terms()
{
    //NextToken();
    //cout << "XX:" << "Terms: " << token << endl;
    //If token is an ADD_LEVEL_OP, then code is needed
    if(AddLevelOp())
    {
        //Push the operator
        pushOperator(token);
        //Call Factor()
        NextToken();
        Factor();
        //Call code with the operator
        Code(popOperator(),popOperand(),popOperand());
        //Call Terms again
        Terms();
    }
    //If token is not a REL_OP, or ")", or ";", then there is an error
    else if(!RelOp() && token != ")" && token != ";" && token != "do" && token != "then")
    {
                //cout << "XX:" << "non-multi level operator, \")\" or semicolon expected - TERMS" << endl;

        printError("non-multi level operator, \")\" or semicolon expected");
    }
}

void Factor()
{
    //NextToken();
    Part();
    Factors();
}

void Factors()
{
    //NextToken();
    //cout << "XX:" << "Factors: " << token << endl;
    //If token is a MULTI_LEVEL_OP, then code is needed
    if(MultLevelOp())
    {
        //Push the operator onto the stack
        pushOperator(token);
        //Call Part()
        NextToken();
        Part();
        //Express code for the operator
        Code(popOperator(),popOperand(),popOperand());
        //Call Factors again
        Factors();
    }
    //If token is not a different operator, or ")", or ";", then there is an error
    else if(!RelOp() && !AddLevelOp() && token != ")" && token != ";" && token != "do" && token != "then")
    {
        //cout << "XX:" << "operator, \")\" or semicolon expected - FACTORS" << endl;
        printError("operator, \")\", \"do\", \"then\" or semicolon expected");
    }
}

void Part()
{
    //NextToken();
    //cout << "XX:" << "Part: " << token << endl;
    if(token == "not")
    {
        NextToken();
        //If token is true, push false
        if(token == "true")
        {
            pushOperand("false");
            NextToken();
        }
        //If token is false, push true
        else if(token == "false")
        {
            pushOperand("true");
            NextToken();
        }
        //If token is a non key identifier, emit code for not NonKeyID
        else if(IsNonKey(token))
        {
            Code("not", token);
            NextToken();
        }
        //If token is "(", begin another expression
        else if(token == "(")
        {
            Express();
            if(token != ")" && token != ";")
            {
                printError("\")\" expected");
            }                
        }
        //Else there is an error
        else
        {
            printError("expression, non-keyword identifier, or boolean literal expected");
        }
        
    }
    else if(token == "+")
    {
        NextToken();
        //If token is "(", begin another expression 
        if(token == "(")
        {
            Express();
            if(token != ")" && token != ";")
            {
                printError("\")\" expected");
            }
        }
        //If token is an integer or a non keyword identifier, then push it onto the stack
        else if(IsInt(token) || IsNonKey(token))
        {
            pushOperand(token);
            NextToken();
        }
        //else there is an error
        else
        {
            printError("expression, non-keyword identifier, or integer literal expected");
        }
        
    }
    else if(token == "-")
    {
        NextToken();
        //If token is "(", begin another expression 
        if(token == "(")
        {
            Express();
            if(token != ")" && token != ";")
            {
                //cout << "XX:" << "NEGATE------------------------------" << endl;
                printError("\")\" expected");
            }
            //Express code to negate the expression
            Code("neg",popOperand());
            //NextToken();
        }
        //If token is an integer then push it onto the stack after negation
        else if(IsInt(token))
        {
            ostringstream neg;
            neg << "-" << token;
            if(find(symbolTable.begin(),symbolTable.end(),neg.str()) == symbolTable.end())
                Insert(neg.str(),INTEGER,CONSTANT,neg.str(),YES,1);
            pushOperand(neg.str());
            NextToken();
        }
        //If the token is a non keyword identifier, then express code to negate it
        else if(IsNonKey(token))
        {
            Code("neg",token);
            NextToken();
        }
        //else there is an error
        else
        {
            printError("expression, non-keyword identifier, or integer literal expected");
        }
    }
    //If the token is an integer, boolean, or non keyword identifier, then push the operand onto the stack
    else if(IsInt(token) || IsBool(token) || IsNonKey(token) || token == "true" || token == "false")
    {
        if(token == "true")
        {
            pushOperand("true");
        }
        else if(token == "false")
        {
            pushOperand("false");
        }
        else
        {
            pushOperand(token);
        }
        NextToken();
    }
    //If the token is "(", then it begins yet another expression
    else if(token == "(")
    {
        NextToken();
        Express();
        if(token != ")")
        {
            printError("\")\" expected");
        }
        NextToken();
    }
    //Else an error has been encountered
    else
    {
        printError("expression, non-keyword identifier, integer or boolean literal, \"not\", \"+\", or \"-\", expected");
    }
    //NextToken();
}

//STAGE 2 PRODUCTIONS
void IfStmt(){
    //cout << "XX:" << "IfStmt : " << token << endl;
    if(token != "if"){
        printError("Keyword \"if\" expected");
    }
    NextToken();
    Express();
    //Might need to change NextToken to token
    if(token != "then"){
        printError("Keyword \"then\" expected");
    }
    Code("then",popOperand());
    NextToken();
    ExecStmt();
    if(token == ";"){
        NextToken();
    }
    ElsePt();
}

void ElsePt(){
    //cout << "XX:" << "ElsePt : " << token << endl;
    if(token == "else"){
        Code("else",popOperand());
        NextToken();
        ExecStmt();
    }
    else
    {
        //NextToken();
    }
    Code("post_if",popOperand());
}

void WhileStmt(){
    //cout << "XX:" << "WhileStmt : " << token << endl;
    if(token != "while"){
        printError("keyword \"while\" expected");
    }
    Code("while");
    NextToken();
    if(token == "(")
    {
        //NextToken();
    }
    Express();
    //cout << "XX:" << "WhileStmtDo : " << token << endl;
    if(token != "do"){
        printError("keyword \"do\" expected");
    }
    Code("do",popOperand());
    NextToken();
    ExecStmt();
    Code("post_while",popOperand(),popOperand());
}

void RepeatStmt(){
    if(token != "repeat"){
        printError("keyword \"repeat\" expected");
    }
    Code("repeat");
    NextToken();
    ExecStmts();
    if(token != "until"){
        printError("keyword \"until\" expected");
    }
    NextToken();
    if(token == "(")
    {
        NextToken();
    }
    
    Express();
    //cout << "XX:" << "Until : " << token << endl;
    Code("until",popOperand(),popOperand());
    if(token == ")"){
        NextToken();
    }
    if(token != ";"){
        printError("semicolon expected");
    }
}

void NullStmt(){
    if(token != ";"){
        printError("semicolon expected");
    }
    NextToken();
}

//True if token is a REL_OP false if not
bool RelOp()
{
    return (token == "=" || token == "<>" || token == "<=" || token == ">=" || token == "<" || token == ">");
}

//True if token is a ADD_LEVEL_OP false if not
bool AddLevelOp()
{
    return (token == "+" || token == "-" || token == "or");
}

//True if token is a MULTI_LEVEL_OP false if not
bool MultLevelOp()
{
    return (token == "*" || token == "div" || token == "mod" || token == "and");
}

//So this recursively collects every Id after a variable declaration
string Ids() //token should be NON_KEY_ID
{
    string temp, tempString;
    //Token must be a NON_KEY_ID
    if (!IsNonKey(token))
    {
        printError("non-keyword identifier expected");//process err
    }
    tempString = token;
    temp = token;
    //If there are more Ids, continue
    if(NextToken() == ",")
    {
        //The Id must be a NON_KEY_ID
        if (!IsNonKey(NextToken()))
        {
            printError("non-keyword identifier expected");//process err
        }
        tempString = temp + "," + Ids();
    }
    return tempString;
}

string GenInternalName(storeType inType)
{
    ostringstream out;
	if(inType == BOOLEAN)
    {
        out << "B";
        out << boolCount;
        boolCount++;
    }
    else if(inType == INTEGER)
    {
        out << "I";
		out << intCount;
        intCount++;
    }
	else if(inType == PROG_NAME)
	{
		out << "P";
		out << 0;
	}
    return out.str();
}

vector<string> parseNames(string names)
{
	vector<string> outnames;
	string name;
	for(uint i=0; i<names.length(); ++i)
	{
		if(names.at(i)==',')
		{
			outnames.push_back(name);
			name = "";
		}
		else
		{
			name += names.at(i);
		}
	}
    //cout << name << ":";
    if(name.length() >= 15)
    {
        name = name.substr(0,15);
    }
    //cout << name << "\n";
	outnames.push_back(name);
	return outnames;
}

//create symbol table entry for each identifier in list of external names
//Multiply inserted names are illegal
void Insert(string externalName,storeType inType, modes inMode, string inValue,
allocation inAlloc, int inUnits, string forcedIntName)
{
	vector<string> nameEntries;
	nameEntries = parseNames(externalName);
	//(name broken from list of external names and put into name != "")
	for(unsigned int i=0; i<nameEntries.size(); ++i)
	{	
		//symbolTable[name] is defined
    	if(find(symbolTable.begin(),symbolTable.end(),nameEntries[i].substr(0,15)) != symbolTable.end())
	    {
	        printError("multiple name definition");//process err
	    }
	    else if(nameEntries[i] != "true" && nameEntries[i] != "false" && IsKeyWord(nameEntries[i]))
	    {
            //cout << "XX:" << "INSERT: " << externalName << endl;
	        printError("illegal use of keyword");//process err
	    }
        else if(symbolTable.size() > 255)
        {
            printError("symbol table overflow");
        }
	    else //create table entry
        {
			entry newent;
			newent.externalName = nameEntries[i].substr(0,15);
			newent.dataType = inType;
			newent.mode = inMode;
            //Special cases for boolean
            if(inValue == "true" || inValue == "yes")
            {
                newent.value = "1";
            }
            else if(inValue == "false" || inValue == "no")
            {
                newent.value = "0";
            }
            else
            {
                newent.value = inValue;
            }
			newent.alloc = inAlloc;
			newent.units = inUnits;
            if(forcedIntName == "ZERO" || forcedIntName == "TRUE" || forcedIntName == "FALS")
            {
                newent.internalName = forcedIntName;
            }
            else
            {   
                //symbolTable[name]=(name,inType,inMode,inValue,inAlloc,inUnits)
                if(isupper(nameEntries[i].at(0)))
                {
                    newent.internalName = nameEntries[i];
                }
                //symbolTable[name]=(GenInternalName(inType),inType,inMode,inValue,inAlloc,inUnits)
                else
                {
                    newent.internalName = GenInternalName(inType);
                }
            }
            symbolTable.push_back(newent);
        }
	}
}

storeType WhichType(string name) //tells which data type a name has
{
	storeType type;
    if (IsBool(name) || IsInt(name) || name == "boolean" || name == "integer")
    {
        if(name == "true" || name == "false" || name == "boolean")
		{
			type = BOOLEAN;
		}
        else
		{
			type = INTEGER;
		}
    }
    else //name is an identifier and hopefully a constant
    {
		//vector<entry>::iterator it;
		//it = find(symbolTable.begin(),symbolTable.end(),name);
        if(find(symbolTable.begin(),symbolTable.end(),name) != symbolTable.end())
		{
			type = (*find(symbolTable.begin(),symbolTable.end(),name)).dataType; //type = (*it).dataType;
		}		/*symbolTable[name] is defined then data type = type of symbolTable[name]*/
        else
        {
            //cout << "XX:" << "WHICHTYPE: " << name << endl;
            printError("reference to undefined constant or variable");//process err
        }
    }
    return type;
}

string WhichValue(string name) //tells which value a name has
{
	string value;
    if(IsBool(name) || IsInt(name))
	{
		value = name;
	}
    else //name is an identifier and hopefully a constant
    {
		/*symbolTable[name] is defined and has a value*/
        if(find(symbolTable.begin(),symbolTable.end(),name) != symbolTable.end())
        {
			value = (*find(symbolTable.begin(),symbolTable.end(),name)).value;
            /*value = value of symbolTable[name]*/
        }
        else
        {
            //cout << "XX:" << "WHICHVALUE: " << name << endl;
            printError("reference to undefined constant or variable");//process err
        }
    }
    return value;
}

//Last Edited 11/10 Anna
string NextToken() //returns the next token or end of file marker
{
    token = "";
    while (token == "")
    {
        if (charac == '{')
		{ //process comment
			char c = NextChar();
            while (c != END_OF_FILE && c != '}')//<--THIS WAS A CURLY BRACE MATCHING A PARENTHESIS AND IDKY BUT I CHANGED IT
            {
					c = NextChar(); 
            }
            if (charac == END_OF_FILE)
			{
				printError("unexpected end of file");//process err
			}
			NextChar();
        }
        else if (charac == '}')
		{
			token += END_OF_FILE;
			printError("'}' cannot begin token");//process error: 
		}
        else if (iswspace(charac))
		{
			NextChar();
		}
		else if (charac == ':' || charac == ',' || charac == ';' || charac == '=' || charac == '+' || charac == '-' || charac == '.' || charac == '<' || charac == '>' || charac == '(' || charac == ')' || charac == '*')
		{
			token = charac;
			NextChar();
			if((token == ":" || token == "<" || token == ">") && (charac == '=' || charac == '>'))
			{
				if(charac == '=' || (token == "<" && charac == '>'))
				{
					token += charac;
					NextChar();
				}
			}
		}
        else if (isalpha(charac) && islower(charac))
		{
			char x;
			token = "";
			do
			{
				token += charac;
				x = NextChar();
			
			} while (isalnum(x) || x == '_');
				
			if (*(token.end()-1) == '_')
			{
				printError("'_' cannot end token");
			}
		}
        else if (isdigit(charac)) 
		{ 
			token = charac;
            while (isdigit(NextChar()))
                token+=charac;
        }
		else if (charac == END_OF_FILE)
		{
			token = charac;
		}
		else
		{
            token = "ERROR";
			printError ("illegal symbol");
		}
	}
    return token;
}

//Last edited: 11/10 Anna
char NextChar() //returns the next character or end of file marker
{
    //read in next character
	if(sourceFile.is_open())
	{
        sourceFile.get(charac);
		if (sourceFile.eof())/*end of file*/
		{
			charac = '$';
			sourceFile.close();
			//cout << "EOF" << endl;//END_OF_FILE     //use a special character to designate end of file
		}
		else
		{
			//sourceFile.get(charac);
            if(hold)
            {
                hold = false;
                //cout << "A\n";
                //while(charac == '\n')
                //{
                    //sourceFile.get(charac);
                    if(sourceFile.fail())
                    {
                        charac = '$';
                        //listingFile << endl << endl;
                        sourceFile.close();
                    }
                    else
                    {
                        lineNum++;
                        if(charac != '\n')
                        {
                            listingFile << "\n" << setw(5) << right << lineNum << "|" << charac;
                        }
                        else
                        {
                            listingFile << "\n" << setw(5) << right << lineNum << "|";
                            hold = true;
                        }
                    }
                //}
            }
            else if(charac == '\n' && !hold)
            {
                hold = true;
            }
            else
            {
                //cout << "O\n";
                listingFile << charac;
            }
            
			//listingFile << charac;
		}
	}

	//listingFile << charac;	/*charac = next character*/
	/*if(sourceFile.is_open() && charac == '\n')
	{
        cout << "B";
		lineNum++;
		listingFile << setw(5) << right << lineNum << "|";
        
	}*/
    return charac;
}

//Last edited: 11/10 Jacob
//Overloads == to work with the tableEntries
bool operator==(entry lhs, string rhs)
{
	return lhs.externalName == rhs;
}

//Last edited: 11/9 Jacob
//Checks to see if str is a BOOLEAN, and returns true if it is
bool IsBool(string str)
{
    return str == "true" || str == "false";
}

//Last edited: 11/9 Jacob
//Checks to see if str is an INTEGER, and returns true if it is
bool IsInt(string str)
{
	int count = 0;
	if(str.length() > 0)
	{
		if(!isdigit(str.at(0)))
		{
			if(str.at(0) != '-' && str.at(0) != '+')
			{
				return false;
			}
			count++;
		}
		
	}
	//Check to see if the rest of str is made of numbers
	for(unsigned int i = count; i<str.length(); ++i)
	{	
		//Check to see if str is made entirely of numbers, if not, then return false
		if(!isdigit(str.at(i)))
		{
			//Then the string is not a valid INTEGER
			return false;
		}
	}
	
	//If this point is reached, str should be a valid INTEGER with only numbers
	return true;
}

//Last edited: 11/9 Jacob
//Checks to see if str is a Pascalite keyword, and returns true if it is
bool IsKeyWord(string str)
{
    return (str == "program" || str == "begin" || str == "end" || str == "var" || str == "const" || str == "integer" || str == "boolean" || str == "true" || str == "false" || str == "not" || str == "mod" || str == "div" || str == "and" || str == "or" || str == "read" || str == "write" || str == "if" || str == "then" || str == "else" || str == "repeat" || str == "while" || str == "do" || str == "until" || str == "case" || str == "of" || str = "for" || str == "to" || str == "downto");
}

//Last edited: 11/9 Jacob H
//Checks to see if str is a NON_KEY_ID and returns true if it is and false if it is not
bool IsNonKey(string str)
{
	//A NON_KEY_ID must not be a keyword and the first char of a NON_KEY_ID must be a lowercase letter and not an underscore 
	if(IsKeyWord(str) || str.length() < 1 || !islower(str.at(0)) || str.at(0) == '_')
	{
		return false;
	}
	
	//Then check to see if the rest of the str is alphanum
	for(unsigned int i=1; i<str.length(); ++i)
	{
		//If the current index is an underscore
		//it MUST be followed by a letter or number
		if(str.at(i)=='_')
		{
			++i;	//Increment the index
			
			//Check to ensure that the new index isn't out of bounds
			if(i >= str.length())
			{
				//Then the string has ended with _ and is thus invalid
				return false;
			}
		}
		
		//Else str has a different char next
		//Then another char exists, so check the see if it is alphanum
		if(!islower(str.at(i)) && !isdigit(str.at(i)))
		{
			//Then the string is not a valid alphanum
			return false;
		}
	}
	
	//If this point is reached, str should be a valid NON_KEY_ID, with only letters, numbers, and underscores
	return true;
}

//Moved stack declarations to the top
void pushOperator(string op)
{
    //cout << "XX:" << "PushOperator : " << op << "+++++++---" << endl;
	operators.push(op);
}

string popOperator()
{
    //if the operators aren't empty, then pop from the stack
    if(!operators.empty())
    {
        string top = operators.top();
        operators.pop();
        //cout << "XX:" << "PopOperator : " << top << "------+" << endl;
        return top;
    }
    

    else
        printError("operator stack underflow");
        
    return "ERROR";
}

void pushOperand(string op)
{
    //cout << "XX:" << "PushOperand : " << op << " ++++++" << endl;
    if(op!= "" && op[0] != 'L')
    {
        if(op != "" && op[0] != 'L' && find(symbolTable.begin(),symbolTable.end(),op) == symbolTable.end()) //has no symbol entry
        {
            //Special cases to handle booleans
            if(op == "true")
            {
                Insert(op,BOOLEAN,CONSTANT,"1",YES,1);
            }
            else if(op == "false")
            {
                Insert(op,BOOLEAN,CONSTANT,"0",YES,1);
            }
            else
            {
                //Insert the literal into the table
                Insert(op,WhichType(op),CONSTANT,op,YES,1);
            }

        }
    }

	//if a name is a literal and has no symbol table entry.
	operands.push(op);
}

string popOperand()
{
    
	if(!operands.empty())
	{
	    string top = operands.top();
	    operands.pop();
        //cout << "XX:" << "popOperand : " << top << " -------" << endl;
		return top;
	}
    else
    {
        printError("operand stack underflow");
    }
    
	return "ERROR";
}
//Giant block of all the operators
void Code(string op, string operand1, string operand2)
{
    //cout << "XX:" << "Code: " << op << " " << operand1 << " " << operand2 << endl;
    //if op is program, then emit the starting code
    if(op == "program")
    {
        //EmitStartCode();
    }
    else if(op == "end")
    {
        //If the end of the program, then emit the end code
        if(operand1 == ".")
        {
            //EmitEndCode();
        }    
    }
    else if(op == "read")
    {
		//think this needs to read in an operand
        EmitReadCode(operand1);
    }
    else if(op == "write")
    {
		//i think this also needs an operand input
        EmitWriteCode(operand1);
    }
    else if(op == "+")
    {
        //Test for unary or binary operator
        if(operand2 == "")
            return;//EmitUniaryAddCode(operand1); Do nothing because this function does nothing
        else
            EmitAdditionCode(operand1,operand2);
    }
    else if(op == "-")
    {
        if(operand2 == "")
            EmitNegationCode(operand1);
        else
            EmitSubtractionCode(operand1,operand2);
    }
    else if(op == "neg")
    {
        EmitNegationCode(operand1);
    }
    else if(op == "not")
    {
        EmitNotCode(operand1);
    }
    else if(op == "*")
    {
        EmitMultiplicationCode(operand1,operand2);
    }
    else if(op == "div")
    {
        EmitDivisionCode(operand1,operand2);
    }
    else if(op == "mod")
    {
        EmitModCode(operand1,operand2);
    }
    else if(op == "and")
    {
        EmitAndCode(operand1,operand2);
    }
    else if(op == "or")
    {
        EmitOrCode(operand1,operand2);
    }
    else if(op == "=")
    {
        EmitEqualsCode(operand1,operand2);
    }
    else if(op == "<>")
    {
        EmitNotEqualsCode(operand1,operand2);
    }
    else if(op == "<=")
    {
        EmitLessEqualCode(operand1,operand2);
    }
    else if(op == ">=")
    {
        EmitGreaterEqualCode(operand1,operand2);
    }
    else if(op == "<")
    {
        EmitLessThanCode(operand1,operand2);
    }
    else if(op == ">")
    {
        EmitGreaterThanCode(operand1,operand2);
    }
    else if(op == ":=")
    {
        EmitAssignCode(operand1,operand2);
    }
    else if(op == "then")
    {
        EmitThenCode(operand1);
    }
    else if(op == "else")
    {
        EmitElseCode(operand1);
    }
    else if(op == "post_if")
    {
        EmitPostIfCode(operand1);
    }
    else if(op == "while")
    {
        EmitWhileCode();
    }
    else if(op == "do")
    {
        EmitDoCode(operand1);
    }
    else if(op == "post_while")
    {
        EmitPostWhileCode(operand1, operand2);
    }
    else if(op == "repeat")
    {
        EmitRepeatCode();
    }
    else if(op == "until")
    {
        EmitUntilCode(operand1, operand2);
    }
    else
    {
        printError("undefined operation");
    }
}
//Anna 11/28
//custom thing, gets passed external name and returns the internal name
string getIntName(string extName)
{
	string intName;
	//just in case this is called with an internal name, this if will check to see if its an internal and just return it.
    if(extName == "")
    {
        return "EMPTY";
    }
	if(extName.size() == 2 && isupper(extName[0]) && isdigit(extName[1]))
	{
		return extName;
	}
	if(find(symbolTable.begin(),symbolTable.end(),extName) != symbolTable.end())
    {
        intName = (*find(symbolTable.begin(),symbolTable.end(),extName)).internalName;
    }
	else
	{
		printError("reference to an undefined constant or variable");
	}
	return intName;
}

//----------------------------EMIT STATEMENTS-----------------------------------
void EmitEndCode()
{
    entry entr;
    //Output halt
    objectFile << rammTab << "HLT     " << rTbFv << endl;
    //Output symbols
	for(unsigned int i=0; i<symbolTable.size(); ++i)
	{
		entr = symbolTable[i];
        //If the entry should be saved
        if(entr.alloc == YES)
        {
            //Output the internal name
            objectFile << setw(4) << left << entr.internalName << right << "  ";
            
            //Determine if a BSS or DEC is needed
            if(entr.mode == CONSTANT)
            {
                objectFile << "DEC ";
                //If type is boolean, output will be different than if it is integer
                if(entr.dataType == BOOLEAN)
                {
                    //If entr is true, then value is "0001", if false "0000"
                    objectFile << "000" << entr.value;
                }
                else
                {
                    //Check if the integer is negative or not
                    if(entr.value.at(0) == '-')
                    {
                        objectFile << "-" << setfill('0') << setw(3) << entr.value.substr(1) << setfill(' ');
                    }
                    else
                    {
                        objectFile << setfill('0') << setw(4) << entr.value << setfill(' ');
                    }
                }
                objectFile << rTbFv;
            }
            else
            {
                objectFile << "BSS 0001" << rTbFv;
            }
            
            //Comment with external name
            objectFile << entr.externalName << endl;
        }
    }
    //Output END
    objectFile << rammTab << "END STRT" << rTbFv << endl;
}

void EmitNegationCode(string operand1)
{
    string intOp1, intAreg; //internal names for operand 1 and 2 and areg, and a variable for holding temps.
    intAreg = getIntName(areg);
    intOp1 = getIntName(operand1);
	if(WhichType(operand1) != INTEGER)
	{
	    printError("neg operator requires integer operand");   
	}
	if(!ZERO)
	{
		Insert("ZERO", INTEGER, CONSTANT, "0", YES, 1);
		ZERO = true;
	}
    //A Register holds a temp not operand1 nor operand2 then deassign it
	if (intAreg[0] == 'T' && intAreg != intOp1)
	{
	    //emit code to store that temp into memory
		objectFile << rammTab << "STA " << setw(4) << left << intAreg << right << rTbFv << "deassign AReg" <<endl;
		//change the allocate entry for the temp in the symbol table to yes
	    (*find(symbolTable.begin(),symbolTable.end(),areg)).alloc = YES;
		areg = "";
		intAreg = "";
	}
    //A register holds a non-temp not operand1 nor operand2 then deassign it
	if (intAreg[0] != 'T' && intAreg != intOp1)
	{
        areg = "";
        intAreg = "";
	}
    if(operand1.at(0) == 'T')
        FreeTemp();
    areg = GetTemp();
	objectFile << rammTab << "LDA ZERO" << rTbFv << endl;
	objectFile << rammTab << "ISB " << setw(4) << left << intOp1 << right << rTbFv << "-" << operand1 << endl;
    pushOperand(areg);
}

void EmitReadCode(string operand1)
{
	vector<string> nameEntries;
	nameEntries = parseNames(operand1);
	//(name broken from list of external names and put into name != "")
	for(unsigned int i=0; i<nameEntries.size(); ++i)
	{
        string intOp = getIntName(nameEntries[i]);
        //If trying to read into a const
        if((*find(symbolTable.begin(),symbolTable.end(),nameEntries[i])).mode == CONSTANT)
        {
            string er = "reading in of read-only location \"" + nameEntries[i] + "\"";
            printError(er);
        }
        
        objectFile << rammTab << "RDI " << setw(4) << left << intOp << right << rTbFv << "read(" << nameEntries[i] << ")" << endl;
	}
}

void EmitWriteCode(string operand1)
{
	vector<string> nameEntries;
	nameEntries = parseNames(operand1);
	//(name broken from list of external names and put into name != "")
	for(unsigned int i=0; i<nameEntries.size(); ++i)
	{
		string intOp = getIntName(nameEntries[i]);
		objectFile << rammTab << "PRI " << setw(4)<< left << intOp << right << rTbFv << "write(" << nameEntries[i] << ")" << endl;
        areg = nameEntries[i];
	}
}

void EmitAdditionCode(string operand1,string operand2) //add operand1 to operand2
{
    string intOp1, intOp2, intAreg, temporary; //internal names for operand 1 and 2 and areg, and a variable for holding temps.
    intAreg = getIntName(areg);
    intOp1 = getIntName(operand1);
    intOp2 = getIntName(operand2);
    
	if (WhichType(operand1) != INTEGER || WhichType(operand2) != INTEGER) 
	{
		printError("operator + requires integer operands");
	}
	
	//A Register holds a temp not operand1 nor operand2 then deassign it
	if (intAreg[0] == 'T' && (intAreg != intOp1 && intAreg != intOp2))
	{
	    //emit code to store that temp into memory
		objectFile << rammTab << "STA " << setw(4) << left << intAreg << right << rTbFv << "deassign AReg" <<endl;
		//change the allocate entry for the temp in the symbol table to yes
	    (*find(symbolTable.begin(),symbolTable.end(),areg)).alloc = YES;
		areg = "";
		intAreg = "";
	}
    //A register holds a non-temp not operand1 nor operand2 then deassign it
	if (intAreg[0] != 'T' && (intAreg != intOp1 && intAreg != intOp2))
	{
        areg = "";
        intAreg = "";
	}
	
	//if neither operand is in A register then
	if(intAreg[0] != 'T' && intAreg != intOp1 && intAreg != intOp2)
	{
	    //emit code to load operand2 into A register
	    objectFile << rammTab << "LDA " << setw(4) << left << left <<intOp2 << right << right <<  rTbFv << endl;
		intAreg = intOp2;
	}
	
	//emit code to perform register-memory addition
	if(intAreg == intOp2)
	{
		objectFile << rammTab << "IAD " << setw(4) <<  left << intOp1 << right <<  rTbFv << operand2 << " + " << operand1 << endl;
	}
	else if (intAreg == intOp1)
	{
		objectFile << rammTab << "IAD " << setw(4) <<  left << intOp2 << right <<  rTbFv << operand2 << " + " << operand1 << endl;
	}
	//deassign all temporaries involved in the addition and free those names for reuse
    //if operator 1 and/or 2 were a temp, free them
    if(operand1[0] == 'T')
    {
        FreeTemp();
    }
    if(operand2[0] == 'T')
    {
        FreeTemp();
    }	
    //A Register = next available temporary name and change type of its symbol table entry to integer
    areg = GetTemp();
    (*find(symbolTable.begin(),symbolTable.end(),areg)).dataType = INTEGER;
    pushOperand(areg);
	//push the name of the result onto operandStk
}


void EmitSubtractionCode(string operand1, string operand2)
{
	string intAreg = getIntName(areg);
    string intOp1 = getIntName(operand1);
    string intOp2 = getIntName(operand2);
    
	if (WhichType(operand1) != INTEGER || WhichType(operand2) != INTEGER) 
	{
		printError("operator - requires integer operands");
	}
	
	//A Register holds a temp not operand1 nor operand2 then deassign it
	if (intAreg[0] == 'T' && intAreg != intOp2)
	{
	    //emit code to store that temp into memory
		objectFile << rammTab << "STA " << setw(4) << left << intAreg << right << rTbFv << "deassign AReg" <<endl;
		//change the allocate entry for the temp in the symbol table to yes
	    (*find(symbolTable.begin(),symbolTable.end(),areg)).alloc = YES;
		areg = "";	
		intAreg = "";
	}
    //A register holds a non-temp not operand1 nor operand2 then deassign it
	if (intAreg[0] != 'T' && intAreg != intOp2)
	{
	    areg = "";
	    intAreg = "";
	}
	
	//operand 2 is not in the A register then load it
	if(intAreg != intOp2)
	{
	    //emit code to load operand2 into A register
	    objectFile << rammTab << "LDA " << setw(4) << left <<intOp2 << right << rTbFv << endl;
	}
	//emit code to perform register-memory subtraction
	objectFile << rammTab<< "ISB " << setw(4) << left <<intOp1 << right << rTbFv << operand2 << " - " << operand1 << endl;
	//deassign all temporaries involved in the addition and free those names for reuse
	//if operator 1 and/or 2 were a temp, free them
    if(operand1[0] == 'T')
    {
        FreeTemp();
    }
    if(operand2[0] == 'T')
    {
        FreeTemp();
    }	
    //A Register = next available temporary name and change type of its symbol table entry to integer
    areg = GetTemp();
    (*find(symbolTable.begin(),symbolTable.end(),areg)).dataType = INTEGER;
    pushOperand(areg);
	//push the name of the result onto operandStk
}

void EmitDivisionCode(string operand1,string operand2) //divide operand2 by operand1
{
    //cout << "XX:" << " EmitDivision: " << operand2 << "/" << operand1 << endl;
    string intOp1, intOp2, intAreg; //internal names for operand 1 and 2 and areg
    intAreg = getIntName(areg);
    intOp1 = getIntName(operand1);
    intOp2 = getIntName(operand2);
    //cout << "XX:" << " EmitDivisionInternals: " << "areg:" << intAreg << " " << intOp2 << "/" << intOp1 << endl;
    //if either operand is not integer
	if (WhichType(operand1) != INTEGER || WhichType(operand2) != INTEGER) 
	{
		printError("operator div requires integer operands");
	}

    //if A Register holds a temp not operand2 
    if(intAreg[0] == 'T' && intAreg != intOp2)
    {
	    //emit code to store that temp into memory
		objectFile << rammTab << "STA " << setw(4) << left << intAreg << right << rTbFv << "deassign AReg" <<endl;
		//change the allocate entry for the temp in the symbol table to yes
	    (*find(symbolTable.begin(),symbolTable.end(),areg)).alloc = YES;
		areg = "";
		intAreg = "";
    }
    //if A register holds a non-temp not operand2 then deassign it
    if(intAreg[0] != 'T' && intAreg != intOp2)
    {
        areg = "";
        intAreg = "";
    }
    //if operand2 is not in A register
    if (intAreg != intOp2)
    {
        //emit instruction to do a register-memory load of operand2 into the A register;
        objectFile << rammTab << "LDA " << setw(4) << left <<intOp2 << right <<rTbFv << endl;
    }
    
    // emit code to perform a register-memory division
    objectFile << rammTab << "IDV "  << setw(4) << left <<intOp1 << right << rTbFv << operand2 << " div " << operand1 << endl;
	//deassign all temporaries involved and free those names for reuse
    //if operator 1 and/or 2 were a temp, free them
    if(operand1[0] == 'T')
    {
        FreeTemp();
    }
    if(operand2[0] == 'T')
    {
        FreeTemp();
    }	
    //A Register = next available temporary name and change type of its symbol table entry to integer
    areg = GetTemp();
    (*find(symbolTable.begin(),symbolTable.end(),areg)).dataType = INTEGER;
    pushOperand(areg);
	//push the name of the result onto operandStk
}

void EmitMultiplicationCode(string operand1,string operand2) //multiply operand2 by operand1
{
    string intOp1, intOp2, intAreg; //internal names for operand 1 and 2 and areg
    intAreg = getIntName(areg);
    intOp1 = getIntName(operand1);
    intOp2 = getIntName(operand2);
    
    //if either operand is not integer
	if (WhichType(operand1) != INTEGER || WhichType(operand2) != INTEGER) 
	{
		printError("operator * requires integer operands");
	}

     //if A Register holds a temp not operand1 nor operand2 then
     if(intAreg[0] == 'T' && (intAreg != intOp1 && intAreg != intOp2 ))
    {
	    //emit code to store that temp into memory
		objectFile << rammTab << "STA " << setw(4) << left <<areg <<right  << rTbFv << "deassign AReg" <<endl;
		//change the allocate entry for the temp in the symbol table to yes
	    (*find(symbolTable.begin(),symbolTable.end(),areg)).alloc = YES;
		areg = "";
		intAreg = "";
    }
    //if A register holds a non-temp not operand2 nor operand1 then deassign it
    if(intAreg[0] != 'T' && intAreg != intOp1 && intAreg != intOp2)
    {
        areg = "";
        intAreg = "";
    }
    //if neither operand is in A register then
    if(intAreg != intOp1 && intAreg != intOp2)
    {
        //emit code to load operand2 into the A register;
        objectFile << rammTab << "LDA " << setw(4) << left << intOp2 << right << rTbFv << endl;
    	areg = intOp2;
    }
    //emit code to perform a register-memory multiplication with A Register holding the result;
    if(areg == intOp1)
	{
		//objectFile << "areg == intOp1" << endl;
		objectFile << rammTab << "IMU " << setw(4) << left << intOp2 << right << rTbFv << operand2 << " * " << operand1 << endl;
	}
	else if (areg == intOp2)
	{
		//objectFile << "areg == intOp2" << endl;
		objectFile << rammTab << "IMU " << setw(4) << left << intOp1 << right << rTbFv << operand2 << " * " << operand1 << endl;
	}
	
	//deassign all temporaries involved in and free those names for reuse
	//if operator 1 and/or 2 were a temp, free them
    if(operand1[0] == 'T')
    {
        FreeTemp();
    }
    if(operand2[0] == 'T')
    {
        FreeTemp();
    }	
    //A Register = next available temporary name and change type of its symbol table entry to integer
    areg = GetTemp();
    (*find(symbolTable.begin(),symbolTable.end(),areg)).dataType = INTEGER;
    pushOperand(areg);
	//push the name of the result onto operandStk
}

void EmitModCode(string operand1, string operand2)
{
	string intOp1, intOp2, intAreg; //internal names for operand 1 and 2 and areg
    intAreg = getIntName(areg);
    intOp1 = getIntName(operand1);
    intOp2 = getIntName(operand2);
    
	//if either operand is not integer
	if (WhichType(operand1) != INTEGER || WhichType(operand2) != INTEGER)
	{
		printError("operator mod requires integer operands");
	}

	//if A Register holds a temp not operand1 nor operand2 then
	if(intAreg[0] == 'T' && intAreg != intOp2)
	{
		//emit code to store that temp into memory
		objectFile << rammTab << "STA " << setw(4) << left << intAreg << right << rTbFv << "deassign AReg" <<endl;
		//change the allocate entry for it in the symbol table to yes
		(*find(symbolTable.begin(),symbolTable.end(),areg)).alloc = YES;
		areg = "";
		intAreg = "";
	
	}
	//if A register holds a non-temp not operand2 nor operand1 then deassign it
	if(intAreg[0] != 'T' && intAreg != intOp2)
	{
		areg = "";
		intAreg = "";
	}
	//if operand 2 is not in the register, load it in there
	if(intAreg != intOp2)
	{
	    //emit code to load operand2 into the A register;
	    objectFile << rammTab << "LDA " << setw(4) << left <<intOp2 << right << rTbFv << endl;
	}
    //divide by op1
	objectFile << rammTab << "IDV " << setw(4)<< left <<intOp1 << right << rTbFv << operand2 << " mod " << operand1 << endl;

	//A Register = next available temporary name and change type of its symbol table entry to integer
	areg = GetTemp();
	(*find(symbolTable.begin(),symbolTable.end(),areg)).dataType = INTEGER;
    (*find(symbolTable.begin(),symbolTable.end(),areg)).alloc = YES;
	//push name of result onto operandStk;
	pushOperand(areg);
	
	objectFile << rammTab << "STQ " << setw(4) << left << areg << right << rTbFv << "store remainder in memory" << endl;
	objectFile << rammTab << "LDA " << setw(4) << left << areg  << right << rTbFv << "load remainder from memory" << endl;
	
	//deassign all temporaries involved and free those names for reuse;
	if(operand1[0] == 'T')
    {
        FreeTemp();
    }
    if(operand2[0] == 'T')
    {
        FreeTemp();
    }
}

void EmitAndCode(string operand1,string operand2) //"and" operand1 to operand2
{
    string intOp1, intOp2, intAreg; //internal names for operand 1 and 2 and areg
    intAreg = getIntName(areg);
    intOp1 = getIntName(operand1);
    intOp2 = getIntName(operand2);
    
    //if either operand is not boolean
	if (WhichType(operand1) != BOOLEAN || WhichType(operand2) != BOOLEAN) 
	{
		printError("operator and requires boolean operands");
	}

    //if A Register holds a temp not operand1 nor operand2 then 
    if(intAreg[0] == 'T' && (intAreg != intOp1 && intAreg != intOp2))
	{
		//emit code to store that temp into memory
		objectFile << rammTab << "STA " << setw(4) << left <<intAreg << right << rTbFv << "deassign AReg" <<endl;
		//change the allocate entry for it in the symbol table to yes 
		(*find(symbolTable.begin(),symbolTable.end(),areg)).alloc = YES;
		areg = "";
		intAreg = "";
	}
    //if A register holds a non-temp not operand2 nor operand1 then deassign it
    if(intAreg[0] != 'T' && (intAreg != intOp1 && intAreg != intOp2))
    {
        areg = "";
        intAreg = "";
    }
    //if neither operand is in A register then    
    if(intAreg != intOp1 && intAreg != intOp2)
    {
        //emit code to load operand2 into the A register;
        objectFile << rammTab << "LDA " << setw(4) << left <<intOp2 << right << rTbFv << endl;
        areg = intOp2;
    }

    //emit code to perform a register-memory multiplication with A Register holding the result;
    if(areg == intOp2)
    {
        objectFile << rammTab << "IMU " << setw(4) << left <<intOp1  << right << rTbFv << operand2 << " and " << operand1 << endl;
    }
    else if (areg == intOp1)
    {
        objectFile << rammTab << "IMU " << setw(4) << left <<intOp2  << right << rTbFv << operand2 << " and " << operand1 << endl;
    }
    //deassign all temporaries involved and free those names for reuse;
     if(operand1[0] == 'T')
    {
        FreeTemp();
    }
    if(operand2[0] == 'T')
    {
        FreeTemp();
    }	
	//A Register is nothing useful so deassign
	areg = GetTemp(); 
	intAreg = "";
    (*find(symbolTable.begin(),symbolTable.end(),areg)).dataType = BOOLEAN;
    pushOperand(areg);
}

void EmitOrCode(string operand1, string operand2)
{
    string intOp1, intOp2, intAreg; //internal names for operand 1 and 2 and areg
    intAreg = getIntName(areg);
    intOp1 = getIntName(operand1);
    intOp2 = getIntName(operand2);
    
    //if either operand is not boolean
	if (WhichType(operand1) != BOOLEAN || WhichType(operand2) != BOOLEAN) 
	{
		printError("operator or requires boolean operands");
	}
    //If there is a temp that is not operand1 or operand 2 in the register, store it out 
    if(areg != "" && areg.at(0) == 'T' && (areg != operand1 && areg != operand2))
    {
	    //emit code to store that temp into memory
		objectFile << rammTab << "STA " << setw(4) << left << intAreg << right << rTbFv << "deassign AReg" << endl;
		//change the allocate entry for the temp in the symbol table to yes
	    (*find(symbolTable.begin(),symbolTable.end(),areg)).alloc = YES;
		areg = "";
		intAreg = "";
    }
    //If there is a non temp variable that is not an operand, then deassign areg
    if(areg != "" && areg.at(0) != 'T' && areg != operand1 && areg != operand2)
    {
        areg = "";
        intAreg = "";
    }
	//if neither operand is in A register then
	if(intAreg != intOp1 && intAreg != intOp2)
	{
	    //emit code to load operand2 into A register
	    objectFile << rammTab << "LDA " << setw(4) << left <<intOp2 << right << rTbFv << endl;
	    areg = intOp2;
	}
    //Add operand 1 and 2
    if(areg == intOp2)
    {
        objectFile << rammTab << "IAD " << setw(4) << left <<intOp1 << right << rTbFv << operand2 << " or " << operand1 << endl;
    }  
    else if(areg == intOp1)
    {
        objectFile << rammTab << "IAD " << setw(4) << left <<intOp2 << right << rTbFv << operand2 << " or " << operand1 << endl;
    }
    //if zero jump to the next lable +1
    objectFile << rammTab << "AZJ " << "L" << setw(3) << left <<  loopCount << "+1   " << endl;
    //If true is not in the symbol table, it must be added
    if(!TRU)
    {
        Insert("TRUE", BOOLEAN, CONSTANT, "1", YES, 1, "TRUE");
		TRU = true;
    }
    objectFile << "L" << setw(3) << left << loopCount << "  LDA " << "TRUE" << rTbFv << endl;
    loopCount++;
    if(operand1.at(0) == 'T')
        FreeTemp();
    if(operand2.at(0) == 'T')
        FreeTemp();
    areg = GetTemp();
    (*find(symbolTable.begin(),symbolTable.end(),areg)).dataType = BOOLEAN;
    pushOperand(areg);
}

void EmitNotCode(string operand)
{
    string intOp = getIntName(operand);
	if(WhichType(operand) != BOOLEAN)
	{
        printError("operator not requires boolean operands");

	}
    /*if(!ZERO)
    {
        Insert("ZERO", INTEGER, CONSTANT, "0", YES, 1, "ZERO");
		ZERO = true;
    }*/
    //If A doesn't contain operand, it must be loaded
    if(areg != operand)
    {
    	objectFile << rammTab << "LDA " << setw(4) << left << intOp << right << rTbFv << endl;
    }
    
    objectFile << rammTab << "AZJ " << "L" << setw(3) << left << loopCount << rTbFv << "not " << operand << endl;
    //If false does not exist, then add it to the symbol table
    if(!FALS)
    {
        Insert("FALSE", BOOLEAN, CONSTANT, "0", YES, 1, "FALS");
		FALS = true;
    }
    //Then load false 
    objectFile << rammTab << "LDA " << "FALS" << rTbFv << endl;
    //Unconditional jump to after the label
    objectFile << rammTab << "UNJ " << "L" << setw(3) << left << loopCount << "+1   " << endl;
    //Label, load true
    //If true does not exist, then add it to the symbol table
    if(!TRU)
    {
        Insert("TRUE", BOOLEAN, CONSTANT, "1", YES, 1, "TRUE");
		TRU = true;
    }
    objectFile << "L" << setw(3) << left << loopCount << "  LDA " << "TRUE" << rTbFv << endl;
    
    if(operand.at(0) == 'T')
    {
        FreeTemp();
    }
    
    areg = GetTemp();
    (*find(symbolTable.begin(),symbolTable.end(),areg)).dataType = BOOLEAN;
    pushOperand(areg);
}


void EmitEqualsCode(string operand1,string operand2) //test whether operand2 equals operand1
{
	string intOp1, intOp2, intAreg; //internal names for operand 1 and 2 and areg
    intAreg = getIntName(areg);
    intOp1 = getIntName(operand1);
    intOp2 = getIntName(operand2);
    
    //if types of operands are not the same
	if(WhichType(operand1) != WhichType(operand2))
	{
		printError("incompatible types");
	}
    //if A Register holds a temp not operand1 nor operand2 then 
    if(intAreg[0] == 'T' && (intAreg != intOp1 && intAreg != intOp2))
    {
        //emit code to store that temp into memory
    	objectFile << rammTab << "STA " << setw(4) << left << intAreg << right << rTbFv << "deassign AReg" <<endl;
        //change the allocate entry for it in the symbol table to yes 
    	(*find(symbolTable.begin(),symbolTable.end(),areg)).alloc = YES;
    	areg = "";
    	intAreg = "";
    }
    // if A register holds a non-temp not operand2 nor operand1 then deassign it
	if(intAreg[0] != 'T' && intAreg != intOp1 && intAreg != intOp2)
	{
		areg = "";
		intAreg = "";
	}
	
	if(intAreg == intOp2)
	{
		 //emit code to load operand2 into the A register;
		//objectFile << rammTab << "LDA " << setw(4) << left <<intOp1 << right << rTbFv  << endl;
		objectFile << rammTab << "ISB " << setw(4) << left <<intOp1 << right << rTbFv << operand2 << " = " << operand1 << endl;
	}
	else if(intAreg == intOp1)
	{
		//objectFile << rammTab << "LDA " << setw(4) << left <<intOp2 << right << rTbFv  << endl;
		objectFile << rammTab << "ISB " << setw(4) << left <<intOp2 << right << rTbFv << operand2 << " = " << operand1 << endl;
	}
	else
	{
		objectFile << rammTab << "LDA " << setw(4) << left <<intOp2 << right << rTbFv  << endl;
		objectFile << rammTab << "ISB " << setw(4) << left <<intOp1 << right << rTbFv << operand2 << " = " << operand1 << endl;
	}
     //emit code to perform a register-memory subtraction with A Register holding the result;
	//objectFile << rammTab << "ISB " << setw(4) << left <<intOp1 << right << rTbFv << operand2 << " = " << operand1 << endl;
     //emit code to perform an AZJ to the next available Ln
	objectFile << rammTab << "AZJ L" << setw(3) << left <<loopCount <<right <<rTbFv << endl;
    //emit code to do a register-memory load FALS
	objectFile << rammTab << "LDA FALS" << rTbFv<< endl;
    //Insert FALS in symbol table with value 0 and external name false
    if(!FALS)
    {
        Insert("FALSE", BOOLEAN, CONSTANT, "0", YES, 1, "FALS");
		FALS = true;
    }
    //emit code to perform a UNJ to the acquired label Ln +1
	objectFile << rammTab << "UNJ " << "L" << setw(3) << left <<loopCount <<right<< "+1   " << rTbFv << endl;
    //emit code to label the next instruction with the acquired label Ln
    //and do a register-memory load TRUE
    objectFile << "L" << setw(3) << left << loopCount << "  LDA " << "TRUE" << rTbFv << endl;

	loopCount++;
	if(!TRU)
    {
        Insert("TRUE", BOOLEAN, CONSTANT, "1", YES, 1, "TRUE");
		TRU = true;
    }
    //Insert TRUE in symbol table with value 1 and external name true
    //deassign all temporaries involved and free those names for reuse;
     if(operand1[0] == 'T')
    {
        FreeTemp();
    }
    if(operand2[0] == 'T')
    {
        FreeTemp();
    }	

    areg = GetTemp();
    (*find(symbolTable.begin(),symbolTable.end(),areg)).dataType = BOOLEAN;
    pushOperand(areg);
}


void EmitNotEqualsCode(string operand1,string operand2) //test whether operand2 equals operand1
{
	string intOp1, intOp2, intAreg; //internal names for operand 1 and 2 and areg
    intAreg = getIntName(areg);
    intOp1 = getIntName(operand1);
    intOp2 = getIntName(operand2);
    
    //if types of operands are not the same
	if(WhichType(operand1) != WhichType(operand2))
	{
		printError("incompatible types");
	}
    //if A Register holds a temp not operand1 nor operand2 then 
    if(intAreg[0] == 'T' && intAreg != intOp1 && intAreg != intOp2 )
    {
        //deassign it
        //emit code to store that temp into memory
    	objectFile << rammTab << "STA " << setw(4) << left << intAreg << right << rTbFv << "deassign AReg" <<endl;
        //change the allocate entry for it in the symbol table to yes 
    	(*find(symbolTable.begin(),symbolTable.end(),areg)).alloc = YES;
    	areg = "";
    	intAreg = "";
	
    }
    // if A register holds a non-temp not operand2 nor operand1 then deassign it
	if(intAreg[0] != 'T' && intAreg != intOp1 && intAreg != intOp2)
	{
		areg = "";
		intAreg = "";
	}
		if(intAreg == intOp2)
	{
		 //emit code to load operand2 into the A register;
		//objectFile << rammTab << "LDA " << setw(4) << left <<intOp1 << right << rTbFv  << endl;
		objectFile << rammTab << "ISB " << setw(4) << left <<intOp1 << right << rTbFv << operand2 << " <> " << operand1 << endl;
	}
	else if(intAreg == intOp1)
	{
		//objectFile << rammTab << "LDA " << setw(4) << left <<intOp2 << right << rTbFv  << endl;
		objectFile << rammTab << "ISB " << setw(4) << left <<intOp2 << right << rTbFv << operand2 << " <> " << operand1 << endl;
	}
	else
	{
		objectFile << rammTab << "LDA " << setw(4) << left <<intOp2 << right << rTbFv  << endl;
		objectFile << rammTab << "ISB " << setw(4) << left <<intOp1 << right << rTbFv << operand2 << " <> " << operand1 << endl;
	}

    //emit code to perform an AZJ to the next available Ln
	objectFile << rammTab << "AZJ L" << setw(3) << left <<loopCount <<right << "+1   " << endl;
	
	objectFile << "L" << loopCount << "    LDA TRUE     " << endl;

	    //Insert TRUE in symbol table with value 1 and external name false
    if(!TRU)
    {
        Insert("TRUE", BOOLEAN, CONSTANT, "1", YES, 1, "TRUE");
		TRU = true;
    }


 loopCount++;
 //deassign all temporaries involved and free those names for reuse;
     if(operand1[0] == 'T')
    {
        FreeTemp();
    }
    if(operand2[0] == 'T')
    {
        FreeTemp();
    }	
	 //A Register is garbage so deassign
    areg = GetTemp();
    (*find(symbolTable.begin(),symbolTable.end(),areg)).dataType = BOOLEAN;
    pushOperand(areg);
}
void EmitLessThanCode(string operand1, string operand2)
{
	string intOp1, intOp2, intAreg; //internal names for operand 1 and 2 and areg
    intAreg = getIntName(areg);
    intOp1 = getIntName(operand1);
    intOp2 = getIntName(operand2);
    
    //if types of operands are not the same
	if(WhichType(operand1) != WhichType(operand2))
	{
		printError("incompatible types");
	}
    //if A Register holds a temp not operand1 nor operand2 then 
    if(intAreg[0] == 'T' && (intAreg != intOp1 && intAreg != intOp2))
    {
        //emit code to store that temp into memory
    	objectFile << rammTab << "STA " << setw(4) << left << intAreg << right << rTbFv << "deassign AReg" <<endl;
        //change the allocate entry for it in the symbol table to yes 
    	(*find(symbolTable.begin(),symbolTable.end(),areg)).alloc = YES;
    	areg = "";
    	intAreg = "";
    }
    // if A register holds a non-temp not operand2 nor operand1 then deassign it
	if(intAreg[0] != 'T' && intAreg != intOp1 && intAreg != intOp2)
	{
		areg = "";
		intAreg = "";
	}
    //if neither operand is in A register then
	if(intAreg != intOp2)
	{
		//emit code to load operand2 into the A register;
		objectFile << rammTab << "LDA " << setw(4) << left <<intOp2 << right << rTbFv  << endl;
	}
	objectFile << rammTab << "ISB " << setw(4) << left <<intOp1 << right << rTbFv << operand2 << " < " << operand1 << endl;
	objectFile << rammTab << "AMJ L" << setw(3) << left <<loopCount <<right << rTbFv << endl;
	//if the result isn't negative, then its false
	objectFile << rammTab << "LDA FALS" << rTbFv << endl;
	if(!FALS)
    {
        Insert("FALSE", BOOLEAN, CONSTANT, "0", YES, 1, "FALS");
		FALS = true;
    }
	objectFile << rammTab << "UNJ " << "L" << setw(3) << left <<loopCount <<right << "+1   " << endl;
	//if the result /IS/ negative then its true
	objectFile << "L" << setw(3)<< left << loopCount << "  LDA " << "TRUE" << rTbFv << endl;
	if(!TRU)
    {
        Insert("TRUE", BOOLEAN, CONSTANT, "1", YES, 1, "TRUE");
		TRU = true;
    }
	loopCount++;
	//deassign all temporaries involved and free those names for reuse;
     if(operand1[0] == 'T')
    {
        FreeTemp();
    }
    if(operand2[0] == 'T')
    {
        FreeTemp();
    }	

    areg = GetTemp();
    (*find(symbolTable.begin(),symbolTable.end(),areg)).dataType = BOOLEAN;
    pushOperand(areg);
}

void EmitLessEqualCode(string operand1, string operand2)
{
	string intOp1, intOp2, intAreg; //internal names for operand 1 and 2 and areg
    intAreg = getIntName(areg);
    intOp1 = getIntName(operand1);
    intOp2 = getIntName(operand2);
    
    //if types of operands are not the same
	if(WhichType(operand1) != WhichType(operand2))
	{
		printError("incompatible types");
	}
 //if A Register holds a temp not operand1 nor operand2 then 
    if(intAreg[0] == 'T' && (intAreg != intOp1 && intAreg != intOp2))
 {
    //emit code to store that temp into memory
	objectFile << rammTab << "STA " << setw(4) << left << areg << right << rTbFv << "deassign AReg" <<endl;
    //change the allocate entry for it in the symbol table to yes 
	(*find(symbolTable.begin(),symbolTable.end(),areg)).alloc = YES;
	areg = "";
	intAreg = "";
 }
// if A register holds a non-temp not operand2 nor operand1 then deassign it
	if(intAreg[0] != 'T' && intAreg != intOp1 && intAreg != intOp2)
	{
		areg = "";
		intAreg = "";
	}
 //if neither operand is in A register then
	if(intAreg != intOp1 && intAreg != intOp2)
	{
		//emit code to load operand2 into the A register;
		objectFile << rammTab << "LDA " << setw(4) << left <<intOp2 << right << rTbFv  << endl;
		areg = intOp2;
	}
	if(areg == intOp1)
	{
	    objectFile << rammTab << "ISB " << setw(4) << left <<intOp2 << right << rTbFv << operand2 << " <= " << operand1 << endl;
	}
	else if(areg == intOp2)
	{
	    objectFile << rammTab << "ISB " << setw(4) << left <<intOp1 << right << rTbFv << operand2 << " <= " << operand1 << endl;
	}

	objectFile << rammTab << "AMJ L" << setw(3) << left <<loopCount <<right << rTbFv << endl;
	objectFile << rammTab << "AZJ L" << setw(3) << left <<loopCount <<right << rTbFv << endl;
	//if the result isn't negative, then its false
	objectFile << rammTab << "LDA FALS" << rTbFv << endl;
	if(!FALS)
    {
        Insert("FALSE", BOOLEAN, CONSTANT, "0", YES, 1, "FALS");
		FALS = true;
    }
	objectFile << rammTab << "UNJ " << "L" << setw(3) << left <<loopCount <<right << "+1   " << endl;
	//if the result /IS/ negative then its true
	objectFile << "L" << setw(3) << left << loopCount << "  LDA " << "TRUE" << rTbFv << endl;
	if(!TRU)
    {
        Insert("TRUE", BOOLEAN, CONSTANT, "1", YES, 1, "TRUE");
		TRU = true;
    }
	loopCount++;
	//deassign all temporaries involved and free those names for reuse;
     if(operand1[0] == 'T')
    {
        FreeTemp();
    }
    if(operand2[0] == 'T')
    {
        FreeTemp();
    }	
    areg = GetTemp();
    (*find(symbolTable.begin(),symbolTable.end(),areg)).dataType = BOOLEAN;
    pushOperand(areg);
}

void EmitGreaterEqualCode(string operand1, string operand2)
{
	string intOp1, intOp2, intAreg; //internal names for operand 1 and 2 and areg
    intAreg = getIntName(areg);
    intOp1 = getIntName(operand1);
    intOp2 = getIntName(operand2);
    
    //if types of operands are not the same
	if(WhichType(operand1) != WhichType(operand2))
	{
		printError("incompatible types");
	}
    //if A Register holds a temp not operand1 nor operand2 then 
    if(intAreg[0] == 'T' && (intAreg != intOp1 && intAreg != intOp2))
    {
        //emit code to store that temp into memory
    	objectFile << rammTab << "STA " << setw(4) << left << areg << right << rTbFv << "deassign AReg" <<endl;
        //change the allocate entry for it in the symbol table to yes 
    	(*find(symbolTable.begin(),symbolTable.end(),areg)).alloc = YES;
    	areg = "";
    	intAreg = "";
    }
    // if A register holds a non-temp not operand2 nor operand1 then deassign it
	if(intAreg[0] != 'T' && intAreg != intOp1 && intAreg != intOp2)
	{
		areg = "";
		intAreg = "";
	}
    //if neither operand is in A register then
	if(intAreg != intOp2)
	{
		//emit code to load operand2 into the A register;
		objectFile << rammTab << "LDA " << setw(4) << left <<intOp2 << right << rTbFv  << endl;
	}
	objectFile << rammTab << "ISB " << setw(4) << left <<intOp1 << right << rTbFv << operand2 << " >= " << operand1 << endl;
	objectFile << rammTab << "AMJ L" << setw(3) << left <<loopCount <<right << rTbFv << endl;
	//if the result isn't negative, then its false
	objectFile << rammTab << "LDA TRUE" << rTbFv << endl;
	if(!FALS)
    {
        Insert("FALSE", BOOLEAN, CONSTANT, "0", YES, 1, "FALS");
		FALS = true;
    }
	objectFile << rammTab << "UNJ " << "L" << setw(3) << left <<loopCount <<right << "+1   " << endl;
	//if the result /IS/ negative then its true
	objectFile << "L" << setw(3) << left << loopCount << "  LDA " << "FALS" << rTbFv << endl;
	if(!TRU)
    {
        Insert("TRUE", BOOLEAN, CONSTANT, "1", YES, 1, "TRUE");
		TRU = true;
    }
	loopCount++;
	//deassign all temporaries involved and free those names for reuse;
     if(operand1[0] == 'T')
    {
        FreeTemp();
    }
    if(operand2[0] == 'T')
    {
        FreeTemp();
    }	
    areg = GetTemp();
    (*find(symbolTable.begin(),symbolTable.end(),areg)).dataType = BOOLEAN;
    pushOperand(areg);
}

void EmitGreaterThanCode(string operand1, string operand2)
{
	string intOp1, intOp2, intAreg; //internal names for operand 1 and 2 and areg
    intAreg = getIntName(areg);
    intOp1 = getIntName(operand1);
    intOp2 = getIntName(operand2);
    
    //if types of operands are not the same
	if(WhichType(operand1) != WhichType(operand2))
	{
		printError("incompatible types");
	}
    //if A Register holds a temp not operand1 nor operand2 then 
    if(intAreg[0] == 'T' && (intAreg != intOp1 && intAreg != intOp2))
     {
        //emit code to store that temp into memory
    	objectFile << rammTab << "STA " << setw(4) << left << areg << right << rTbFv << "deassign AReg" <<endl;
        //change the allocate entry for it in the symbol table to yes 
    	(*find(symbolTable.begin(),symbolTable.end(),areg)).alloc = YES;
    	areg = "";
    	intAreg = "";
     }
    // if A register holds a non-temp not operand2 nor operand1 then deassign it
	if(intAreg[0] != 'T' && intAreg != intOp1 && intAreg != intOp2)
	{
		areg = "";
		intAreg = "";
	}
	if(intAreg != intOp2)
	{
		 //emit code to load operand2 into the A register;
		objectFile << rammTab << "LDA " << setw(4) << left <<intOp2 << right << rTbFv  << endl;
	}
	objectFile << rammTab << "ISB " << setw(4) << left <<intOp1 << right << rTbFv << operand2 << " > " << operand1 << endl;
	objectFile << rammTab << "AMJ L" << setw(3) << left <<loopCount <<right << rTbFv << endl;
	objectFile << rammTab << "AZJ L" << setw(3) << left <<loopCount <<right << rTbFv << endl;
	//if the result isn't negative, then its false	
    if(!TRU)
    {
        Insert("TRUE", BOOLEAN, CONSTANT, "1", YES, 1, "TRUE");
		TRU = true;
    }
	objectFile << rammTab << "LDA TRUE" << rTbFv << endl;
	if(!FALS)
    {
        Insert("FALSE", BOOLEAN, CONSTANT, "0", YES, 1, "FALS");
		FALS = true;
    }
	objectFile << rammTab << "UNJ " << "L" << setw(3) << left <<  loopCount <<  "+1   " << endl;
	//if the result /IS/ negative then its true
	objectFile << "L" << setw(3) << left << loopCount << "  LDA " << "FALS" << rTbFv << endl;

	loopCount++;
	//deassign all temporaries involved and free those names for reuse;
     if(operand1[0] == 'T')
    {
        FreeTemp();
    }
    if(operand2[0] == 'T')
    {
        FreeTemp();
    }	
    areg = GetTemp();
    (*find(symbolTable.begin(),symbolTable.end(),areg)).dataType = BOOLEAN;
    pushOperand(areg);
}

void EmitAssignCode(string operand1,string operand2) //assign the value of operand1 to operand2
{
	string intOp1, intOp2, intAreg; //internal names for operand 1 and 2 and areg
    intAreg = getIntName(areg);
    intOp1 = getIntName(operand1);
    intOp2 = getIntName(operand2);
	//if types of operands are not the same
	if (WhichType(operand1) != WhichType(operand2)) 
	{
		printError("incompatible types"); //process error: incompatible types
	}
	 //if storage mode of operand2 is not VARIABLE
	if((*find(symbolTable.begin(),symbolTable.end(),operand2)).mode != VARIABLE)
	{
		//process error: symbol on left-hand side of assignment must have a storage mode of VARIABLE
		printError("symbol on left-hand side of assignment must have a storage mode of VARIABLE");
	}
	//if operand1 = operand2 return;
	//symbol table doesn't update values so if the values this won't work
	//if((*find(symbolTable.begin(),symbolTable.end(),operand2)).value == (*find(symbolTable.begin(),symbolTable.end(),operand1)).value)
	if(operand1 == operand2)
	{
		return;
	}	

	//if operand1 is not in A register then
	if (areg != intOp1)
	{
		//emit code to load operand1 into the A register;
		objectFile << rammTab << "LDA " << setw(4) << left <<intOp1 << right << rTbFv  << endl;
	}
	//emit code to store the contents of that register into the memory location pointed to by operand2

	objectFile << rammTab << "STA " << setw(4) << left << intOp2 << right << rTbFv << operand2 << " := " << operand1 << endl;
	//deassign operand1;
	//if operand1 is a temp then free its name for reuse;
	if(intOp1[0] == 'T')
	{
	    FreeTemp();
	    areg = "";
	}
    areg = operand2;
	objectFile << "areg = " << operand2 << " from EmitAssign" << endl;
	//free temporary
	//operand2 can never be a temporary since it is to the left of ':='
}

void FreeTemp()
{
    tempCount--;
    if (tempCount < -1)
    {
        printError("compiler error, currentTempNo should be >= –1");
    }
}
string GetTemp()
{
    ostringstream temp;
    tempCount++;
    temp << "T";
    temp << tempCount;
    if (tempCount > maxTempCount)
    {
        //Insert a temp that does not take up memory and has default type of INTEGER, this should be changed by the calling function
        Insert(temp.str(), INTEGER, VARIABLE, "", NO, 1);
        maxTempCount++;
    }
    return temp.str();
}
void EmitStartCode()
{
	entry ent = symbolTable[0];
    //If the first entry in the symbol table isn't the program name, then look through the entire table for it
    if(ent.dataType != PROG_NAME)
    {
        for(unsigned int i=0; i<symbolTable.size(); ++i)
        {
            if(symbolTable[i].dataType == PROG_NAME)
            {
                ent = symbolTable[i];
                break;
            }
        }
    }
	objectFile << "STRT  NOP" << rTbFv << rTbFv << ent.externalName << " - Anna Porter, Jacob Hallenberger" << endl;
}


void EmitThenCode(string operand) //emit code that follows 'then' and statement predicate
{
    ostringstream tempLabel;
	tempLabel << "L" << loopCount;
	loopCount++;
	if(WhichType(operand) != BOOLEAN)
    {
        printError("Condition must be a boolean");
    }
	//emit instruction to set the condition code depending on the value of operand;
	string intOp = getIntName(operand);
	if(areg != operand)
	{
		objectFile << rammTab << "LDA " << intOp << rTbFv << endl;
	}
	//emit instruction to branch to tempLabel if the condition code indicates operand is zero (false)
	objectFile << rammTab << "AZJ " << setw(4) << left << tempLabel.str() << right << rTbFv << "if false jump to " << tempLabel.str() << endl;
	//push tempLabel onto operandStk so that it can be referenced when EmitElseCode() or EmitPostIfCode() is called;
	pushOperand(tempLabel.str());
	//if operand is a temp then
	if(operand[0] == 'T')
	{
		//free operand's name for reuse;
		FreeTemp();
	}	
	//deassign operands from all registers
	areg = "";
}
void EmitElseCode(string operand) //emit code that follows else clause of if statement
{
    ostringstream tempLabel;
	tempLabel << "L" << loopCount;
	loopCount++;
	//emit instruction to branch unconditionally to tempLabel;
	objectFile << rammTab << "UNJ " << setw(4) << left << tempLabel.str() << right << rTbFv << "jump to end if" << endl;
	//emit instruction to label this point of object code with the argument operand;
	objectFile << setw(6) << left << operand << setw(6) << left << "NOP" << rTbFv << "  else" << endl;
	//push tempLabel onto operandStk;
	pushOperand(tempLabel.str());
	//deassign operands from all registers
	areg = "";
}
void EmitPostIfCode(string operand) //emit code that follows end of if statement
{
	//emit instruction to label this point of object code with the argument operand;
	objectFile << setw(6) << left << operand << setw(6) << left << "NOP" << rTbFv << "  end if" << endl;
	//deassign operands from all registers
	areg = "";
}
void EmitWhileCode() //emit code that follows while
{
    ostringstream tempLabel;
	tempLabel << "L" << loopCount;
	//assign next label to tempLabel;
	//tempLabel += (""+loopCount);
	loopCount++;
	//emit instruction to label this point of object code as tempLabel;
	objectFile << setw(6) << left << tempLabel.str() << setw(6) << left << "NOP" << rTbFv << "  while" << endl;
	//push tempLabel onto operandStk;
 	pushOperand(tempLabel.str());
	//deassign operands from all registers
	areg = "";
}
void EmitDoCode(string operand) //emit code that follows do
{
    ostringstream tempLabel;
	tempLabel << "L" << loopCount;
	loopCount++;
    if(WhichType(operand) != BOOLEAN)
    {
        printError("Condition must be a boolean");
    }
	string intOp = getIntName(operand);
	//emit instruction to set the condition code depending on the value of operand;
	if(areg != operand)
	{
		objectFile << rammTab << "LDA " << setw(4) << intOp << rTbFv << endl;
	}
	//emit instruction to branch to tempLabel if the condition code indicates operand is zero (false)
	objectFile << rammTab << "AZJ " << setw(4) <<tempLabel.str() << rTbFv << "do" << endl;
	//push tempLabel onto operandStk;
 	pushOperand(tempLabel.str());
	//if operand is a temp then
	if(operand[0] == 'T')
	{
		//free operand's name for reuse;
		FreeTemp();
	}
	//deassign operands from all registers
	areg = "";
}
void EmitPostWhileCode(string operand1,string operand2)
 //emit code at end of while loop, operand2 is the label of the beginning of the loop,
 //operand1 is the label which should follow the end of the loop
{
	//emit instruction which branches unconditionally to the beginning of the loop, i.e., to the value of operand2
	objectFile << rammTab << "UNJ " << setw(4) << left << operand2 << right << rTbFv << "end while" << endl;
	//emit instruction which labels this point of the object code with the argument operand1;
	objectFile << setw(6) << left << operand1 << setw(6) << left << "NOP          " << endl;
	//deassign operands from all registers
	areg = "";
}
void EmitRepeatCode() //emit code that follows repeat
{
    ostringstream tempLabel;
	tempLabel << "L" << loopCount;
	loopCount++;
	//emit instruction to label this point in the object code with the value of tempLabel;
	objectFile << setw(6) << left << tempLabel.str() << setw(6) << left << "NOP" << rTbFv << "  repeat" << endl;
	//push tempLabel onto operandStk;
 	pushOperand(tempLabel.str());
	//deassign operands from all registers
	areg = "";
}
void EmitUntilCode(string operand1, string operand2)
 //emit code that follows until and the predicate of loop. operand1 is the value of the
 //predicate. operand2 is the label that points to the beginning of the loop
{
    if(WhichType(operand1) != BOOLEAN)
    {
        printError("Condition must be a boolean");
    }
	//emit instruction to set the condition code depending on the value of operand1;
	string intOp = getIntName(operand1);
	if(areg != operand1)
	{
		objectFile << rammTab << "LDA " << intOp << rTbFv << endl;
	}
	//emit instruction to branch to the value of operand2 if the condition code indicates operand1 is zero (false)
	objectFile << rammTab << "AZJ " << operand2 << rTbFv << "  until" << endl;
	//if operand1 is a temp then
	if(operand1[0] == 'T')
	{
		//free operand1's name for reuse;
		FreeTemp();
	}
	//deassign operands from all registers
	areg = "";
}
