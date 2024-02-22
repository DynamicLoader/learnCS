#pragma once

#include <iostream>
#include <queue>
#include <functional>
#include <string>

#include "common.h"

namespace Compiler
{

/**
 * Please note that the class should be used by build-output-clear method, no runtime remove provided.
 */
class BoolChain
{
  public:

    // The callback function is required to return a unique label of true;
    // stmts will be generated right after boolchain, and jump to outside
    using preEvalFunc_t = std::function<std::string(DriverImpl &drv)>;
    enum type_t
    {
        EXP = 0,
        AND,
        OR
    };

  private:
    std::queue<BoolChain *> _subChain;
    std::deque<std::pair<preEvalFunc_t, type_t>> _evals;
    type_t _ctype = EXP;
    bool _invert = false;

    DriverImpl &_drv;

  public:
    BoolChain(DriverImpl &drv) : _drv(drv)
    {
    }

    BoolChain(DriverImpl &drv, preEvalFunc_t e) : _drv(drv)
    {
        this->addCond(e,EXP);
    }

    BoolChain(DriverImpl &drv,BoolChain *l, BoolChain *r, type_t t) : _drv(drv)
    {
        if (l == nullptr || r == nullptr)
            throw std::string("Invalid sub-boolchain: nullptr");

        auto ret = this;
        ret->_subChain.push(l);
        ret->_subChain.push(r);
        ret->addCond([l](DriverImpl &) -> std::string { return l->process(); }, l->_ctype);
        ret->addCond([r](DriverImpl &) -> std::string { return r->process(); }, r->_ctype);
        ret->_ctype = t;

    }

    virtual ~BoolChain() = default;

    BoolChain *newBoolchain(preEvalFunc_t e)
    {
       return new BoolChain(_drv,e);
    }

    BoolChain *newBoolchain(BoolChain *l, BoolChain *r, type_t t)
    {
        return new BoolChain(_drv,l,r,t);
    }

    void type(type_t t){
        _ctype = t;
    }

    type_t type()
    {
        return _ctype;
    }

    void invert(bool i){
        _invert = i;
    }

    bool invert(){
        return _invert;
    }

    void addCond(preEvalFunc_t func, type_t t = EXP)
    {
        _evals.push_back(std::make_pair(func, t));
    }


    std::string process();

    void clear()
    {
        while (_subChain.size())
        {
            _subChain.front()->clear();
            delete _subChain.front();
            _subChain.pop();
        }

        _evals.clear();
        _ctype = EXP;
    }

    bool empty()
    {
        return (_evals.size() == 0);
    }

    // ======================== Helper Functions =============================

    static std::string always_true(DriverImpl &drv);

    static std::string always_false(DriverImpl &drv);

    static preEvalFunc_t if_jumps(const char *cond, const std::string &left, const std::string &right);

    static preEvalFunc_t if_less(const std::string &left, const std::string &right)
    {
        return if_jumps("<", left, right);
    }

    static preEvalFunc_t if_leq(const std::string &left, const std::string &right)
    {
        return if_jumps("<=", left, right);
    }

    static preEvalFunc_t if_greater(const std::string &left, const std::string &right)
    {
        return if_jumps(">", left, right);
    }

    static preEvalFunc_t if_geq(const std::string &left, const std::string &right)
    {
        return if_jumps(">=", left, right);
    }

    static preEvalFunc_t if_equal(const std::string &left, const std::string &right)
    {
        return if_jumps("=", left, right);
    }

    static preEvalFunc_t if_notequ(const std::string &left, const std::string &right)
    {
        return if_jumps("<>", left, right);
    }
};

} // namespace Compiler