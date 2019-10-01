%{
#include <iostream>
#include <ostream>
#include <string>
#include <cstdlib>
#include "default-defs.h"

int yylex(void);
int yyerror(char *); 

// print AST?
bool printAST = false;

using namespace std;

// this global variable contains all the generated code
static llvm::Module *TheModule;

// this is the method used to construct the LLVM intermediate code (IR)
static llvm::LLVMContext TheContext;
static llvm::IRBuilder<> Builder(TheContext);
// the calls to TheContext in the init above and in the
// following code ensures that we are incrementally generating
// instructions in the right order

// dummy main function
// WARNING: this is not how you should implement code generation
// for the main function!
// You should write the codegen for the main method as 
// part of the codegen for method declarations (MethodDecl)
static llvm::Function *TheFunction = 0;

// we have to create a main function 
llvm::Function *gen_main_def() {
  // create the top-level definition for main
  llvm::FunctionType *FT = llvm::FunctionType::get(llvm::IntegerType::get(TheContext, 32), false);
  llvm::Function *TheFunction = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", TheModule);
  if (TheFunction == 0) {
    throw runtime_error("empty function block"); 
  }
  // Create a new basic block which contains a sequence of LLVM instructions
  llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", TheFunction);
  // All subsequent calls to IRBuilder will place instructions in this location
  Builder.SetInsertPoint(BB);
  return TheFunction;
}

#include "decafexpr.cc"

%}

%define parse.error verbose
%union{
    class decafAST *ast;
    std::string *sval;
    int number;
 }

%token T_PACKAGE T_INTTYPE T_VOID T_FUNC T_EXTERN T_STRINGTYPE T_IF T_ELSE T_WHILE T_FOR T_RETURN T_BREAK T_CONTINUE
%token T_BOOLTYPE  T_COMMA T_SEMICOLON T_ASSIGN T_VAR T_PLUS T_MINUS T_MULT T_DIV T_LEFTSHIFT T_RIGHTSHIFT T_MOD T_GT T_LT T_GEQ T_LEQ T_EQ T_NEQ T_AND T_OR T_NOT
%token T_LCB T_RCB T_LPAREN T_RPAREN T_LSB T_RSB  
%token <sval> T_ID T_STRINGCONSTANT T_TRUE T_FALSE
%token <number> T_INTCONSTANT T_CHARCONSTANTONE T_CHARCONSTANTTWO

%type <ast> extern_list decafpackage method_decl_list method_decl method_block typed_symbol_list extern_type_list  extern_func field field_decl field_decl_list var_decl_list var_decl expr constant rvalue assign statement statement_list method_arg_list method_arg method_call bool_expr block assign_list id_list idtype_list
%type <sval> decaf_type method_type extern_type

%left T_OR
%left T_AND
%left T_EQ T_NEQ T_LT T_GEQ T_LEQ T_GT
%left T_PLUS T_MINUS
%left T_MULT T_DIV T_MOD T_LEFTSHIFT T_RIGHTSHIFT
%left T_NOT
%right UMINUS
%%

start: program

program: extern_list decafpackage
    { 
        ProgramAST *prog = new ProgramAST((decafStmtList *)$1, (PackageAST *)$2); 
		if (printAST) {
			cout << getString(prog) << endl;
		}
        try {
            prog->Codegen();
        } 
        catch (std::runtime_error &e) {
            cout << "semantic error: " << e.what() << endl;
            //cout << prog->str() << endl; 
            exit(EXIT_FAILURE);
        }
        delete prog;
    };

extern_list: /* extern_list can be empty */
    { decafStmtList *slist = new decafStmtList(); $$ = slist; }
	| extern_func 
    { decafStmtList *slist = new decafStmtList(); slist->push_back($1); $$ = slist;}
    | extern_list extern_func
    { decafStmtList *slist = (decafStmtList*)$1;
      slist->push_back($2); $$ = slist; }
    ;

extern_func : T_EXTERN T_FUNC T_ID T_LPAREN T_RPAREN method_type T_SEMICOLON 
    { $$ = new ExternAST(*$3, $6, new decafStmtList()); delete $3; }
	| T_EXTERN T_FUNC T_ID T_LPAREN extern_type_list T_RPAREN method_type T_SEMICOLON 
    { $$ = new ExternAST(*$3, $7, (decafStmtList*)$5); delete $3; }
    ;

extern_type: T_STRINGTYPE
    { $$ = new string("StringType");}
    | decaf_type
    { $$ = $1; }
    ;
 
extern_type_list:  extern_type
    { decafStmtList *slist = new decafStmtList();
      slist->push_back(new Type_IdAST($1)); $$ = slist; }
    | extern_type_list T_COMMA extern_type   
    { decafStmtList *slist = (decafStmtList*)$1; slist->push_back(new Type_IdAST($3)); $$ = slist; } 
    ;    

decafpackage: T_PACKAGE T_ID T_LCB field_decl_list method_decl_list T_RCB 
    { $$ = new PackageAST(*$2, (decafStmtList*)$4, (decafStmtList*)$5); delete $2; }
    ;

field_decl_list: 
    { $$ = new decafStmtList(); }
    | field_decl_list field_decl 
    { decafStmtList *slist = (decafStmtList*)$1; slist->push_back($2);}
    ;

field_decl: field T_SEMICOLON
    { $$ = $1; }
    ;

/*  */ 
field: T_VAR id_list decaf_type
    { $$ = new FieldAST($2, $3, new FieldSize()); }
    | T_VAR id_list T_LSB T_INTCONSTANT T_RSB decaf_type
    { $$ = new FieldAST($2, $6, new FieldSize($4)); }
    | T_VAR id_list decaf_type T_ASSIGN constant
    { $$ = new AssignGlobalVarAST($2,$3,$5); }
    ;

id_list: id_list T_COMMA T_ID
    { decafStmtList *slist = (decafStmtList *)$1; 
      slist->push_back(new StringAST(*$3)); 
      $$ = slist; } 
    | T_ID 
    { decafStmtList *slist = new decafStmtList(); 
      slist->push_back(new StringAST(*$1)); 
      $$ = slist; }
    ;

method_decl_list: 
    { $$ = new decafStmtList(); }
    | method_decl 
    { decafStmtList *mylist = new decafStmtList();  mylist->push_back($1); $$ = mylist;}
    | method_decl_list method_decl 
    { decafStmtList *slist = (decafStmtList*)$1; slist->push_back($2); $$ = slist;}
    ;

method_decl: /*case of empty typed_symbol_list */
	 T_FUNC T_ID T_LPAREN T_RPAREN method_type method_block 
    { $$ = new MethodAST(*$2, $5, new decafStmtList(), (MethodBlockAST*)$6); 
	  delete $2; }
    | T_FUNC T_ID T_LPAREN typed_symbol_list T_RPAREN method_type method_block
    { $$ = new MethodAST(*$2, $6, (decafStmtList*)$4, (MethodBlockAST*)$7); delete $2; }
    ;

typed_symbol_list: 
    { $$ = new decafStmtList(); }
    | idtype_list decaf_type
    { decafStmtList *slist = new decafStmtList(); 
      slist->push_back(new Type_IdAST($1,$2)); $$ = slist;}
    | typed_symbol_list T_COMMA idtype_list decaf_type    
    { decafStmtList *slist = (decafStmtList*)$1; 
      slist->push_back(new Type_IdAST($3,$4)); $$ = slist;}  
    ;

var_decl_list: 
    { $$ = new decafStmtList(); }
    | var_decl
    { decafStmtList *slist = new decafStmtList(); 
      slist->push_back($1); $$ = slist; }
    | var_decl_list var_decl 
    { decafStmtList *slist = (decafStmtList*)$1; 
      slist->push_back($2); $$ = slist;}
    ;

var_decl: T_VAR idtype_list decaf_type T_SEMICOLON
    { decafStmtList *slist = new decafStmtList();
      slist->push_back(new Type_IdAST($2, $3));    
      $$ = slist; }    
    ;

idtype_list: T_ID 
    { decafStmtList *slist = new decafStmtList(); 
      slist->push_back(new StringAST(*$1)); $$ = slist; }
    | id_list T_COMMA T_ID 
    { decafStmtList *slist = (decafStmtList*)$1; 
      slist->push_back(new StringAST(*$3)); $$ = slist; }

/*method_block in method_decl */
method_block: T_LCB T_RCB 
    { $$ = new MethodBlockAST(new decafStmtList(), new decafStmtList()); } 
    | T_LCB var_decl_list statement_list T_RCB 
    { $$ = new MethodBlockAST((decafStmtList*)$2, (decafStmtList *)$3); }
    ;

/* block using in statement DONE */ 
block: T_LCB  T_RCB 
    { $$ = new BlockAST(new decafStmtList(), new decafStmtList()); }
    | T_LCB var_decl_list statement_list T_RCB
    { $$ = new BlockAST((decafStmtList *)$2, (decafStmtList *)$3); }

/* done */
statement_list: 
    { decafStmtList *slist = new decafStmtList(); $$ = slist;}
    | statement_list statement
    { decafStmtList *slist = (decafStmtList*)$1; slist->push_back($2); $$ = slist; } 
    ;

/* NOT done */ 
statement: assign T_SEMICOLON 
    { $$ = $1; }
    | method_call T_SEMICOLON 
    { $$ = $1; }
    | T_IF T_LPAREN expr T_RPAREN block 
    { $$ = new IfStmtAST($3, (BlockAST *)$5, new BlockAST()); }
    | T_IF T_LPAREN expr T_RPAREN block T_ELSE block  
    { $$ = new IfStmtAST($3, (BlockAST *)$5, (BlockAST *)$7); }
    | T_WHILE T_LPAREN expr T_RPAREN block 
    { $$ = new WhileStmtAST($3, (BlockAST *)$5); }
    | T_FOR T_LPAREN assign_list T_SEMICOLON expr T_SEMICOLON assign_list T_RPAREN block 
    { $$ = new ForStmtAST((decafStmtList *)$3, $5, (decafStmtList *)$7, (BlockAST *)$9); } 
    | T_RETURN T_LPAREN expr T_RPAREN T_SEMICOLON
    { $$ = new ReturnStmtAST($3); }
    | T_RETURN T_LPAREN T_RPAREN T_SEMICOLON
    { $$ = new ReturnStmtAST(); }
    | T_RETURN T_SEMICOLON
    { $$ = new ReturnStmtAST(); }
    | T_BREAK T_SEMICOLON
    { $$ = new BreakStmtAST(); }
    | T_CONTINUE T_SEMICOLON
    { $$ = new ContinueStmtAST(); }
    | block 
    { $$ = $1; }
    ;

/* done */
assign: T_ID T_ASSIGN expr 
    { $$ = new AssignVarAST(*$1, $3); }
    | T_ID T_LSB expr T_RSB T_ASSIGN expr 
    { $$ = new AssignArrayLocAST(*$1, $3, $6);}
    ;

assign_list: 
    { $$ = new decafStmtList(); }
    | assign 
    { decafStmtList *slist = new decafStmtList; slist->push_back($1); $$ = slist; }
    | assign_list T_COMMA assign 
    { decafStmtList *my_list = (decafStmtList*)$1; my_list->push_back($3); $$ = my_list; }

/* done */ 
rvalue: T_ID
    { $$ = new VariableExprAST(*$1); }
    | T_ID T_LSB expr T_RSB
    { $$ = new ArrayLocExprAST(*$1, $3); }
    ;

/* NOT done  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< finish this first */ 
expr: T_LPAREN expr T_RPAREN
    { $$ = $2; }
    | rvalue
    { $$ = $1; } 
    | method_call
    { $$ = $1; }
    | constant
    { $$ = $1; }
    | expr T_PLUS expr 
    { string *str = new string("+"); $$ = new BinaryExprAST(*str, $1, $3); }
    | expr T_MINUS expr 
    { string *str = new string("-"); $$ = new BinaryExprAST(*str, $1, $3); }
    | expr T_MULT expr 
    { string *str = new string("*"); $$ = new BinaryExprAST(*str, $1, $3); }
    | expr T_DIV expr
    { string *str = new string("/"); $$ = new BinaryExprAST(*str, $1, $3); }
    | expr T_LEFTSHIFT expr
    { string *str = new string("<<"); $$ = new BinaryExprAST(*str, $1, $3); }
    | expr T_RIGHTSHIFT expr
    { string *str = new string(">>"); $$ = new BinaryExprAST(*str, $1, $3); }
    | expr T_MOD expr
    { string *str = new string("%"); $$ = new BinaryExprAST(*str, $1, $3); }
    | expr T_GT expr
    { string *str = new string(">"); $$ = new BinaryExprAST(*str, $1, $3); }
    | expr T_LT expr
    { string *str = new string("<"); $$ = new BinaryExprAST(*str, $1, $3); }
    | expr T_GEQ expr
    { string *str = new string(">="); $$ = new BinaryExprAST(*str, $1, $3); }
    | expr T_LEQ expr
    { string *str = new string("<="); $$ = new BinaryExprAST(*str, $1, $3); }
    | expr T_EQ expr
    { string *str = new string("=="); $$ = new BinaryExprAST(*str, $1, $3); }
    | expr T_NEQ expr
    { string *str = new string("!="); $$ = new BinaryExprAST(*str, $1, $3); }
    | expr T_AND expr
    { string *str = new string("&&"); $$ = new BinaryExprAST(*str, $1, $3); }
    | expr T_OR expr
    { string *str = new string("||"); $$ = new BinaryExprAST(*str, $1, $3); }
    | T_MINUS expr %prec UMINUS
    { string *str = new string("-"); $$ = new UnaryExprAST(*str, $2); }
    | T_NOT expr 
    { string *str = new string("!"); $$ = new UnaryExprAST(*str, $2); }
    ;

constant: T_INTCONSTANT
    { $$ = new NumberExprAST($1);}
    | T_CHARCONSTANTONE 
    { $$ = new NumberExprAST($1);}
    | T_CHARCONSTANTTWO 
    { $$ = new NumberExprAST($1);}
    | bool_expr
    { $$ = $1;}

bool_expr: T_TRUE 
    { $$ = new BoolExprAST("True"); }
    | T_FALSE 
    { $$ = new BoolExprAST("False"); }

/* done */
method_call: T_ID T_LPAREN method_arg_list T_RPAREN
    { $$ = new MethodCallAST(*$1, (decafStmtList *)$3);}
    | T_ID T_LPAREN T_RPAREN
    { $$ = new MethodCallAST(*$1, new decafStmtList()); } 
    ;

/* done */
method_arg_list: method_arg 
    { decafStmtList *slist = new decafStmtList(); slist->push_back($1); $$ = slist; }
    | method_arg_list T_COMMA method_arg
    { decafStmtList *my_list = (decafStmtList*)$1; my_list->push_back($3); $$ = my_list; }
    ;

/* done */
method_arg: T_STRINGCONSTANT
    { $$ = new StringConstAST(*$1); }  
    | expr
    { $$ = $1; } 
    ;

decaf_type: T_INTTYPE
    { $$ = new string("IntType"); }
    | T_BOOLTYPE
    { $$ = new string("BoolType"); }
    ;

method_type: T_VOID
    { $$ = new string("VoidType");}
    | decaf_type
    { $$ = $1; }
    ;

%%

int main() {
  // initialize LLVM
  llvm::LLVMContext &Context = TheContext;
  // Make the module, which holds all the code.
  TheModule = new llvm::Module("Test", Context);
  // set up symbol table

  // set up dummy main function
  //TheFunction = gen_main_def();


  // parse the input and create the abstract syntax tree
  int retval = yyparse();

  // remove symbol table

  // Finish off the main function. (see the WARNING above)
  // return 0 from main, which is EXIT_SUCCESS
  //Builder.CreateRet(llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0)));

  // Validate the generated code, checking for consistency.
  //verifyFunction(*TheFunction);
  // Print out all of the generated code to stderr
  TheModule->print(llvm::errs(), nullptr);
  return(retval >= 1 ? EXIT_FAILURE : EXIT_SUCCESS);
}

