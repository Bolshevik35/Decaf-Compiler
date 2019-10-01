#include "default-defs.h"
#include <list>
#include <ostream>
#include <iostream>
#include <sstream>
#include <string>

#ifndef YYTOKENTYPE
#include "decafast.tab.h"
#endif

using namespace std;

/// decafAST - Base class for all abstract syntax tree nodes.
class decafAST {
public:
  virtual ~decafAST() {}
  virtual string str() { return string(""); }
};

string getString(decafAST *d) {
        if (d != NULL) {
                return d->str();
        } else {
                return string("None");
        }
}


string getDecafType(string *a){
        if(*a == "IntType") { return string("IntType");}
        if(*a == "BoolType") { return string("BoolType");}
        if(*a == "VoidType") { return string("VoidType");}
        if(*a == "StringType") { return string("StringType");}
}

template <class T>
string commaList(list<T> vec) {
    string s("");
    for (typename list<T>::iterator i = vec.begin(); i != vec.end(); i++) {
        s = s + (s.empty() ? string("") : string(",")) + (*i)->str();
    }
    if (s.empty()) {
        s = string("None");
    }
    return s;
}

/// decafStmtList - List of Decaf statements
class decafStmtList : public decafAST {
        list<decafAST *> stmts;
public:
        decafStmtList() {}
        ~decafStmtList() {
                for (list<decafAST *>::iterator i = stmts.begin(); i != stmts.end(); i++) {
                        delete *i;
                }
        }
        int size() { return stmts.size(); }
        void push_front(decafAST *e) { stmts.push_front(e); }
        void push_back(decafAST *e) { stmts.push_back(e); }
        string str() { return commaList<class decafAST *>(stmts); }
};

class StringConstAST : public decafAST {
        string Name;
public:
		StringConstAST() {}
        StringConstAST(string name ) : Name(name){}
        ~StringConstAST () {}
        string str() { return string("StringConstant") + "("  +  Name + ")"; }
};

class StringAST : public decafAST {
        string Name;
public:
        StringAST() {}
        StringAST(string name ) : Name(name){}
        ~StringAST () {}
        string str() { return Name ; }
};

class Type_IdAST : public decafAST {
        string *Type;
        decafAST *NameList; 
public:
		Type_IdAST(string* type) : Type(type) {}
        Type_IdAST(decafAST *namelist, string *type) : NameList(namelist), Type(type) {}
        ~Type_IdAST() { 
        	delete Type, NameList;
        }
        string str() { 
            if(NameList != NULL){ 
                 int size = NameList->str().length(); 
                 std::list<string> v;
                 string temp; 
                 for (int i =0; i < size; i++){
                    if (NameList->str()[i] != ',')
                        temp.push_back(NameList->str()[i]);
                    else {
                        string a =  string("VarDef") + "(" + temp + "," + getDecafType(Type) + ")";  
                        temp = "";
                        v.push_back(a);
                    }
                 }
                 string a =  string("VarDef") + "(" + temp + "," + getDecafType(Type) + ")";  
                 temp = "";
                 v.push_back(a); 
                 string s("");
                 for (list<string>::iterator i = v.begin(); i != v.end(); i++) { 
                    s = s + (s.empty() ? string("") : string(",")) + (*i);
                 }
                return s;               
            }
        	else
        		return string("VarDef") + "(" + getDecafType(Type) + ")" ;   
    	}
};

// class VariableExprAST : public decafAST { 
//     string Name; 
//     string *Type; 
// public: 
//     VariableExprAST(string* type) : Type(type) {}
//     VariableExprAST(string name, string *type) : Name(name), Type(type) {}
//     ~VariableExprAST() {}
//     string str() {
//         if (Name != ""){
//             return string("VarDef") + "(" + Name + "," + getDecafType(Type) + ")" ;
//         }
//         else return string("VarDef") + "(" + getDecafType(Type) + ")"; 
//     }
//     //llvm::Value *Codegen(){ return NULL; }
// };

class NumberExprAST : public decafAST {
    int Number; 
public: 
    NumberExprAST(int number) : Number(number) {}
    ~NumberExprAST() {}
    string str() { 
        string ret = to_string(Number);
        return string("NumberExpr") + "(" + ret  + ")" ;
    }
}; 

class BoolExprAST : public decafAST { 
    string Boolea;
public: 
    BoolExprAST(string boolea) : Boolea(boolea) {}
    ~BoolExprAST() {}
    string str() {return string("BoolExpr") + "(" + Boolea + ")" ;}
};

class BinaryExprAST : public decafAST { 
    string Op ;
    decafAST *Lhs; 
    decafAST *Rhs; 
public: 
    BinaryExprAST(string op, decafAST *lhs, decafAST *rhs): Op(op), Lhs(lhs), Rhs(rhs) {}
    ~BinaryExprAST() { delete Lhs, Rhs;}
    string str() { 
        return string("BinaryExpr") + "(" + Op + "," + getString(Lhs) + "," +getString(Rhs) + ")" ; }
};

class UnaryExprAST : public decafAST { 
    string Op; 
    decafAST *Expr; 
public: 
    UnaryExprAST(string op, decafAST *expr) : Op(op), Expr(expr) {}
    ~UnaryExprAST() { delete Expr; }
    string str() { 
        return string("UnaryExpr") + "(" + Op + "," + getString(Expr) + ")" ; }
};

class AssignGlobalVarAST : public decafAST {  
	string *Type;
	decafAST *Value; 
	decafAST *NameList;
public:
	AssignGlobalVarAST(decafAST *namelist, string *type, decafAST *value)
	: NameList(namelist), Type(type), Value(value) {}
	~AssignGlobalVarAST() {  delete Type, Value, NameList; }
	string str() { 
		int size = NameList->str().length(); 
		std::list<string> v;
		string temp; 
		for (int i =0; i < size; i++){
		    if (NameList->str()[i] != ',')
		        temp.push_back(NameList->str()[i]);
		    else {
		        exit (EXIT_FAILURE); 
		    }
		}
		string a =  string("AssignGlobalVar") + "(" + temp + "," + getDecafType(Type) + "," + getString(Value) +  ")";  
		temp = "";
		v.push_back(a); 
		string s("");
		for (list<string>::iterator i = v.begin(); i != v.end(); i++) { 
			s = s + (s.empty() ? string("") : string(",")) + (*i);
		}
		return s;    
	}
};

class AssignVarAST : public decafAST { 
	string Name;
	decafAST *Value; 
public: 
	AssignVarAST(string name, decafAST *value) 
	: Name(name) , Value(value) {}
	~AssignVarAST() { 
		if(Value != NULL) {delete Value;} 
	}
	string str() {return string("AssignVar") + "(" + Name + "," + getString(Value) + ")" ;}  
};

class VariableExprAST : public decafAST {
    string Name; 
public: 
    VariableExprAST(string name)
    : Name(name) {}
    ~VariableExprAST() {}
    string str() { return string("VariableExpr") + "(" + Name + ")" ;}
};

class AssignArrayLocAST : public decafAST { 
	string Name; 
	decafAST *Index;
	decafAST *Value; 
public:		
	AssignArrayLocAST(string name, decafAST *index, decafAST *value)
	: Name(name), Index(index), Value(value) {}
	~AssignArrayLocAST() {
		if(Index != NULL) {delete Index;}
		if(Value != NULL) {delete Value;}
	}
	string str() {return string("AssignArrayLoc") + "(" + Name + "," + getString(Index) + "," + getString(Value) + ")" ;}
};

class ArrayLocExprAST : public decafAST { 
    string Name; 
    decafAST *Index; 
public: 
    ArrayLocExprAST(string name, decafAST *index)
    : Name(name), Index(index) {}
    ~ArrayLocExprAST() {
        if(Index != NULL) {delete Index;}
    }
    string str() {return string("ArrayLocExpr") + "(" + Name + "," + getString(Index)  + ")" ;}    
};

class PackageAST : public decafAST {
        string Name;
        decafStmtList *FieldDeclList;
        decafStmtList *MethodDeclList;
public:
        PackageAST(string name, decafStmtList *fieldlist, decafStmtList *methodlist)
                : Name(name), FieldDeclList(fieldlist), MethodDeclList(methodlist) {}
        ~PackageAST() {
                if (FieldDeclList != NULL) { delete FieldDeclList; }
                if (MethodDeclList != NULL) { delete MethodDeclList; }
        }
        string str() {
                return string("Package") + "(" + Name + "," + getString(FieldDeclList) + "," + getString(MethodDeclList) + ")";
        }
};

/// ProgramAST - the decaf program
class ProgramAST : public decafAST {
        decafStmtList *ExternList;
        PackageAST *PackageDef;
public:
        ProgramAST(decafStmtList *externs, PackageAST *c) : ExternList(externs), PackageDef(c) {}
        ~ProgramAST() {
                if (ExternList != NULL) { delete ExternList; }
                if (PackageDef != NULL) { delete PackageDef; }
        }
        string str() { return string("Program") + "(" + getString(ExternList) + "," + getString(PackageDef) + ")"; }
};


class ExternAST : public decafAST {
        string Name;
        string *ReturnType;
        decafStmtList *ExternList;
public:
        ExternAST(string name, string *returntype, decafStmtList *externlist)
        : Name(name), ReturnType(returntype), ExternList(externlist) {}
        ~ExternAST() {}
        string str(){ return string("ExternFunction") + "(" + Name + "," + getDecafType(ReturnType) + "," + getString(ExternList) + ")" ;}
};

class FieldSize : public decafAST {  
	int ArraySize = -1; 
public: 
    FieldSize() {}
	FieldSize(int arraysize) : ArraySize(arraysize) {}
	string str() { if (ArraySize == -1) return string("Scalar"); 
				   else { string ret = to_string(ArraySize);
				   		return string("Array") + "(" + ret + ")" ;}
				 }

};

class FieldAST : public decafAST { 
	decafAST *NameList ;
	string *ReturnType; 
	FieldSize *Fieldsize; 
public: 
	FieldAST() { }
	FieldAST(decafAST *namelist, string *returntype, FieldSize *fieldsize)
	: NameList(namelist), ReturnType(returntype), Fieldsize(fieldsize) {}
	~FieldAST() { delete NameList, ReturnType, Fieldsize; }
	string str() { 
		if(NameList != NULL){
			int size = NameList->str().length(); 
			std::list<string> v; 
			string temp;
			for(int i=0; i < size ; i++){
				if (NameList->str()[i] != ','){
					temp.push_back(NameList->str()[i]);
				}
				else{ 
					string a = string("FieldDecl") + "(" + temp + "," + getDecafType(ReturnType) + "," + getString(Fieldsize) + ")" ; 
					temp = "";
					v.push_back(a);
				}
			}
			string a = string("FieldDecl") + "(" + temp + "," + getDecafType(ReturnType) + "," + getString(Fieldsize) + ")" ; 
			temp = "";
			v.push_back(a);
			string s("");
			for(list<string>::iterator i = v.begin(); i != v.end(); i++){
				s = s + (s.empty() ? string("") : string(",")) + (*i);
			}
			return s; 
		}
	}
} ;

class MethodBlockAST : public decafAST {
        decafStmtList *Var_Decl;
        decafStmtList *State_List;
public:
        MethodBlockAST(decafStmtList *var_decl, decafStmtList *state_list)
        :Var_Decl(var_decl), State_List(state_list) {}
        ~MethodBlockAST(){
                if(Var_Decl != NULL) {delete Var_Decl;}
                if(State_List != NULL) {delete State_List;}
        }
        string str(){
                return string("MethodBlock") + "(" + getString(Var_Decl) + "," + getString(State_List) + ")" ;
        }
};

class BlockAST : public decafAST { 
		decafStmtList *Var_Decl; 
		decafStmtList *State_List;
public:
        BlockAST() {}
		BlockAST(decafStmtList *var_decl, decafStmtList *state_list)
		:Var_Decl(var_decl) , State_List(state_list){}
		~BlockAST(){
                if(Var_Decl != NULL) {delete Var_Decl;}
                if(State_List != NULL) {delete State_List;}
        }
        string str(){
        	if((Var_Decl == NULL) && (State_List == NULL))
        		return string("None");
            else    
            	return string("Block") + "(" + getString(Var_Decl) + "," + getString(State_List) + ")" ;
        }
};

class MethodAST :public decafAST {
        string Name;
        string *DecafType;
        decafStmtList *Method_Param;
        MethodBlockAST *Block;
public:
        MethodAST (string name, string *decaftype, decafStmtList *method_params, MethodBlockAST *block)
        : Name(name), DecafType(decaftype), Method_Param(method_params), Block(block) {}
        ~MethodAST() {
                if(Method_Param != NULL) { delete Method_Param;}
                if(Block != NULL) { delete Block;}
        }
        string str(){
                return string("Method") + "(" + Name + "," + getDecafType(DecafType) + "," + getString(Method_Param) + "," + getString(Block) + ")";
        }
};

class MethodCallAST : public decafAST { 
	string Name; 
	decafStmtList *Args;  
public: 
	MethodCallAST(string name, decafStmtList *args) 
	: Name(name), Args(args) {}
	~MethodCallAST() { 
		if(Args != NULL) {delete Args; } 
	}
	string str() {return string("MethodCall") + "(" + Name + "," + getString(Args) + ")" ;}
};

class IfStmtAST : public decafAST { 
    decafAST *Condition; 
    BlockAST *IfBlock; 
    BlockAST *ElseBlock; 
public: 
    IfStmtAST(decafAST *condition, BlockAST *ifblock, BlockAST *elseblock) : Condition(condition), IfBlock(ifblock), ElseBlock(elseblock) {} 
    ~IfStmtAST() { delete Condition, IfBlock, ElseBlock; }
    string str() { return string("IfStmt") + "(" + getString(Condition) + "," + getString(IfBlock) + "," + getString(ElseBlock) + ")" ;}
};

class WhileStmtAST : public decafAST { 
    decafAST *Condition; 
    BlockAST *WhileBlock; 
public: 
    WhileStmtAST(decafAST *condition, BlockAST *whileblock) : Condition(condition), WhileBlock(whileblock) {}
    ~WhileStmtAST() { delete Condition, WhileBlock; }
    string str() { return string("WhileStmt") + "(" + getString(Condition) + "," + getString(WhileBlock) + ")" ;}
};

class ForStmtAST : public decafAST { 
    decafStmtList *InitList;
    decafAST *Condition; 
    decafStmtList *LoopCond;
    BlockAST *ForBlock;  
public: 
    ForStmtAST(decafStmtList *initList, decafAST *condition, decafStmtList *loopcond, BlockAST *forBlock) 
    : InitList(initList), Condition(condition), LoopCond(loopcond), ForBlock(forBlock) {}
    ~ForStmtAST() { delete InitList, Condition, LoopCond, ForBlock; }
    string str() { return string("ForStmt") + "(" + getString(InitList) + "," + getString(Condition) + "," + getString(LoopCond) + "," + getString(ForBlock) + ")" ;}
};

class ReturnStmtAST : public decafAST { 
	decafAST *Ret; 
public: 
	ReturnStmtAST() {}
	ReturnStmtAST(decafAST *ret) : Ret(ret) {}
	~ReturnStmtAST() { delete Ret;}
	string str() { return string("ReturnStmt") + "(" + getString(Ret) + ")" ;}
};

class BreakStmtAST : public decafAST { 
public:
	BreakStmtAST() {}
	~BreakStmtAST() {}
	string str() { return string("BreakStmt") ;}
};

class ContinueStmtAST : public decafAST {
public: 
	ContinueStmtAST() {}
	~ContinueStmtAST() {}
	string str() { return string("ContinueStmt") ;}
};

// >>>>>>>>>> DONE <<<<<<<<<
