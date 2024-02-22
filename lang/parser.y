/* 
    Parser definition for Simple Language

  Author: @AlexLin, @DynamicLoader 

 */

%require "3.5"
%language "c++"

%define lr.type lalr
%define api.namespace {Compiler} 
%define api.parser.class { Parser }
%define api.token.constructor
%define api.value.type variant 
%define parse.assert
%define parse.error verbose
%locations
%param { Compiler::DriverImpl & drv }

%defines // Do not remove this; it will generate necessary include files

%code requires
{

  #include <iostream>
  #include <string>
  #include <vector>
  #include <cstdint>
  #include <cmath>
  #include "common.h"
  #include "symbols.hpp"

}

%code top
{

  #include <iostream>
  #include "scanner.h"
  #include "parser.hpp"
  #include "driver.hpp"
  #include "location.hh"

  namespace Compiler{
    static Parser::symbol_type yylex(DriverImpl& drv){
        return drv.scanner.nextToken();
    }

  }

  using namespace Compiler;

}

%token
    // Keywords
    TAND 
    TARRAY
    TBEGIN
    TBOOL
    TCALL
    TCASE
    TCHAR
    TCONSTANT
    TDIM
    TDO
    TELSE
    TEND
    TFALSE
    TFOR
    TIF
    TINPUT
    TINTEGER
    TNOT
    TOF
    TOR
    TOUTPUT
    TPROCEDURE
    TPROGRAM
    TREAD
    TREAL
    TREPEAT
    TSET
    TSTOP
    TTHEN
    TTO
    TTRUE
    TUNTIL
    TVAR
    TWHILE
    TWRITE

    // Lang Ext 202301.0 Keywords 
    TRETURN
    TBREAK
    TVOID

    // Operators
    TLPHAR    // (
    TRPHAR    // )
    TASRIC    // *
    TLCOMMENT // /*
    TPLUS     // +
    TCOMMA    // ,
    TMINUS    // -
    TDOT      // .
    TDOTS     // ..
    TDIV      // /
    TRCOMMENT // */
    TCOLON    // :
    TASSIGN   // :=
    TSCOLON   // ;
    TLESS     // <
    TLE       // <=
    TNEQ       // <>
    TEQUAL    // =
    TGREAT    // >
    TGE       // >=
    TLSQUARE  // [
    TRSQUARE  //]

    TMOD // %
    
;

%token <std::string> TID "identifier"
%token <long long> TINT_LIT "integer_literal"
%token <std::string> TCHAR_LIT "character_literal"

// Lang Patch. 02 Polysemous token for identifiers
// Ref: https://www.gnu.org/software/bison/manual/bison.html#Context-Dependency
%token <std::string> TID_BOOL "identifier_bool"
%token <std::string> TID_NUMB "identifier_numberic"
%token <std::string> TID_CHAR "identifier_char"
%token <std::string> TID_PTR "identifier_pointer"

%right TNOT
%left TAND
%left TOR

%left TPLUS
%left TMINUS

/* Give higher precedence for TELSE to solve danging ELSE problem*/
%right TTHEN TELSE 

%type <simple_types> 
    type
    normal_type
    pointer_type

%type <expression> 
    expr
    term
    factor
    char_expr
    func_eval

%type <BoolChain*>
    bool_expr
    bool_term
    bool_factor
    bool_primary

%type <statement> 
    stmt
    stmts
    single_stmt
    block_stmt
    assignment
    if_stmt
    repeat_stmt
    repeat_begin
    while_stmt
    while_begin

%type <std::string>
    gp_id
    ptr_id
    lit

%start starts

%%

/* only 1 program is allowed at the end */
starts: start_repeats program | program
start_repeats : start_repeat start_repeats | start_repeat
start_repeat : procedure | global

/* Lang Ext 202301.2: procedure */

procedure: TPROCEDURE procdef TSCOLON decl  TBEGIN stmts TEND TDOT
        {
           drv.extension_check("Procedures");
            drv.end_context();
        }
        | TPROCEDURE procdef TSCOLON
        {
            drv.dbg << " as external proc" << std::endl;
            drv.extension_check("Procedures");
            drv.end_context(true); // mark as external proc
        }



procdef: TID argsdef TCOLON type 
        {
            drv.dbg << "Procedure name : " << $1 << std::endl;
            drv.begin_context($1,$4);
        }

argsdef : TLPHAR defs TRPHAR | %empty

/* Lang Ext 202301.1: globals */


global : TDIM ids TCOLON type TSCOLON
        {
            drv.extension_check("Globals");
            drv.register_idqueue($4,0,true);
        }
        |
        TCONSTANT ids TCOLON type TLPHAR lit TRPHAR TSCOLON
        {
            drv.extension_check("Globals");
            drv.register_idqueue($4,1,true,$6);
        }

lit:    TINT_LIT
        {
            $$ = std::to_string($1);
        }
        | TTRUE 
        {
            $$ = "True";
        }
        | TFALSE
        {
            $$ = "False";
        }
        | TCHAR_LIT
        {
            $$ = $1;
        }

program : program_start decl TBEGIN stmts TEND TDOT
        {
            drv.end_context();
        }

program_start : TPROGRAM TID TSCOLON
            {
                drv.begin_context($2,type_void,true); // mark as main
                drv.dbg << "Program name : " << $2 << std::endl;
            }


decl: TVAR defs | %empty

defs: def defs | def

def     : ids TCOLON type TSCOLON
        {
            drv.register_idqueue($3);
        }

ids : gp_id TCOMMA ids 
    {
        drv.id_queue.emplace($1);
    }
    | gp_id
    {
        drv.id_queue.emplace($1);
    }

gp_id:  TID {$$=$1;}
        | TID_BOOL {$$=$1;}
        | TID_CHAR {$$=$1;}
        | TID_NUMB {$$=$1;}
        | TID_PTR {$$ = $1;}

ptr_id: TDOTS gp_id { $$ = $2;}

type    : normal_type 
        | pointer_type
        {
            $$ = $1;
            drv.extension_check("Pointer");
        }

normal_type: TINTEGER 
            {
                $$ = simple_types::type_int;
            }
            | TREAL
            {
                $$ = simple_types::type_real;
            }
            | TBOOL     
            {
                $$ = simple_types::type_bool;
            }
            | TCHAR
            {
                $$ = simple_types::type_char;
            }
            | TVOID
            {
                $$ = simple_types::type_void;
            }

        /* Lang Ext 202301.4 Pointer */
pointer_type: normal_type TDOTS 
            {
                $$ = simple_types(simple_types::type_pointer + $1);
            }
            | TDOTS
            {
                $$ = simple_types::type_pointer;
            }
        
stmt: single_stmt | block_stmt

single_stmt : assignment
    {
        drv.eval_stack = 0;
    }
    /* Lang Ext. 202301.3 Procedure calls */
    | call_stmt {}
    | return_stmt {}
    | break_stmt {}
    | %empty
    {
        drv.dbg << "Warning: useless ';'" << std::endl;
    }

block_stmt: if_stmt
    {
        drv.eval_stack = 0;
    }
    | while_stmt
    {
        drv.eval_stack = 0;
    }
    | repeat_stmt
    {
        drv.eval_stack = 0;
    }
    | TBEGIN stmts TEND
    {
        drv.eval_stack = 0;
        drv.emit.comment("INLINE BLOCK END");
    }


stmts: stmt TSCOLON stmts
     | stmt

assignment  : gp_id TASSIGN expr 
            {
                identifier id = drv.lookup_id($1);

                drv.type_check("Error : cannot assign non-number value to a numberic variable", id.type, simple_types::type_real, simple_types::type_int);
                if(id.attr &1)
                    throw Parser::syntax_error(drv.loc,"Cannot assign to constant");
                drv.emit.assignment
                (
                    $1, $3.get_result()
                );
            }
            /* Lang Patch-01 Assignment to bool variable */
            | gp_id TASSIGN bool_expr
            {
                identifier id = drv.lookup_id($1);

                drv.type_check("Error : cannot assign non-bool value to a boolean variable", id.type, simple_types::type_bool);

                auto true_label = $3->process();

                drv.label_false.push(drv.gen_label());
                drv.label_outside.push(drv.gen_label());

                drv.emit.jmp(drv.label_false.top());
                drv.emit.label(true_label);
                drv.emit.assignment
                (
                    $1, "True"
                );
                drv.emit.jmp(drv.label_outside.top());
                drv.emit.label(drv.label_false.top());
                drv.emit.assignment
                (
                    $1, "False"
                );
                drv.emit.label(drv.label_outside.top());

                drv.label_false.pop();
                drv.label_outside.pop();
            }
            |  gp_id TASSIGN char_expr
            {
                identifier id = drv.lookup_id($1);
                drv.type_check("Error : cannot assign non-char value to a char variable", id.type, simple_types::type_char);

                drv.dbg.warning << "Warning: Not yet support assignment on char expr!" << std::endl;
                // drv.emit.assignment($1,$3.get_result());
            }
            /* Lang Ext. 202301.4 Pointer - assignments */
            | gp_id TASSIGN TID_PTR
            {
                identifier id = drv.lookup_id($1);
                identifier pid = drv.lookup_id($3);
                if((int) id.type & simple_types::type_pointer){
                    if(id.type != pid.type){
                        if(id.type != simple_types::type_pointer && pid.type != simple_types::type_pointer){
                            throw Parser::syntax_error(drv.loc,"Error: cannot assign pointer that has different basic type");
                        }else{
                            drv.dbg.warning << "Warning: Force type convertion detected; it may cause problems!" << std::endl;
                        }
                    }
                    drv.emit.assignment($1,$3);
                }else{
                    throw Parser::syntax_error(drv.loc,"Error: cannot assign pointer to non-pointer variable");
                }
            }
            | gp_id TASSIGN ptr_id
            {
                identifier id = drv.lookup_id($1);
                identifier pid = drv.lookup_id($3);
                if((int) id.type & simple_types::type_pointer){
                    if(((int) id.type & ~(int)simple_types::type_pointer) != (int)pid.type)
                        throw Parser::syntax_error(drv.loc,"Error: cannot assign pointer from different basic type");
                    drv.emit.assignment($1,"&" + $3);
                }else{
                    throw Parser::syntax_error(drv.loc,"Error: cannot assign pointer to non-pointer variable");
                }
            }
            

if_stmt         : if_stmt_begin TTHEN stmt
                {
                    drv.emit.comment("END IF");
                    drv.emit.label(drv.label_false.top()); // no else, false label is used as outside label
                    drv.label_false.pop();
                    drv.label_outside.pop();
                }
                | if_stmt_begin TTHEN if_with_else stmt
                {
                    drv.emit.comment("END IF");
                    drv.emit.label(drv.label_outside.top());
                    drv.label_false.pop();
                    drv.label_outside.pop();
                }

if_with_else: stmt TELSE
            {
                drv.emit.jmp(drv.label_outside.top());
                drv.emit.comment("BEGIN IF-ELSE");
                drv.emit.label(drv.label_false.top());
            }

if_stmt_begin   : TIF bool_expr 
                {
                    drv.label_outside.push(drv.gen_label());
                    
                    drv.emit.comment("BEGIN IF");
                    auto label_true = $2->process();
                    
                    drv.label_false.push(drv.gen_label());
                    drv.emit.jmp(drv.label_false.top());
                    drv.emit.comment("True Stmt");
                    drv.emit.label(label_true);
                    
                    $2->clear();
                    delete $2;
                    // follow by true statements
                }

repeat_begin    : TREPEAT
                {
                    drv.emit.comment("BEGIN REPEAT");

                    drv.label_false.push(drv.gen_label());
                    drv.loop_false.push(drv.gen_label());

                    drv.emit.label(drv.label_false.top());
                    $$.repeat_head = drv.label_false.top();
                }

repeat_stmt     : repeat_begin stmt TUNTIL bool_expr
                {
                    auto true_label = $4->process();
                    drv.emit.jmp(drv.label_false.top());
                    drv.emit.label(true_label);
                    drv.emit.label(drv.loop_false.top());

                    drv.label_false.pop();
                    drv.loop_false.pop();
                    drv.emit.comment("END REPEAT");
                }

while_begin     : TWHILE bool_expr TDO
                {
                    $$.while_head = drv.gen_label();
                    drv.label_false.push(drv.gen_label());
                    drv.loop_false.push(drv.label_false.top());

                    drv.emit.comment("BEGIN WHILE");
                    drv.emit.label($$.while_head);

                    auto label_true = $2->process();
                    drv.emit.jmp(drv.label_false.top());
                    drv.emit.label(label_true);
                }

while_stmt  : while_begin stmt
            {
                drv.emit.jmp($1.while_head);
                drv.emit.label(drv.label_false.top());

                drv.label_false.pop();
                drv.loop_false.pop();

                drv.emit.comment("END WHILE");
            }

/* Lang Ext. 202301.3 Procedure calls, ignores retval */
call_stmt   : TCALL func_eval 
            {
                drv.emit.comment("Call only, ignore retval");
            }


func_eval   : TID TLPHAR call_args TRPHAR 
            {
                $$.result_str = drv.calls($1);
            }
            | TID TLPHAR TRPHAR
            {
                $$.result_str = drv.calls($1);
            }

call_args   : expr TCOMMA call_args 
            {
                drv.emit.push_arg($1.get_result());
            }
            | expr 
            {
                drv.emit.push_arg($1.get_result());
            }
            | char_expr TCOMMA call_args
            {
                drv.emit.push_arg($1.get_result());
            }
            | char_expr
            {
                drv.emit.push_arg($1.get_result());
            }

return_stmt: TRETURN 
            {
                drv.emit.ret("");
            }
            | TRETURN expr 
            {
                drv.emit.ret($2.get_result());
            }
            | TRETURN bool_expr
            {
                drv.dbg.warning << "Returning bool is not yet supported!" << std::endl;
            }

break_stmt: TBREAK
            {
                drv.emit.jmp(drv.loop_false.top());
            }
            

expr        : term
            {
                $$ = $1;
            }
            | expr TPLUS term
            {
                drv.eval('+', $$, $1, $3);
            }
            | expr TMINUS term
            {
                drv.eval('-', $$, $1, $3);
            }

term        : term TASRIC factor
            {
                drv.eval('*', $$, $1, $3);
            }
            | term TDIV factor
            {
                drv.eval('/', $$, $1, $3);
            }
            | term TMOD factor
            {
                drv.eval('%', $$, $1, $3);
            }
            | factor
            {
                $$ = $1;
            }
            | TMINUS factor
            {  
                drv.eval('-', $$, expression{.result_str = "0"}, $2);
            }
            
factor      : TID_NUMB
            {
                identifier id = drv.lookup_id($1);

                drv.type_check("Type error : non-number type in expression", id.type, simple_types::type_int ,simple_types::type_real);

                $$.in_temp = false;
                $$.result_str = $1;
            }
            | TINT_LIT
            {
                $$.in_temp = false;
                $$.result_str = std::to_string($1);
            }
            | TLPHAR expr TRPHAR
            {
                $$ = $2;
            }
            /* Lang Ext. 202301.3 Procedure calls, as right value */
            | func_eval 
            {
                if($1.result_str == "")
                    throw Parser::syntax_error(drv.loc,"Cannot get return value from procedure with return type 'void'");
                drv.emit.patch_call_ret($1.result_str);
                $$ = $1;
            }
            

bool_expr   : bool_term
            {
                $$ = $1;
            }
            | bool_term TOR bool_expr
            {
                $$ = new BoolChain(drv,$1,$3,BoolChain::type_t::OR);
            }

bool_term   : bool_factor
            {
                $$ = $1;
            }
            | bool_factor TAND bool_term
            {
                $$ = new BoolChain(drv,$1,$3,BoolChain::type_t::AND);
            }


bool_factor : bool_primary
            {
                $$ = $1;
            }
            | TNOT bool_primary
            {
                $2->invert(!$2->invert());
                $$ = $2;
            }

bool_primary: expr TEQUAL expr
            {
                $$ = new BoolChain(drv,BoolChain::if_equal($1.get_result(),$3.get_result()));
            }
            | expr TNEQ expr
            {
                $$ = new BoolChain(drv,BoolChain::if_notequ($1.get_result(),$3.get_result()));
            }
            | expr TLESS expr
            {
                $$ = new BoolChain(drv,BoolChain::if_less($1.get_result(),$3.get_result()));
            }
            | expr TLE expr
            {
                $$ = new BoolChain(drv,BoolChain::if_leq($1.get_result(),$3.get_result()));
            }
            | expr TGREAT expr
            {
                $$ = new BoolChain(drv,BoolChain::if_greater($1.get_result(),$3.get_result()));
            }
            | expr TGE expr
            {
                $$ = new BoolChain(drv,BoolChain::if_geq($1.get_result(),$3.get_result()));
            }
            | TTRUE
            {
                $$ = new BoolChain(drv,BoolChain::always_true);
            }
            | TFALSE
            {
                $$ = new BoolChain(drv,BoolChain::always_false);
            }            
            | TLPHAR bool_expr TRPHAR
            {
                $$ = $2;
            }
            | TID_BOOL
            {
                identifier id = drv.lookup_id($1);
                drv.type_check("Error : type error.", id.type, simple_types::type_bool);

                $$ = new BoolChain(drv,BoolChain::if_equal($1,"True"));
            }

char_expr   : TCHAR_LIT
            {
                $$.in_temp = false;
                $$.result_str = $1;
            }
            | TID_CHAR
            {
                identifier id = drv.lookup_id($1);
                drv.type_check("Error : type error.", id.type,simple_types::type_char);

                $$.in_temp = false;
                $$.result_str = $1;
            }

%%