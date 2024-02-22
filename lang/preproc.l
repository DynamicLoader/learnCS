%option c++
%option noyywrap

%option batch
%option debug

%option yyclass="preScanner"
%option prefix="pre"

%{

  
  #include "prescanner.hpp"
  #include "driver.hpp"
  #include "location.hh" 

  #include <string>

  #undef YY_DECL
  #define YY_DECL Compiler::Preprocessor::symbol_type Compiler::Prescanner::nextToken()


  #define YY_USER_ACTION  _drv.loc.step(); _drv.loc.columns(YYLeng());

  #define yyterminate() Preprocessor::symbol_type(0,loc);

  #ifdef YY_NULL
    #undef YY_NULL
    #define YY_NULL Preprocessor::symbol_type()
  #endif

    int if_layer = 0;
%}

/* https://westes.github.io/flex/manual/Start-Conditions.html#Start-Conditions */
%s PREC 
%s IGNORE

%%

%{
    auto& loc = _drv.loc;

    if (_drv.ignore_trigger)
    {
        BEGIN(IGNORE);
        _drv.ignore_trigger = false;
    }

%}

<INITIAL>"#" {
    BEGIN(PREC);
}

<PREC>[ \t] {}

<PREC>"include"  {
    return Preprocessor::make_TINCLUDE(loc);
}

<PREC>"define" {
    return Preprocessor::make_TDEFINE(loc);
}

<PREC>"undef" {
    return Preprocessor::make_TUNDEF(loc);
}

<PREC>"ifdef" {
    return Preprocessor::make_TIFDEF(loc);
}

<PREC>"ifndef" {
    return Preprocessor::make_TIFNDEF(loc);
}

<PREC>"if" {
    return Preprocessor::make_TIF(loc);
}

<PREC>"endif" {
    return Preprocessor::make_TENDIF(loc);
}

<PREC>"(" {
    return Preprocessor::make_TLPAREN(loc);
}

<PREC>")" {
    return Preprocessor::make_TRPAREN(loc);
}

<PREC>"true" {
    return Preprocessor::make_TNUMBER(1, loc);
}

<PREC>"false" {
    return Preprocessor::make_TNUMBER(0, loc);
}

<PREC>"and" {
    return Preprocessor::make_TAND(loc);
}

<PREC>"or" {
    return Preprocessor::make_TOR(loc);
}

<PREC>"not" {
    return Preprocessor::make_TNOT(loc);
}

<PREC>[0-9]+ {
    std::string s = yytext;
    return Preprocessor::make_TNUMBER(std::stoi(s), loc);
}

<PREC>\{.+\} {
    std::string s = yytext;
    return Preprocessor::make_TINCCONTENT(s.substr(1,s.size()-2), loc);
}

<PREC>[A-Za-z][A-Za-z0-9]* {
    std::string s = yytext;
    return Preprocessor::make_TID(s, loc);
}

<PREC>\n {
    _drv.loc.lines(1);
    BEGIN(INITIAL);
}

<INITIAL>[^#\n]* {
    return Preprocessor::make_passthrough(yytext, loc);
}

<INITIAL>\n {
    _drv.loc.lines(1);
    return Preprocessor::make_passthrough(yytext, loc);
}

<IGNORE>"#if" {
    ++if_layer;
}

<IGNORE>"#ifdef" {
    ++if_layer;
}

<IGNORE>"#ifndef" {
    ++if_layer;
}

<IGNORE>"#endif" {
    if (if_layer == 0)
    {
        BEGIN(INITIAL);
        return Preprocessor::make_TENDIF(loc);
    }
    else
    {
        --if_layer;
    }
}

<IGNORE>\n {
    _drv.loc.lines(1);
}

<IGNORE>. {}

<<EOF>> {
    return yyterminate();
}

%%
