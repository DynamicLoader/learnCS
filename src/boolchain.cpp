#include "boolchain.hpp"
#include "driver.hpp"

namespace Compiler
{

std::string BoolChain::process()
{
    // subchains will be processed by conditon function in sort, no need to specially call them
    std::string lastExport = "";
    auto finalExport = _drv.gen_label();

    // for (auto e : _evals)
    for (auto e = _evals.begin(); e != _evals.end(); ++e)
    {
        auto x = e->first;
        auto t = e->second;
        switch (_ctype)
        {
        case EXP: {
            if (_evals.size() > 1)
                throw std::invalid_argument("Unexpected EXP boolchain with more than one evals");
            finalExport = x(_drv);
        }
        break;
        case AND: {
            auto r = x(_drv);
            if (e != _evals.end() - 1)
            {
                if (t == OR || t == EXP) // reverse the default context state
                {
                    _drv.emit.jmp(finalExport);
                    _drv.emit.label(r);
                }
                else
                {
                    _drv.emit.replace_label(r, finalExport);
                }
            }
            else
            {
                _drv.emit.label(finalExport);
                finalExport = r;
            }
        }
        break;
        case OR: {
            auto r = x(_drv);
            // if (t == AND)
            // {
            //     _drv.emit.jmp(finalExport);
            //     _drv.emit.label(r);
            // }
            // else
            // {
            // if (lastExport != "")
            _drv.emit.replace_label(lastExport, r); // link
            lastExport = r;
            // }
        }
        break;
        default:
            throw std::invalid_argument("Unexpected type of boolchain");
        }
    }
    // if (_ctype == OR)
    _drv.emit.replace_label(lastExport, finalExport);

    if (_invert)
    {
        auto oldFinalExport = finalExport;
        finalExport = _drv.gen_label();
        _drv.emit.jmp(finalExport);
        _drv.emit.label(oldFinalExport);
    }

    return finalExport;
}

std::string BoolChain::always_true(DriverImpl &drv)
{
    auto ret = drv.gen_label();
    drv.emit.comment("Cond: Always true");
    drv.emit.jmp(ret);
    return ret;
}

std::string BoolChain::always_false(DriverImpl &drv)
{
    drv.emit.comment("Cond: Always false");
    return drv.gen_label(); // Unused, but it will replace all true labels in the eval
}

BoolChain::preEvalFunc_t BoolChain::if_jumps(const char *cond, const std::string &left, const std::string &right)
{
    return [cond, left, right](DriverImpl &drv) -> std::string {
        drv.emit.comment(std::string("Cond: ") + cond);
        auto ret = drv.gen_label();
        drv.emit.cond_jmp(cond, left, right, ret);
        return ret;
    };
}

} // namespace Compiler
