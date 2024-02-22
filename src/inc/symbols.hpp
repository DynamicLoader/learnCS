#ifndef __SYMBOLS_HPP__
#define __SYMBOLS_HPP__

#include <string>
#include <vector>
#include <map>

#include "common.h"

#if defined(ENABLE_INTPR) && ENABLE_INTPR

#include <cereal/types/string.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#endif

namespace Compiler
{

enum simple_types
{
    type_void = 0,
    type_int,
    type_char,
    type_bool,
    type_real,

    type_pointer = 0x80,
};

struct identifier
{
    simple_types type;
    int attr;
    int addr;
    std::string initval;

#if defined(ENABLE_INTPR) && ENABLE_INTPR
    template <class Ar> void serialize(Ar &ar)
    {
        ar(type, attr, addr, initval);
    }
#endif
};

struct expression
{
    bool in_temp;
    int addr;

    std::string result_str;

    std::string get_result(void)
    {
        if (in_temp)
        {
            return "$" + std::to_string(addr);
        }
        else
        {
            return result_str;
        }
    }
};

struct statement
{
    std::string while_head;
    std::string repeat_head;
};

struct callable_t
{
    std::string name;
    simple_types retval_type;
    std::vector<std::string> args_name;
    std::vector<identifier> args_attr;
    enum
    {
        NONE = 0,
        FUNC,
        EXT,
        MAIN
    } type;

#if defined(ENABLE_INTPR) && ENABLE_INTPR
    template <class Ar> void serialize(Ar &ar)
    {
        ar(name, retval_type, args_name, args_attr,type);
    }
#endif
};

using idmap_t = std::map<std::string, identifier>;

struct quaternion_t
{
    // We use op = LABEL, ret = {label_name} as label marker
    std::string op;
    std::string a1;
    std::string a2;
    std::string ret;

#if defined(ENABLE_INTPR) && ENABLE_INTPR
    template <class Ar> void serialize(Ar &ar)
    {
        ar(op, a1, a2, ret);
    }
#endif
};

struct CompiledInfo_t
{
    std::map<int, idmap_t> ids;
    std::vector<callable_t> callable_symbols;

#if defined(ENABLE_INTPR) && ENABLE_INTPR
    template <class Ar> void serialize(Ar &ar)
    {
        ar(ids, callable_symbols);
    }
#endif
};

using stmts_t = std::vector<quaternion_t>;

} // namespace Compiler

#endif