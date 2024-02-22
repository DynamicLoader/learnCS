#ifndef PREPROC_SCANNER_HPP
#define PREPROC_SCANNER_HPP

#if !defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer preFlexLexer

#include <FlexLexer.h>

#endif

#include "common.h"
#include "preproc.hpp"

// YY_DECL, redefined.

#undef YY_DECL
#define YY_DECL Compiler::Preprocessor::symbol_type Compiler::Prescanner::nextToken()

namespace Compiler
{

class PreprocDriver;

class Prescanner : public yyFlexLexer
{
    PreprocDriver &_drv;

  public:
    Prescanner(PreprocDriver &drv) : _drv(drv)
    {
    }

    virtual Preprocessor::symbol_type nextToken();
};

} // namespace Compiler

#endif