#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <iostream>
#include <queue>
#include <map>
#include <stack>
#include <type_traits>
#include <sstream>

#include "common.h"
#include "symbols.hpp"
#include "utils.hpp"

#include "scanner.h"
#include "parser.hpp"
#include "emitter.hpp"
#include "boolchain.hpp"
#include "preprocDriver.hpp"
#include "translator.hpp"
#include "optimizer.hpp"

#include "defineTable.hpp"

namespace Compiler
{

class DriverImpl : public CompiledInfo_t
{
  public:
    PreprocDriver preprocDriver;
    Scanner scanner;
    Parser parser;
    Emitter emit;
    Optimizer optim;

    DefineTable defineTable;

    int eval_stack{0};
    int if_cnt{0};
    int while_cnt{0};
    int repeat_cnt{0};

    int id_cnt;
    std::queue<std::string> id_queue;
    location loc;

    int label_cnt = 0;
    std::stack<std::string> label_outside;
    std::stack<std::string> label_false;
    std::stack<std::string> loop_false;

    int callable_syms_cnt = 0; // also "current context"

    Utils::Logger dbg;

    drv_opts_t opts;

  public:
    DriverImpl() : parser(*this), scanner(*this), preprocDriver(*this, defineTable), optim(*this, emit)
    {
        // global ids
        ids.emplace(std::make_pair(-1, std::map<std::string, identifier>()));
    }
    virtual ~DriverImpl() = default;

    void options(drv_opts_t o)
    {
        opts = o;
        optim.options(o);
        dbg.setOutputLevel(o.loglevel);
        dbg.setColorful(o.log_colorful);

        for (auto x : o.defs)
        {
            std::string s = x;
            auto sp = s.find_first_of("=");
            std::string so = "";
            if (sp != std::string::npos)
            {
                so = s.substr(sp);
            }
            defineTable.add(s.substr(0, sp), so);
        }
    }

    drv_opts_t options()
    {
        return opts;
    }

    int parse(std::istream &in = std::cin, std::ostream &dbg = std::cerr)
    {
        std::stringstream mid;

        this->dbg.setOutput(dbg);

        preprocDriver.process(in, &mid);
        scanner.switch_streams(mid, dbg);
        return parser.parse();
    }

    int parse(std::string infile, std::ostream &dbg = std::cerr)
    {
        std::stringstream mid;

        this->dbg.setOutput(dbg);

        preprocDriver.process(infile, &mid);
        scanner.switch_streams(mid, dbg);
        return parser.parse();
    }

    auto getStmts()
    {
        return emit.getQuaternionList();
    }

    auto getCompiledInfo()
    {
        return (CompiledInfo_t) * this;
    }

    std::string entry()
    {
        for (auto x : callable_symbols)
        {
            if (x.type == callable_t::MAIN)
                return x.name;
        }
        return callable_symbols.rbegin()->name; // defaults to the last one
    }

    // Compile related functions

    void register_id(const std::string &name, identifier id, bool global = false)
    {
        if (ids.find(callable_syms_cnt) == ids.end())
        {
            ids.emplace(std::make_pair(callable_syms_cnt, std::map<std::string, identifier>()));
        }
        auto &regids = ids[global ? -1 : callable_syms_cnt];
        if (regids.find(name) != regids.end())
            throw Parser::syntax_error(loc, "Redefinition of identifier: " + name);

        regids.insert(std::make_pair(name, id));
    }

    identifier lookup_id(const std::string &name)
    {
        if (ids.find(callable_syms_cnt) == ids.end())
        {
            ids.emplace(std::make_pair(callable_syms_cnt, std::map<std::string, identifier>()));
        }
        auto &regids = ids[callable_syms_cnt];
        auto r = regids.find(name);
        if (r == regids.end())
        {
            auto f = ids[-1].find(name);
            if (f == ids[-1].end())
                throw Parser::syntax_error(loc, "undefined identifier: " + name);
            return f->second;
        }
        return r->second;
    }

    void register_idqueue(simple_types current_type, int attr = 0, bool global = false, std::string initval = "")
    {
        while (!id_queue.empty())
        {
            identifier id;
            id.type = current_type;
            id.attr = attr;
            id.initval = initval;
            id.addr = ++id_cnt;

            dbg << (global ? " added global id \"" : "added id \"") << id_queue.front();
            dbg << "\" as type " << current_type << " with attr = " << attr << ", initval = " << initval << std::endl;

            register_id(id_queue.front(), id, global);
            id_queue.pop();
        }
    }

    callable_t proc_initargs()
    {
        auto args = ids[callable_syms_cnt];
        callable_t ret;
        for (auto x = args.begin(); x != args.end(); ++x)
        {
            ret.args_name.push_back(x->first);
            ret.args_attr.push_back(x->second);
            emit.pop_arg(x->first);
        }
        return ret;
    }

    void begin_context(const std::string &name, simple_types st = type_void, bool main = false)
    {
        emit.begin_proc(name);
        auto cbl = proc_initargs();
        cbl.name = name;
        cbl.retval_type = st;
        cbl.type = (main ? callable_t::MAIN : callable_t::FUNC);
        callable_symbols.push_back(cbl);
    }

    void end_context(bool ext = false)
    {
        emit.end_proc();
        if (ext)
            callable_symbols[callable_syms_cnt].type = callable_t::EXT;
        ++callable_syms_cnt;
    }

    auto find_callable(const std::string &name)
    {
        for (auto x : callable_symbols)
        {
            if (x.name == name)
                return x;
        }
        throw std::invalid_argument("Undefined callable");
    }

    std::string calls(const std::string &proc)
    {
        extension_check("Procedure Calls");
        auto x = find_callable(proc);
        emit.call(proc);
        if (x.retval_type == simple_types::type_void)
            return "";
        else
            return "$" + std::to_string(++eval_stack);
    }

    template <typename T, typename... Ts> void type_check(const std::string &msg, T tid, Ts... types)
    {
        if (!Utils::assert_types(tid, types...))
            throw Parser::syntax_error(this->loc, msg);
    }

    template <typename T, typename... Ts> void args_check(const std::string &msg, T i, T v, Ts... args)
    {
        if (!Utils::assert_pair(i, v, args...))
            throw std::runtime_error(msg);
    }

    void extension_check(const std::string &feature)
    {
        if (opts.langver <= 0)
            throw std::runtime_error("Language Extision [" + feature +
                                     "] is used but not enabled! Did you forget to use -e parameter?");
    }

    template <typename TL, typename TR1, typename TR2> void eval(char op, TL &left, TR1 r1, TR2 r2)
    {
        emit.expr(op, r1.get_result(), r2.get_result(), "$" + std::to_string(++eval_stack));
        left.in_temp = true;
        left.addr = eval_stack;
    }

    std::string gen_label()
    {
        return "label_" + std::to_string(++label_cnt);
    }

    template <class T = Translator>
    typename std::enable_if<std::is_base_of<Translator, T>::value, int>::type output(std::ostream &os = std::cout)
    {
        emit.detect_unused_jmp();
        T tr;
        tr.setEmitter(&emit);
        tr.setOutput(os);
        tr.output();
        return 0;
    }
};

// Endpoint
class Driver : private DriverImpl
{
  public:
    using DriverImpl::entry;
    using DriverImpl::getCompiledInfo;
    using DriverImpl::getStmts;
    using DriverImpl::options;
    using DriverImpl::output;
    using DriverImpl::parse;
};
} // namespace Compiler
#endif
