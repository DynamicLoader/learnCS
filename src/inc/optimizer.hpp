#ifndef __OPTIMIZER_HPP__
#define __OPTIMIZER_HPP__

#include <iostream>

#include "common.h"
#include "emitter.hpp"
// #include "driver.hpp"
#include "symbols.hpp"

namespace Compiler
{
class Optimizer
{
  private:
    CompiledInfo_t &_cinfo;
    Emitter &_emit;

    optimizer_opts_t _opt;

  public:
    Optimizer(CompiledInfo_t &cinfo, Emitter &emit) : _cinfo(cinfo), _emit(emit)
    {
    }

    void optimize()
    {
        
    }

    void options(const optimizer_opts_t &o)
    {
        _opt = o;
    }

    auto options()
    {
        return _opt;
    }
};
} // namespace Compiler

#endif
