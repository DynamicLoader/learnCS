#ifndef __IR_EMIT_HPP__
#define __IR_EMIT_HPP__

#include "symbols.hpp"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

namespace Compiler
{

// @todo: To be filled and apply globally
namespace IR
{
constexpr const char *JLE = "j<";
}

class Emitter
{

  public:

    using quaternion_list_t = stmts_t;

  private:
    quaternion_list_t _global_stmt;
    quaternion_list_t *_stmt = &_global_stmt;
    std::vector<int> _label_list;

    bool _isJmp(const std::string &s)
    {
        return (s.at(0) == 'j');
    }

  public:
    Emitter() = default;

    virtual ~Emitter() = default;

    void assignment(std::string left, std::string right)
    {
        _stmt->push_back(quaternion_t{.op = ":=", .a1 = right, .a2 = "-", .ret = left});
    }

    void expr(char op, std::string left, std::string right, std::string result)
    {
        _stmt->push_back(quaternion_t{.op = std::string(&op, 1), .a1 = left, .a2 = right, .ret = result});
    }

    void jmp(std::string label)
    {
        _stmt->push_back(quaternion_t{.op = "j", .ret = label});
    }

    void cond_jmp(std::string cond, std::string l, std::string r, std::string label)
    {
        _stmt->push_back(quaternion_t{.op = "j" + cond, .a1 = l, .a2 = r, .ret = label});
    }

    void label(std::string name)
    {
        _stmt->push_back(quaternion_t{.op = "LABEL", .ret = name});
        _label_list.push_back(_global_stmt.size() + _stmt->size() - 1);
    }

    void comment(std::string cmt)
    {
        _stmt->push_back(quaternion_t{.op = "COMMENT", .ret = cmt});
    }

    /// @todo
    void replace_label(std::string ol, std::string nl)
    {
        if (ol == "" || nl == "")
            return;
        for (auto x = _stmt->begin(); x != _stmt->end(); ++x)
        {
            if (x->op == "LABEL" || _isJmp(x->op))
            {
                if (x->ret == ol)
                {
                    x->ret = nl;
                }
            }
        }
    }

    void push_arg(const std::string &v)
    {
        _stmt->push_back(quaternion_t{.op = "PUSHARG", .ret = v});
    }

    void pop_arg(const std::string &v)
    {
        _stmt->push_back(quaternion_t{.op = "POPARG", .ret = v});
    }

    void ret(const std::string &value)
    {
        _stmt->push_back(quaternion_t{.op = "RETURN", .ret = value});
    }

    void call(const std::string &procname)
    {
        _stmt->push_back(quaternion_t{.op = "CALL", .a1 = procname});
    }

    void patch_call_ret(const std::string &ret)
    {
        // _stmt->back().ret = ret;
        for (auto x = _stmt->rbegin(); x != _stmt->rend(); ++x)
        {
            if (x->op == "CALL")
            {
                x->ret = ret;
                return;
            }
        }
        throw std::runtime_error("Cannot find a valid call inst to patch!");
    }

    void begin_proc(const std::string &name)
    {
        if (_stmt != &_global_stmt)
            throw std::runtime_error("Sub proc within a proc is not allow!");
        _stmt = new quaternion_list_t;
        _stmt->push_back(quaternion_t{.op = "PROCBGN", .ret = name});
    }

    void end_proc()
    {
        if (_stmt == &_global_stmt)
            throw std::runtime_error("No sub proc to end!");
        _stmt->push_back(quaternion_t{.op = "PROCEND"});
        for (auto x : *_stmt)
            _global_stmt.push_back(x);
        delete _stmt;
        _stmt = &_global_stmt;
    }

    void detect_unused_jmp()
    {
        if (_stmt != &_global_stmt)
            throw std::runtime_error("Cannot detect unused jmp in sub proc!");

        auto verify = [&](const quaternion_t &q) -> int {
            for (auto x : _label_list)
            {
                if (_stmt->at(x).ret == q.ret)
                    return x;
            }
            return -1;
        };
        int cnt = 0;
        for (auto x = _global_stmt.begin(); x < _global_stmt.end(); ++x)
        {
            if (_isJmp(x->op))
            {
                int v = verify(*x);
                if (v < 0)
                    x->op = (++cnt, "-UNRESOLVED-" + x->op);
            }
        }
        if (cnt)
            std::cerr << "Detected " + std::to_string(cnt) + " unresolved jumps" << std::endl;
    }

    auto getQuaternionList()
    {
        return _global_stmt;
    }

  protected:
    friend class Optimizer;
};

} // namespace Compiler

#endif