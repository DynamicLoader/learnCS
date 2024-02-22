
#ifndef __SCANNER_H__
#define __SCANNER_H__

#if !defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer basicFlexLexer

#include <FlexLexer.h>

#endif

// YY_DECL, redefined.
#undef YY_DECL
#define YY_DECL Compiler::Parser::symbol_type Compiler::Scanner::nextToken()

#include "common.h"
#include "parser.hpp"

namespace Compiler
{

class Scanner : public yyFlexLexer
{
    bool _isComment = false;
    DriverImpl &_drv;

  public:
    Scanner(DriverImpl &drv) : _drv(drv)
    {
    }

    virtual Parser::symbol_type nextToken(); // To be generated by Flex
};
} // namespace Compiler

#endif