%require "3.5"
%language "c++"

%define lr.type lalr
%define api.namespace { Compiler } 
%define api.parser.class { Preprocessor }

%define api.token.constructor
%define api.value.type variant 
%define parse.assert
%defines

%code requires
{
    #include <iostream>
    #include <string>
    #include <vector>
    #include <cstdint>
    #include <cmath>
    #include <cmath>
    #include "common.h"
}

%code top
{

  #include <iostream>
  #include "preproc.hpp"
  #include "driver.hpp"
  #include "location.hh"

  namespace Compiler{
    static Preprocessor::symbol_type yylex(PreprocDriver& drv){
        return drv.pscan.nextToken();
    }

    void Preprocessor::error(const location& loc, const std::string& msg)
    {
        std::cout << loc << ": " << msg << std::endl;
        throw msg;
    }

  }

  using namespace Compiler;

}

%param { Compiler::PreprocDriver &drv }
%locations
%define parse.error verbose

%token 
    TINCLUDE

    TDEFINE
    TUNDEF
    TIF
    TENDIF

    TIFDEF
    TIFNDEF

    TAND
    TOR
    TNOT

    TLPAREN
    TRPAREN

%token <std::string> TSTRING

%token <std::string> TID
%token <std::string> TINCCONTENT
%token <int> TNUMBER

%token <std::string> passthrough

%start lines

%type <bool> bool_expr bool_term bool_factor bool_primary
%type <std::string> anystring


%%

lines : line
      | lines line 

line    : include
        | define_stmt
        | undef_stmt
        | if_stmt
        | pass

pass    : passthrough
        {
            *drv.result << $1;
        }

include     : TINCLUDE TINCCONTENT
            {
                // drv.drv.dbg << "Including " << $2 << std::endl;

                PreprocDriver newdrv(drv.drv, drv.table);
                newdrv.process($2, drv.result);
            }

if_stmt     : if_header lines TENDIF
            { 
            }
            |
            if_header TENDIF
            {
            }
        
if_header   : TIF bool_expr
            {
                // drv.drv.dbg << "Evaluation: " << ($2 ? "true" : "false") << std::endl;

                if (!$2)
                    drv.ignore_trigger = true;
            }
            | TIFDEF TID
            {
                // drv.drv.dbg << "Evaluation: " << (drv.table.contains($2) ? "true" : "false") << std::endl;

                if (!drv.table.contains($2))
                    drv.ignore_trigger = true;
            }
            | TIFNDEF TID
            {
                // drv.drv.dbg << "Evaluation: " << (!drv.table.contains($2) ? "true" : "false") << std::endl;

                if (drv.table.contains($2))
                    drv.ignore_trigger = true;
            }

bool_expr   : bool_term
            {
                $$ = $1;
            }
            | bool_expr TOR bool_term
            {
                $$ = $1 || $3;
            }

bool_term   : bool_factor
            {
                $$ = $1;
            }
            | bool_term TAND bool_factor
            {
                $$ = $1 && $3;
            }

bool_factor : TNOT bool_primary
            {
                $$ = !$2;
            }
            | bool_primary
            {
                $$ = $1;
            }
            | TLPAREN bool_expr TRPAREN
            {
                $$ = $2;
            }

bool_primary: TID 
            {
                if (drv.table.contains($1))
                {
                    $$ = bool(drv.table[$1]);
                } 
                else 
                {
                    $$ = false;
                }
            }
            | TNUMBER
            {
                $$ = bool($1);
            }

define_stmt : TDEFINE TID
            {
                drv.table.add($2, "");
            }
            | TDEFINE TID anystring
            {
                drv.table.add($2, $3);
            }

undef_stmt  : TUNDEF TID
            {
                drv.table.remove($2);
            }

anystring : TSTRING
          {
              $$ = $1;
          }
          | TID
          {
              $$ = $1;
          }
          | TNUMBER
          {
              $$ = std::to_string($1);
          }
%%

