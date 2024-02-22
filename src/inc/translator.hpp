#ifndef __TRANSLATOR_HPP__
#define __TRANSLATOR_HPP__

#include <iostream>
#include <string>
#include <map>

#include "emitter.hpp"

namespace Compiler
{

class Translator
{
  protected:
    std::ostream *_dest = &std::cout;
    Emitter *_emitter = nullptr;

    virtual std::string dump_string(const std::string &s)
    {
        return "\"" + s + "\"";
    }

  public:
    virtual void setOutput(std::ostream &out)
    {
        _dest = &out;
    }

    virtual void setEmitter(Emitter *e)
    {
        if (!e)
            return;
        _emitter = e;
    }

    virtual void output()
    {
        if (!_emitter)
            throw std::string("null emitter!");

        auto ir_list = _emitter->getQuaternionList();
        for (auto ir : ir_list)
        {
            if (ir.op == "LABEL")
            {
                *_dest << ir.ret << ":" << std::endl;
                continue;
            }

            if (ir.op == "COMMENT")
            {
                *_dest << "/* " << ir.ret << " */" << std::endl;
                continue;
            }

            if (ir.op == "PROCBGN")
            {
                *_dest << "/* func " << ir.ret << " { */" << std::endl;
                continue;
            }

            if (ir.op == "PROCEND")
            {
                *_dest << "/* } */" << std::endl << std::endl;
                continue;
            }

            *_dest << "{" << dump_string(ir.op) << ", " << dump_string(ir.a1) << ", " << dump_string(ir.a2) << ", "
                   << dump_string(ir.ret) << "}" << std::endl;
        }
    }
};

class SimpleTranslator : public Translator
{
  public:
    virtual void output() override
    {
        if (!_emitter)
            throw std::string("null emitter!");

        auto ir_list = _emitter->getQuaternionList();

        std::map<std::string, uint64_t> label_addr_table;
        uint64_t addr = 0;

        // Address calculation
        for (auto ir : ir_list)
        {
            if (ir.op == "LABEL")
            {
                label_addr_table.insert(std::make_pair(ir.ret, addr));
                continue;
            }

            // skip comments
            if (ir.op == "COMMENT") // || ir.op == "PROCBGN" || ir.op == "PROCEND"
            {
                continue;
            }

            addr++;
        }

        addr = 0;
        // Real spit
        for (auto ir : ir_list)
        {
            if (ir.op == "LABEL" || ir.op == "COMMENT")
            {
                continue;
            }

            if (ir.op == "PROCBGN")
            {
                *_dest << "(" << addr << ")\t("
                       << "program, " << ir.ret << ", -, -)" << std::endl;
                addr++;
                continue;
            }

            if (ir.op == "PROCEND")
            {
                *_dest << "(" << addr << ")\t(sys, -, -, -)" << std::endl;
                addr++;
                continue;
            }

            if (ir.op.at(0) == 'j')
            {
                auto x = label_addr_table.find(ir.ret);

                if (ir.a1.empty())
                    ir.a1 = "-";
                if (ir.a2.empty())
                    ir.a2 = "-";

                if (x != label_addr_table.end())
                {
                    ir.ret = std::to_string(x->second);
                }
            }

            *_dest << "(" << addr << ")\t(" << ir.op << ", " << ir.a1 << ", " << ir.a2 << ", " << ir.ret << ")"
                   << std::endl;
            addr++;
        }
    }
};

} // namespace Compiler

#endif