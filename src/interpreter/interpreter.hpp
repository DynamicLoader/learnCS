#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

#if defined(ENABLE_INTPR) & ENABLE_INTPR

#ifndef __x86_64
#error The interpreter mode only support x86-64 target!
#endif

#include <istream>
#include <ostream>
#include <utility>
#include <string>
#include <vector>
#include <stack>

#include <asmjit/asmjit.h>

#include <cereal/archives/binary.hpp>

#include "symbols.hpp"


namespace Compiler
{

namespace ExternalFuncs
{
static void prints(const char *s)
{
    std::cout << s;
}

static void printsln(const char *s)
{
    std::cout << s << std::endl;
}

static void printInt(int i)
{
    std::cout << i;
}

static int readInt()
{
    int i = 0;
    std::cin >> i;
    return i;
}
} // namespace ExternalFuncs

class Interpreter
{
  private:
    asmjit::JitRuntime rt;
    asmjit::CodeHolder ch;
    asmjit::FileLogger flog;

    struct func_t
    {
        std::string name;
        asmjit::Label fnl;
        asmjit::FuncSignatureBuilder *fs = nullptr;
    };
    std::vector<func_t> funclist;

    struct var_t
    {
        int obj = 0;
        asmjit::TypeId type = asmjit::TypeId::kVoid;
#ifdef __x86_64
        asmjit::x86::Gp gp;   // 0
        asmjit::x86::Mem mem; // 1
#endif
    };

    std::map<std::string, var_t> globals;
    std::map<std::string, void *> extfunc;

#ifdef __x86_64
    bool _processFunc(asmjit::x86::Compiler &cc, const callable_t &func, const idmap_t &id,
                      stmts_t::const_iterator stbegin, stmts_t::const_iterator stend);
#endif

  public:
    Interpreter() :flog(stderr)
    {
        ch.init(rt.environment(), rt.cpuFeatures());
        flog.addFlags(asmjit::FormatFlags::kRegType | asmjit::FormatFlags::kRegCasts |
                      asmjit::FormatFlags::kExplainImms);
        // ch.setLogger(&flog);

        initExtFunc();
    }
    ~Interpreter()
    {
        for (auto x : funclist)
        {
            if (x.fs)
                delete x.fs;
        }
    };

    void initExtFunc(const std::string &s = "")
    {
        using namespace ExternalFuncs;
        if (s == "")
        {
            extfunc = {
                {"prints", (void *)&prints},
                {"printsln", (void *)&printsln},
                {"readInt", (void *)&readInt},
                {"printInt", (void *)&printInt},
            };
        }
    }

    int proc(const stmts_t &s, const CompiledInfo_t &cinfo);

    int proc(const std::pair<stmts_t, CompiledInfo_t>& data)
    {
        return proc(data.first, data.second);
    }

    int run(const std::string &entry);

    static void embed(std::ostream &os, const stmts_t &s, const CompiledInfo_t &cinfo)
    {
        cereal::BinaryOutputArchive boa(os);
        boa(s, cinfo);
    }

    static auto extract(std::istream &is)
    {
        stmts_t s;
        CompiledInfo_t cinfo;
        cereal::BinaryInputArchive bia(is);
        bia(s, cinfo);
        return std::make_pair(s, cinfo);
    }
};

} // namespace Compiler

#endif

#endif