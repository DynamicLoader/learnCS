
#if defined(INTPR_STANDALONE) 

#if __x86_64__

#include <fstream>
#include <algorithm>

#include "interpreter.hpp"
#include "symbols.hpp"
#include "utils.hpp"
#include "common.h"

using namespace Compiler;
using namespace std;

const char *magicinv = "__RPTNIDTS__"; // __STDINTPR__
#ifdef __WIN32
#include <windows.h>

wchar_t __progname[2048] = {0};
#else
extern char *__progname;
#endif

bool findMagic(std::istream &in)
{
    std::string mag = magicinv;
    std::reverse(mag.begin(), mag.end());
    int st = 0;
    char i;
    while (!in.eof() && in >> i)
    {
        st = (i == mag[st] ? st + 1 : 0);
        if (st == mag.size())
            return true;
    }
    return false;
}

int main(int argc, char **argv)
{
    // cerr << argv[0] << endl;
#ifdef __WIN32
    GetModuleFileNameW(NULL, __progname, 2040);
#endif
    ifstream fs(__progname, std::ios::binary);
    if (!fs)
    {
        cerr << "Cannot open self. Aborted." << endl;
        return -1;
    }
    if (!findMagic(fs))
    {
        cerr << "Cannot find compiled code. Interrupted." << endl;
        return -2;
    }

    Interpreter intpr;

    auto data = intpr.extract(fs);
    auto func = std::find_if(data.second.callable_symbols.begin(), data.second.callable_symbols.end(),
                             [&](callable_t cb) { return cb.type == Compiler::callable_t::MAIN; });
    if (func == data.second.callable_symbols.end())
    {
        cerr << "Cannot find entry! Aborted." << endl;
        return -3;
    }
    intpr.proc(data);
    return intpr.run(func->name);
}

#endif

#endif