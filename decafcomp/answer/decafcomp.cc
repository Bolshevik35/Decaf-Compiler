
#include "default-defs.h"
#include <list>
#include <ostream>
#include <iostream>
#include <sstream>
#include <map>

#ifndef YYTOKENTYPE
#include "decafcomp.tab.h"
#endif

using namespace std;

typedef llvm::Value descriptor;
typedef map<string, descriptor* > symbol_table;
typedef list<symbol_table *> symbol_table_list;
symbol_table_list symtbl;


void enter_symtbl(string ident,llvm::Value* val) {
        symbol_table *syms = new symbol_table();
        symtbl.push_front(syms);
        (*syms)[ident] = val;
}

descriptor* access_symtbl(string ident) {
    for (auto i : symtbl) {
        auto find_ident = i->find(ident);
        if (find_ident != i->end()) {
            return find_ident->second;
        }
        //cout << find_ident->first << ", " << find_ident->second << endl; 
    }
    return NULL;
}

//llvm::AllocaInst *Alloca;
llvm::AllocaInst *defineVariable(llvm::Type *llvmTy, string ident) {
        llvm::AllocaInst *Alloca;
        Alloca = Builder.CreateAlloca(llvmTy, 0, ident.c_str());
        enter_symtbl(ident, (llvm::Value *)Alloca);
        return Alloca;
}
string getDecafType(string *a){
    if((*a).compare(string("IntType")) == 0) { return string("IntType");}
    if((*a).compare(string("BoolType")) == 0) { return string("BoolType");}
    if((*a).compare(string("VoidType")) == 0) { return string("VoidType");}
    if((*a).compare(string("StringType")) == false) { return string("StringType");}
    else {throw runtime_error("unknown type");}
}

llvm::Type *getLLVMType(string ty) {
        if(ty.compare(string("VoidType")) == 0) {return Builder.getVoidTy();}
        if(ty.compare(string("IntType")) == 0) {return Builder.getInt32Ty();}
        if(ty.compare(string("BoolType")) == 0) {return Builder.getInt1Ty();}
        if(ty.compare(string("StringType")) == 0) {return Builder.getInt8PtrTy();}
        else {throw runtime_error("unknown type");}
}

/// decafAST - Base class for all abstract syntax tree nodes.
class decafAST {
public:
  virtual ~decafAST() {}
  virtual string str() { return string(""); }
  virtual llvm::Value *Codegen() = 0;
};

string getString(decafAST *d) {
        if (d != NULL) {
                return d->str();
        } else {
                return string("None");
        }
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

template <class T>
llvm::Value *listCodegen(list<T> vec) {
        llvm::Value *val = NULL;
        for (typename list<T>::iterator i = vec.begin(); i != vec.end(); i++) {
                llvm::Value *j = (*i)->Codegen();
                if (j != NULL) { val = j; }
        }
        return val;
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
        list<decafAST *>::iterator begin() { return stmts.begin();}
        list<decafAST *>::iterator end() { return stmts.end();}
        string str() { return commaList<class decafAST *>(stmts); }
        llvm::Value *Codegen() {
                return listCodegen<decafAST *>(stmts);
        }
};

class StringConstAST : public decafAST {
        string Name;
public:
                StringConstAST() {}
        StringConstAST(string name ) : Name(name){}
        ~StringConstAST () {}
        string str() { return string("StringConstant") + "("  +  Name + ")"; }
        llvm::Value *Codegen() {
                string newStr = Name;
                int size = newStr.length();
                int i =0;
                while (newStr[i] != '\0'){
                        if (newStr[i] == '\\'){
                                if(newStr[i+1] == 'n'){char a = '\n'; string ret; ret.push_back(a); newStr.replace(i,2,ret);i++;}
                                else if(newStr[i+1] == 'a'){char a = '\a'; string ret; ret.push_back(a); newStr.replace(i,2,ret);i++;}
                                else if(newStr[i+1] == 'b'){char a = '\b'; string ret; ret.push_back(a); newStr.replace(i,2,ret);i++;}
                                else if(newStr[i+1] == 't'){char a = '\t'; string ret; ret.push_back(a); newStr.replace(i,2,ret);i++;}
                                else if(newStr[i+1] == 'r'){char a = '\r'; string ret; ret.push_back(a); newStr.replace(i,2,ret);i++;}
                                else if(newStr[i+1] == 'v'){char a = '\v'; string ret; ret.push_back(a); newStr.replace(i,2,ret);i++;}
                                else if(newStr[i+1] == 'f'){char a = '\f'; string ret; ret.push_back(a); newStr.replace(i,2,ret);i++;}
                                else if(newStr[i+1] == '\\'){char a = '\\'; string ret; ret.push_back(a); newStr.replace(i,2,ret);i++;}
                                else if(newStr[i+1] == '"'){char a = '\"'; string ret; ret.push_back(a); newStr.replace(i,2,ret); i++;}
                                else i++;
                        }
                        else if(newStr[i] == '"')
                                newStr.erase(newStr.begin()+i);
                        else {
                                i++;
                        }
                }
                const char *s = newStr.c_str();
                llvm::GlobalVariable *GS = Builder.CreateGlobalString(s, "globalstring");
                llvm::Value  *val = Builder.CreateConstGEP2_32(GS->getValueType(), GS, 0, 0  );
                return val;
        }
};

class StringAST : public decafAST {
        string Name;
public:
        StringAST() {}
        StringAST(string name ) : Name(name){}
        ~StringAST () {}
        string str() { return Name ; }
        llvm::Value *Codegen() {return NULL; }
};

class Type_IdAST : public decafAST {
        string *Type;
        decafAST *NameList;
public:
        Type_IdAST(string* type) : Type(type) {}
        Type_IdAST(decafAST *namelist, string *type) : NameList(namelist), Type(type) {}
        string getType(){ return *Type;}
        vector<string> getNameList() { 
        	int size = NameList->str().length();
                std::vector<string> v;
                string temp;
                for (int i =0; i < size; i++){
                    if (NameList->str()[i] != ',')
                        temp.push_back(NameList->str()[i]);
                    else {
                        v.push_back(temp);
                        temp = "";
                    }
                }
            v.push_back(temp);
            return v;  
        }
        ~Type_IdAST() {
                delete Type;
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
                        string a = string("VarDef") + "(" + temp + "," + getDecafType(Type) + ")"; ;
                        temp = "";
                        v.push_back(a);
                    }
                }
                string a = string("VarDef") + "(" + temp + "," + getDecafType(Type) + ")"; ;
                temp = "";
                v.push_back(a);
                string s("");
                for (list<string>::iterator i = v.begin(); i != v.end(); i++) {
                    s = s + (s.empty() ? string("") : string(",")) + (*i);
                }
                return s;
            }
                else
                    return  string("VarDef") + "(" + getDecafType(Type) + ")";
        }
        llvm::Value *Codegen() {
            if(NameList != NULL){
                int size = NameList->str().length();
                std::list<string> v;
                string temp;
                for (int i =0; i < size; i++){
                    if (NameList->str()[i] != ',')
                        temp.push_back(NameList->str()[i]);
                    else {
                        string a = temp ;
                        temp = "";
                        v.push_back(a);
                    }
                }
                string a =temp;
                temp = "";
                v.push_back(a);
                //llvm::Value *rval ;
                for (list<string>::iterator i = v.begin(); i != v.end(); i++){
                    llvm::AllocaInst *Alloca = defineVariable(getLLVMType(*Type), (*i));
                }
                    return (llvm::Value*)getLLVMType(*Type);
            }
            else{
                return (llvm::Value*)(getLLVMType(*Type)); 
            }
        }
};

class NumberExprAST : public decafAST {
    int Number;
public:
    NumberExprAST(int number) : Number(number) {}
    ~NumberExprAST() {}
    int RetInt(){
        return Number;
    }
    string str() {
        string ret = to_string(Number);
        return string("NumberExpr") + "(" + ret  + ")" ;
    }
    llvm::Value *Codegen() {
        return Builder.getInt32(Number);
    }
};

class BoolExprAST : public decafAST {
    string Boolea;
public:
    BoolExprAST(string boolea) : Boolea(boolea) {}
    ~BoolExprAST() {}
    string str() {return string("BoolExpr") + "(" + Boolea + ")" ;}
    llvm::Value *Codegen() {
        int ret;
        if (Boolea.compare(string("True")) == 0) {
                ret = 1;
                return Builder.getInt1(ret);
        }
        else if (Boolea.compare(string("False")) == 0) {
                ret = 0;
                return Builder.getInt1(ret);
        }
        else return NULL;
    }
};

class BinaryExprAST : public decafAST {
    string Op ;
    decafAST *Lhs;
    decafAST *Rhs;
public:
    BinaryExprAST(string op, decafAST *lhs, decafAST *rhs): Op(op), Lhs(lhs), Rhs(rhs) {}
    ~BinaryExprAST() { delete Lhs;}
    string str() {
        if(Op.compare(string("+")) == 0)
            return string("BinaryExpr") + "(" + string("PLus") + "," + getString(Lhs) + "," +getString(Rhs) + ")" ; 
        if(Op.compare(string("-")) == 0)
            return string("BinaryExpr") + "(" + string("Minus") + "," + getString(Lhs) + "," +getString(Rhs) + ")" ; 
        if(Op.compare(string("*")) == 0)
            return string("BinaryExpr") + "(" + string("Mult") + "," + getString(Lhs) + "," +getString(Rhs) + ")" ; 
        if(Op.compare(string("/")) == 0)
            return string("BinaryExpr") + "(" + string("Div") + "," + getString(Lhs) + "," +getString(Rhs) + ")" ; 
        if(Op.compare(string("<<")) == 0)
            return string("BinaryExpr") + "(" + string("Leftshift") + "," + getString(Lhs) + "," +getString(Rhs) + ")" ; 
        if(Op.compare(string(">>")) == 0)
            return string("BinaryExpr") + "(" + string("Rightshift") + "," + getString(Lhs) + "," +getString(Rhs) + ")" ;
        if(Op.compare(string("%")) == 0)
            return string("BinaryExpr") + "(" + string("Mod") + "," + getString(Lhs) + "," +getString(Rhs) + ")" ;
        if(Op.compare(string(">")) == 0)
            return string("BinaryExpr") + "(" + string("Gt") + "," + getString(Lhs) + "," +getString(Rhs) + ")" ;
        if(Op.compare(string("<")) == 0)
            return string("BinaryExpr") + "(" + string("Lt") + "," + getString(Lhs) + "," +getString(Rhs) + ")" ;
        if(Op.compare(string(">=")) == 0)
            return string("BinaryExpr") + "(" + string("Geq") + "," + getString(Lhs) + "," +getString(Rhs) + ")" ;
        if(Op.compare(string("<=")) == 0)
            return string("BinaryExpr") + "(" + string("Leq") + "," + getString(Lhs) + "," +getString(Rhs) + ")" ;
        if(Op.compare(string("==")) == 0)
            return string("BinaryExpr") + "(" + string("Eq") + "," + getString(Lhs) + "," +getString(Rhs) + ")" ;
        if(Op.compare(string("!=")) == 0)
            return string("BinaryExpr") + "(" + string("Neq") + "," + getString(Lhs) + "," +getString(Rhs) + ")" ;
        if(Op.compare(string("&&")) == 0)
            return string("BinaryExpr") + "(" + string("And") + "," + getString(Lhs) + "," +getString(Rhs) + ")" ;
        if(Op.compare(string("||")) == 0)
            return string("BinaryExpr") + "(" + string("Or") + "," + getString(Lhs) + "," +getString(Rhs) + ")" ;
        else 
            throw runtime_error("Not a valid Binary Operator");
    }
    llvm::Value *Codegen() {
        llvm::Value *L = Lhs->Codegen();
        llvm::Value *R = Rhs->Codegen();
        if (L == 0 || R == 0) return 0;
  
        if(Op.compare(string("+")) == 0)
            return Builder.CreateAdd(L, R, "addtmp");
        else if (Op.compare(string("-")) == 0)
            return Builder.CreateSub(L, R, "subtmp");
        else if (Op.compare(string("*")) == 0)
            return Builder.CreateMul(L, R, "multmp");
        else if (Op.compare(string("/")) == 0)
            return Builder.CreateSDiv(L, R, "divtmp");
        else if (Op.compare(string("<<")) == 0)
            return Builder.CreateShl(L, R, "shiftleft");
        else if (Op.compare(string(">>")) == 0)
            return Builder.CreateLShr(L, R, "shiftright");
        else if (Op.compare(string("%")) == 0)    
            return Builder.CreateSRem(L, R, "modtmp");
        else if (Op.compare(string("<")) == 0)    
            return Builder.CreateICmpSLT(L, R, "lttmp");
        else if (Op.compare(string(">")) == 0)    
            return Builder.CreateICmpSGT(L, R, "gttmp");
        else if (Op.compare(string("<=")) == 0)    
            return Builder.CreateICmpSLE(L, R, "leqtmp");
        else if (Op.compare(string(">=")) == 0)    
            return Builder.CreateICmpSGE(L, R, "geqtmp");
        else if (Op.compare(string("==")) == 0)    
            return Builder.CreateICmpEQ(L, R, "eqtmp");
        else if (Op.compare(string("!=")) == 0)    
            return Builder.CreateICmpNE(L, R, "neqtmp");
        else if (Op.compare(string("&&")) == 0)
            return Builder.CreateAnd(L, R, Op);
        else if (Op.compare(string("||")) == 0)
            return Builder.CreateOr(L, R, Op);
        else
            throw runtime_error("what operator is this? never heard of it."); 
    }
};

class UnaryExprAST : public decafAST {
    string Op;
    decafAST *Expr;
public:
    UnaryExprAST(string op, decafAST *expr) : Op(op), Expr(expr) {}
    ~UnaryExprAST() { delete Expr; }
    string str() {
        if(Op.compare(string("-")) == 0)
            return string("UnaryExpr") + "(" + string("UnaryMinus") + "," + getString(Expr) + ")" ; 
        else if (Op.compare(string("!")) == 0)
            return string("UnaryExpr") + "(" + string("Not") + "," + getString(Expr) + ")" ;
        else
            throw runtime_error("Not a valid Unary Operator.");
        }
    llvm::Value *Codegen() {
        llvm::Value *exp = Expr->Codegen();
        if(NULL == exp)
            return NULL;
        if(Op.compare(string("-")) == 0)
            return Builder.CreateNeg(exp, "negtmp");
        else if (Op.compare(string("!")) == 0)
            return Builder.CreateNot(exp,"nottmp");
        else 
            throw runtime_error("what operator is this? never heard of it.");
    }
};

class AssignGlobalVarAST : public decafAST {
        string *Type;
        decafAST *Value;
        decafAST *NameList;
public:
        AssignGlobalVarAST(decafAST *namelist, string *type, decafAST *value)
        : NameList(namelist), Type(type), Value(value) {}
        ~AssignGlobalVarAST() {  delete Type; }
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
        llvm::Value *Codegen() {return NULL; }
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
        llvm::Value *Codegen() {
                llvm::Value *rval = NULL;
                if(NULL != Value)
                    rval = Value->Codegen();
                llvm::Value *Alloca = access_symtbl(Name);
                if(Alloca != NULL){
                        const llvm::PointerType *ptrTy = rval->getType()->getPointerTo();
                        if(ptrTy == Alloca->getType()){
                                llvm::Value *val = Builder.CreateStore(rval, Alloca);
                                return val;
                        }
                }
                else {
                        throw runtime_error("unmatch type");
                }
                return NULL;
        }
};

class VariableExprAST : public decafAST {
    string Name;
public:
    VariableExprAST(string name)
    : Name(name) {}
    ~VariableExprAST() {}
    string str() { return string("VariableExpr") + "(" + Name + ")" ;}
    llvm::Value *Codegen() {
        llvm::Value *val = access_symtbl(Name);
        return Builder.CreateLoad(val, Name.c_str()); }
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
        llvm::Value *Codegen() {return NULL; }
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
    llvm::Value *Codegen() {return NULL; }
};

class FieldSize : public decafAST {
        int ArraySize = -1;
public:
    FieldSize() {}
        FieldSize(int arraysize) : ArraySize(arraysize) {}
        string str(){
                if (ArraySize == -1) return string("Scalar");
                else {
                        string ret = to_string(ArraySize);
                        return string("Array") + "(" + ret + ")" ;
                }
        }
        llvm::Value *Codegen() {return NULL; }
};

class FieldAST : public decafAST {
        decafAST *NameList ;
        string *ReturnType;
        FieldSize *Fieldsize;
public:
        FieldAST() { }
        FieldAST(decafAST *namelist, string *returntype, FieldSize *fieldsize)
        : NameList(namelist), ReturnType(returntype), Fieldsize(fieldsize) {}
        ~FieldAST() { delete NameList; }
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
                else return string("");
        }
        llvm::Value *Codegen() {return NULL; }
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
        llvm::Value *Codegen() {
                llvm::Value *val = NULL;
                if(NULL != Var_Decl)
                        val = Var_Decl->Codegen();
                if(NULL != State_List)
                        val = State_List->Codegen();
                else {
                        throw runtime_error("no method block definition in decaf program");
                }
                return val;
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
        llvm::Value *Codegen() {
                llvm::Value *val = NULL;
                if(NULL != Var_Decl)
                        val = Var_Decl->Codegen();
                if(NULL != State_List)
                        val = State_List->Codegen();    
                else{
                        throw runtime_error("empty block definition in decaf program");
                }
                symtbl.pop_front();
                return val;
        }
};


class MethodAST :public decafAST {
        string Name;
        string *DecafType;
        decafStmtList *Method_Param;
        MethodBlockAST *Block;
public:
		string getName() { return Name; }
        MethodAST (string name, string *decaftype, decafStmtList *method_params, MethodBlockAST *block)
        : Name(name), DecafType(decaftype), Method_Param(method_params), Block(block) {}
        ~MethodAST() {
                if(Method_Param != NULL) { delete Method_Param;}
                if(Block != NULL) { delete Block;}
        }
        string str(){
                return string("Method") + "(" + Name + "," + getDecafType(DecafType) + "," + getString(Method_Param) + "," + getString(Block) + ")";
        }

        void declareFunc() { 
            llvm::Type *returnType;
            if((*DecafType).compare(string("IntType")) == 0)
                returnType = llvm::Type::getInt32Ty(TheContext);
            else if((*DecafType).compare(string("BoolType")) == 0)
                returnType = llvm::Type::getInt1Ty(TheContext);
            else if((*DecafType).compare(string("VoidType")) == 0)
                returnType = llvm::Type::getVoidTy(TheContext);
            //else return NULL;
            std::vector<llvm::Type *> args;
            vector<string> liststr;
            llvm::Type *ty; 
            for(list<decafAST *>::iterator i = Method_Param->begin(); i != Method_Param->end(); i++){
                Type_IdAST *ret = (Type_IdAST *)(*i); 
                string liststring = ret->getNameList()[0];
                liststr.push_back(liststring); 
                if((ret->getType()).compare(string("IntType")) == 0){           	 
                	ty = Builder.getInt32Ty();
                	args.push_back(ty);	
                }
                else if((ret->getType()).compare(string("BoolType")) == 0){ 
                	ty = Builder.getInt1Ty();
                	args.push_back(ty);           
                }
            } 
            llvm::Function *TheFunction = llvm::Function::Create(llvm::FunctionType::get(returnType, args, false), llvm::Function::ExternalLinkage, Name, TheModule);
            if (TheFunction == 0) {
                throw runtime_error("empty function block");
            }
            enter_symtbl(Name, (llvm::Value *)TheFunction);



			
        }
        llvm::Value *Codegen() { 
        	llvm::Value *val = NULL;

        	std::vector<llvm::Type *> args;
            vector<string> liststr;
            for(list<decafAST *>::iterator i = Method_Param->begin(); i != Method_Param->end(); i++){
                Type_IdAST *ret = (Type_IdAST *)(*i); 
                string liststring = ret->getNameList()[0];
                liststr.push_back(liststring);
            }
            
            llvm::Function *TheFunction = (llvm::Function *)access_symtbl(Name);

            llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", TheFunction);
            Builder.SetInsertPoint(BB);
            if(NULL != Method_Param){  
                val = Method_Param->Codegen();
                unsigned Idx = 0;
	            for (auto &Arg : TheFunction->args()){
	  				Arg.setName(liststr[Idx]);
	  				llvm::Value *param = access_symtbl(liststr[Idx]); 
	        		llvm::Value *retS = Builder.CreateStore((llvm::Value *)&Arg, param);
	           		Idx++;
				} 
            }
            if(NULL != Block)
                val = Block->Codegen();
            else {
                throw runtime_error("no method definition in decaf program");
            }
            
            //Return void in the end of Method declaration.
            if((*DecafType).compare(string("VoidType")) == 0)
                Builder.CreateRetVoid();
            else
            	Builder.CreateRet(llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0)));  
            return TheFunction;
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
        llvm::Value *Codegen() {
        	std::vector<llvm::Value *> args;
        	llvm::Function *func= TheModule->getFunction(Name);
        	bool is_void = func->getReturnType()->isVoidTy();
        	if(Args != NULL){ 
	            auto iter = func->arg_begin();    
	            for(list<decafAST *>::iterator i = Args->begin(); i != Args->end(); ++i){  
	                if((iter->getType()->isIntegerTy(32)) && ((*i)->Codegen()->getType()->isIntegerTy(1))) {  
	                    llvm::Value *promo = Builder.CreateZExt((llvm::Value *)((*i)->Codegen()), Builder.getInt32Ty(), "zexttmp");                           
	                    args.push_back(promo);
	                }
	                else { 
	                    args.push_back((*i)->Codegen());
	                }
	                iter++;
	            }
            }
            llvm::Value *val = Builder.CreateCall(func, args, is_void? "" :"calltmp");
	        return val;
        }
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

        llvm::Value *Codegen() {
                llvm::Value *val = NULL;
                for(list<decafAST *>::iterator i = MethodDeclList->begin(); i != MethodDeclList->end(); i++){
                	MethodAST *temp = (MethodAST *)(*i);
                	temp->declareFunc();   
                	//enter_symtbl(temp->getName(), (*i)->Codegen());
                }
                TheModule->setModuleIdentifier(llvm::StringRef(Name));
                if (NULL != FieldDeclList) {
                    val = FieldDeclList->Codegen();
                }
                if (NULL != MethodDeclList) { 
                    val = MethodDeclList->Codegen();
                }
                // Q: should we enter the class name into the symbol table?
                return val;
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
        llvm::Value *Codegen() {
                symbol_table *syms = new symbol_table();
                symtbl.push_back(syms);
                llvm::Value *val = NULL;
                if (NULL != ExternList) {
                        val = ExternList->Codegen();
                }
                if (NULL != PackageDef) {
                        val = PackageDef->Codegen();
                } else {
                        throw runtime_error("no package definition in decaf program");
                }
                return val;
        }
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
    llvm::Value *Codegen() {
        llvm::Value *val = NULL;
        llvm::Type *returnTy = getLLVMType(*ReturnType);
        std::vector<llvm::Type *> args;
        for(list<decafAST *>::iterator i = ExternList->begin(); i != ExternList->end(); i++){
                args.push_back((llvm::Type*)(*i)->Codegen());
        }
        llvm::Function *func = llvm::Function::Create(llvm::FunctionType::get(returnTy, args, false), llvm::Function::ExternalLinkage, Name, TheModule);
        enter_symtbl(Name, (llvm::Value *)(func));
        if (func == 0) {
            throw runtime_error("empty function block");
        }
        if(NULL != ExternList)
            val = ExternList->Codegen();
        else {
            throw runtime_error("no extern definition in decaf program");
        }
            return func;
    }
};

class IfStmtAST : public decafAST {
    decafAST *Condition;
    BlockAST *IfBlock;
    BlockAST *ElseBlock;
public:
    IfStmtAST(decafAST *condition, BlockAST *ifblock, BlockAST *elseblock) : Condition(condition), IfBlock(ifblock), ElseBlock(elseblock) {}
    ~IfStmtAST() { delete Condition; }
    string str() { return string("IfStmt") + "(" + getString(Condition) + "," + getString(IfBlock) + "," + getString(ElseBlock) + ")" ;}
    llvm::Value *Codegen() {return NULL; }
};

class WhileStmtAST : public decafAST {
    decafAST *Condition;
    BlockAST *WhileBlock;
public:
    WhileStmtAST(decafAST *condition, BlockAST *whileblock) : Condition(condition), WhileBlock(whileblock) {}
    ~WhileStmtAST() { delete Condition; }
    string str() { return string("WhileStmt") + "(" + getString(Condition) + "," + getString(WhileBlock) + ")" ;}
        llvm::Value *Codegen() {return NULL; }
};

class ForStmtAST : public decafAST {
    decafStmtList *InitList;
    decafAST *Condition;
    decafStmtList *LoopCond;
    BlockAST *ForBlock;
public:
    ForStmtAST(decafStmtList *initList, decafAST *condition, decafStmtList *loopcond, BlockAST *forBlock)
    : InitList(initList), Condition(condition), LoopCond(loopcond), ForBlock(forBlock) {}
    ~ForStmtAST() { delete InitList; }
    string str() { return string("ForStmt") + "(" + getString(InitList) + "," + getString(Condition) + "," + getString(LoopCond) + "," + getString(ForBlock) + ")" ;}
    llvm::Value *Codegen() {return NULL; }
};

class ReturnStmtAST : public decafAST {
        decafAST *Ret;
public:
        ReturnStmtAST() {}
        ReturnStmtAST(decafAST *ret) : Ret(ret) {}
        ~ReturnStmtAST() { delete Ret;}
        string str() { return string("ReturnStmt") + "(" + getString(Ret) + ")" ;}
        llvm::Value *Codegen() {
        	if(Ret == NULL){ 
        		return Builder.CreateRetVoid(); 
        	}
            else{
            	llvm::Value *val = Ret->Codegen();
            	if(NULL != val)
                	return Builder.CreateRet(val);
                else 
                	return NULL;	
        	}
        }    
};

class BreakStmtAST : public decafAST {
public:
        BreakStmtAST() {}
        ~BreakStmtAST() {}
        string str() { return string("BreakStmt") ;}
        llvm::Value *Codegen() {return NULL; }
};

class ContinueStmtAST : public decafAST {
public:
        ContinueStmtAST() {}
        ~ContinueStmtAST() {}
        string str() { return string("ContinueStmt") ;}
        llvm::Value *Codegen() {return NULL; }
};


