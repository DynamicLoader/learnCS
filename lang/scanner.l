
%option c++
%option noyywrap 

%option batch
%option debug

%option yyclass="Scanner"
%option prefix="basic"

%{
  #include <iostream>
  #include <cerrno>
  #include <climits>
  #include <cstdlib>
  #include <string>

  #include "driver.hpp"
  #include "parser.hpp"  
  #include "scanner.h"   
  #include "location.hh" 

  #undef YY_DECL
  #define YY_DECL Compiler::Parser::symbol_type Compiler::Scanner::nextToken()


  #define YY_USER_ACTION  _drv.loc.step(); _drv.loc.columns(YYLeng());

  #define yyterminate() Parser::symbol_type(0,loc);

  #ifdef YY_NULL
    #undef YY_NULL
    #define YY_NULL Parser::symbol_type()
  #endif

  using namespace Compiler;

%}


letter [a-zA-Z]
digit [0-9]
pdigit [1-9]

id {letter}({letter}|{digit})* 
number (0|{pdigit}{digit}*) 
char_lit '.*'

%%

%{
    auto& loc = _drv.loc;
%}

("/*"[^\n]*"*/") {}

"\n" {
    loc.lines(1);
}

"and" {
    return Parser::make_TAND(loc);
}

"array" {
    return Parser::make_TARRAY(loc);
}

"begin" {
    return Parser::make_TBEGIN(loc);
}

"bool" {
    return Parser::make_TBOOL(loc);
}

"call" {
    return Parser::make_TCALL(loc);
}

"case" {
    return Parser::make_TCASE(loc);
}

"char" {
    return Parser::make_TCHAR(loc);
}

"constant" {
    return Parser::make_TCONSTANT(loc);
}

"dim" {
    return Parser::make_TDIM(loc);
}

"do" {
    return Parser::make_TDO(loc);
}

"else" {
    return Parser::make_TELSE(loc);
}

"end" {
    return Parser::make_TEND(loc);
}

"false" {
    return Parser::make_TFALSE(loc);
}

"for" {
    return Parser::make_TFOR(loc);
}

"if" {
    return Parser::make_TIF(loc);
}

"input" {
    return Parser::make_TINPUT(loc);
}

"integer" {
    return Parser::make_TINTEGER(loc);
}

"not" {
    return Parser::make_TNOT(loc);
}

"of" {
    return Parser::make_TOF(loc);
}

"or" {
    return Parser::make_TOR(loc);
}

"output" {
    return Parser::make_TOUTPUT(loc);
}

"procedure" {
    return Parser::make_TPROCEDURE(loc);
}

"program" {
    return Parser::make_TPROGRAM(loc);
}

"read" {
    return Parser::make_TREAD(loc);
}

"real" {
    return Parser::make_TREAL(loc);
}

"repeat" {
    return Parser::make_TREPEAT(loc);
}

"set" {
    return Parser::make_TSET(loc);
}

"stop" {
    return Parser::make_TSTOP(loc);
}

"then" {
    return Parser::make_TTHEN(loc);
}

"to" {
    return Parser::make_TTO(loc);
}

"true" {
    return Parser::make_TTRUE(loc);
}

"until" {
    return Parser::make_TUNTIL(loc);
}

"var" {
    return Parser::make_TVAR(loc);
}

"while" {
    return Parser::make_TWHILE(loc);
}

"write" {
    return Parser::make_TWRITE(loc);
}

    /* Lang Ext 202301.4 Keywords */

"void" {
    _drv.extension_check("Keyword 'void'");
    return Parser::make_TVOID(loc);
}

"return" {
    _drv.extension_check("Keyword 'return'");
    return Parser::make_TRETURN(loc);
}

"break" {
    _drv.extension_check("Keywords 'break'");
    return Parser::make_TBREAK(loc);
}

"(" {
    return Parser::make_TLPHAR(loc);
}

")" {
    return Parser::make_TRPHAR(loc);
}

"[" {
    return Parser::make_TLSQUARE(loc);
}

"]" {
    return Parser::make_TRSQUARE(loc);
}

"+" {
    return Parser::make_TPLUS(loc);
}

"-" {
    return Parser::make_TMINUS(loc);
}

"*" {
    return Parser::make_TASRIC(loc);
}

"/" {
    return Parser::make_TDIV(loc);
}

"%" {
    return Parser::make_TMOD(loc);
}

":=" {
    return Parser::make_TASSIGN(loc);
}

"=" {
    return Parser::make_TEQUAL(loc);
}

"<>" {
    return Parser::make_TNEQ(loc);
}

"<" {
    return Parser::make_TLESS(loc);
}

"<=" {
    return Parser::make_TLE(loc);
}

">" {
    return Parser::make_TGREAT(loc);
}

">=" {
    return Parser::make_TGE(loc);
}

"," {
    return Parser::make_TCOMMA(loc);
}

";" {
    return Parser::make_TSCOLON(loc);
}

":" {
    return Parser::make_TCOLON(loc);
}

"." {
    return Parser::make_TDOT(loc);
}

".." {
    return Parser::make_TDOTS(loc);
}

[\t \a]+ {
    // Ignore whitespace.
}

{number} {
    int tmp = std::stoll(yytext);
    return Parser::make_TINT_LIT(tmp, loc);
}

{id} {
    std::string id = (yytext);
    try
    {
        identifier fid = _drv.lookup_id(id);
        if((int)fid.type & simple_types::type_pointer)
            return Parser::make_TID_PTR(id,loc);
        
        switch(fid.type){
            case simple_types::type_bool:
                return Parser::make_TID_BOOL(id,loc);
            case simple_types::type_int:
            case simple_types::type_real:
               return Parser::make_TID_NUMB(id,loc);
            case simple_types::type_char:
               return Parser::make_TID_CHAR(id,loc);
            default:
               return Parser::make_TID(id, loc);
        }
    }
    catch(Parser::syntax_error e)
    {
        return Parser::make_TID(id, loc);
    }

}

{char_lit} {
    return Parser::make_TCHAR_LIT(yytext, loc);
}

<<EOF>> {
    return yyterminate();
}

%% 
