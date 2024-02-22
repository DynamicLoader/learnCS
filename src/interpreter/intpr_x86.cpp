

#if defined(ENABLE_INTPR) & ENABLE_INTPR

#include "interpreter.hpp"
#include "symbols.hpp"

#ifdef __x86_64

namespace Compiler
{

template <typename T>
static bool callsWhenTypeMatchs(std::function<void(T *)> func, T *ptr, simple_types expected, simple_types actual)
{
    if (expected != actual)
        return false;
    func(ptr);
    return true;
};

bool Interpreter::_processFunc(asmjit::x86::Compiler &cc, const callable_t &func, const idmap_t &id,
                               stmts_t::const_iterator stbegin, stmts_t::const_iterator stend)
{
    using namespace asmjit;
    // First create the func and push into func list
    func_t f = {.name = func.name};

    FuncSignatureBuilder *fsb = new FuncSignatureBuilder;

    // arg types
    for (auto arg : func.args_attr)
    {
        bool ret = false;
        ret |= callsWhenTypeMatchs<FuncSignatureBuilder>(&FuncSignatureBuilder::addArgT<Type::Bool>, fsb,
                                                         simple_types::type_bool, arg.type);
        ret |= callsWhenTypeMatchs<FuncSignatureBuilder>(&FuncSignatureBuilder::addArgT<Type::Int32>, fsb,
                                                         simple_types::type_int, arg.type);
        ret |= callsWhenTypeMatchs<FuncSignatureBuilder>(&FuncSignatureBuilder::addArgT<Type::Int64>, fsb,
                                                         simple_types::type_real, arg.type);
        ret |= callsWhenTypeMatchs<FuncSignatureBuilder>(&FuncSignatureBuilder::addArgT<Type::UIntPtr>, fsb,
                                                         simple_types::type_char, arg.type);
        // ret |= callsWhenTypeMatchs<FuncSignatureBuilder>(&FuncSignatureBuilder::addArgT<void *>, &fsb,
        //  simple_types::type_pointer, arg.type);
        if (!ret)
            throw std::invalid_argument("Unexpected type for argument");
    }
    // retval types
    {
        bool ret = false;
        ret |= callsWhenTypeMatchs<FuncSignatureBuilder>(&FuncSignatureBuilder::setRetT<void>, fsb,
                                                         simple_types::type_void, func.retval_type);
        ret |= callsWhenTypeMatchs<FuncSignatureBuilder>(&FuncSignatureBuilder::setRetT<Type::Bool>, fsb,
                                                         simple_types::type_bool, func.retval_type);
        ret |= callsWhenTypeMatchs<FuncSignatureBuilder>(&FuncSignatureBuilder::setRetT<Type::Int32>, fsb,
                                                         simple_types::type_int, func.retval_type);
        ret |= callsWhenTypeMatchs<FuncSignatureBuilder>(&FuncSignatureBuilder::setRetT<Type::Int64>, fsb,
                                                         simple_types::type_real, func.retval_type);
        // ret |= callsWhenTypeMatchs<FuncSignatureBuilder>(&FuncSignatureBuilder::setRetT<void *>, &fsb,
        //  simple_types::type_pointer, func.retval_type);
        if (!ret)
            throw std::invalid_argument("Unexpected type for retval");
    }

    auto fn = cc.addFunc(*fsb);

    if (!fn)
        throw std::runtime_error("Cannot create function");
    f.fnl = fn->label();
    f.fs = fsb;
    funclist.push_back(f);

    // External func
    if (func.type == callable_t::EXT)
    {
        auto ff = extfunc.find(func.name);
        if (ff == extfunc.end())
            throw std::invalid_argument("Cannot link to external func");
        cc.jmp(ff->second);
        cc.ret();
        cc.endFunc();
        return true;
    }

    // Register all local variables as virtual registers (all non-constant)
    std::map<std::string, var_t> regs;
    for (auto ds : id)
    {
        var_t v;
        switch (ds.second.type)
        {
        case Compiler::simple_types::type_bool:
            v.type = TypeId::kUInt8;
            v.gp = cc.newGp(v.type);
            break;
        case Compiler::simple_types::type_int:
            v.type = TypeId::kInt32;
            v.gp = cc.newGp(v.type);
            break;
        case Compiler::simple_types::type_real:
            v.type = TypeId::kInt64;
            v.gp = cc.newGp(v.type);
            break;
        case Compiler::simple_types::type_char:
            v.type = TypeId::kUIntPtr;
            v.gp = cc.newGp(v.type);
            break;
        // case Compiler::simple_types::type_pointer:
        //     gp = cc.newUIntPtr();
        //     break;
        default:
            throw std::runtime_error("Currently not support this type!");
        }
        regs.insert(std::make_pair(ds.first, v));
    }

    // Register all labels
    std::map<std::string, Label> labels;
    for (auto st = stbegin; st != stend; ++st)
    {
        auto &op = st->op;
        if (op == "LABEL")
        {
            Label l = cc.newLabel();
            labels.insert(std::make_pair(st->ret, l));
        }
    }

    // After that, we could generate code for each stmt
    auto filterCP = [](stmts_t::value_type stmt) -> bool { // a filter that filters the comment, prog note
        return (stmt.op != "COMMENT" && (stmt.op.find("PROC") == std::string::npos));
    };

    auto isVarNumber = [](const std::string &a) { return (a.at(0) >= '0' && a.at(0) <= '9'); };
    auto isVarBool = [](const std::string &a) -> bool { return (a == "True" || a == "False"); };
    auto isVarChar = [](const std::string &a) { return (a.at(0) == '\''); };

    int argidx = 0;
    std::stack<std::string> argstack;

    auto codeGen = [&](stmts_t::value_type st) {
        // local helper functions
        struct findVarRet_t
        {
            bool found = false;
            var_t v{};
        };

        auto findVar = [&](const std::string &var) -> findVarRet_t {
            findVarRet_t ret;
            auto f = regs.find(var);
            if (f != regs.end())
            {
                ret.found = true;
                ret.v = f->second;
                return ret;
            }
            f = globals.find(var);
            if (f != globals.end())
            {
                ret.found = true;
                ret.v = f->second;
                return ret;
            }
            return ret;
        };

        struct findFuncRet_t
        {
            bool found = false;
            func_t f;
        };

        auto findFunc = [&](const std::string &func) {
            findFuncRet_t ret;
            for (auto x : funclist)
            {
                if (x.name == func)
                {
                    ret.found = true;
                    ret.f = x;
                    break;
                }
            }
            return ret;
        };

        auto cvtConst = [&](const std::string &id, findVarRet_t &v) {
            if (v.found)
                return;
            if (isVarNumber(id))
            {
                v.v.mem = cc.newInt64Const(ConstPoolScope::kLocal, std::stoll(id));
                v.v.type = TypeId::kInt64;
                v.v.obj = 1;
                v.found = true;
                return;
            }
            // as char
            if (isVarChar(id))
            {
                std::vector<char> tmp(id.begin() + 1, id.end() - 1);
                tmp.push_back(0);
                while (tmp.size() & 0b1111)
                    tmp.push_back(0);
                v.v.mem = cc.newConst(ConstPoolScope::kLocal, tmp.data(), tmp.size());
                v.v.type = TypeId::kUIntPtr;
                v.v.obj = 1;
                v.found = true;
            }
        };

        // returns the lower cast of gp
        auto cvtRegLower = [&](x86::Gp &a, x86::Gp &b) {
            auto s = std::min(a.size(), b.size());
            x86::Gp &ret = (s == a.size() ? b : a);
            switch (s)
            {
            case 1:
                ret = ret.r8();
                break;
            case 2:
                ret = ret.r16();
                break;
            case 4:
                ret = ret.r32();
                break;
            case 8:
                ret = ret.r64();
                break;
            default:
                throw std::invalid_argument("Cannot cast-down reg");
            }
        };

        auto cvtRegSize = [&](x86::Gp &ret, uint8_t size) {
            if (ret.size() > size)
            {
                switch (size)
                {
                case 1:
                    ret = ret.r8();
                    break;
                case 2:
                    ret = ret.r16();
                    break;
                case 4:
                    ret = ret.r32();
                    break;
                case 8:
                    ret = ret.r64();
                    break;
                default:
                    throw std::invalid_argument("Cannot cast-down reg");
                }
            }
            std::cout << ret.size() << ' ' << size << std::endl;
        };

        auto maxType = [](TypeId a, TypeId b) { return (TypeUtils::sizeOf(a) > TypeUtils::sizeOf(b) ? a : b); };

        // process temp regs in the ret field as dst
        auto procTempReg = [&]() {
            if (st.ret.length() == 0 || st.ret[0] != '$') // no temp reg as dst
                return;
            if (std::string("+ - * /").find(st.op) != std::string::npos) // double oprand
            {
                var_t ret;
                auto fa = findVar(st.a1), fb = findVar(st.a2);
                if (isVarNumber(st.a1))
                {
                    fa.v.type = TypeId::kInt64;
                    fa.found = true;
                }
                if (isVarNumber(st.a2))
                {
                    fb.v.type = TypeId::kInt64;
                    fb.found = true;
                }

                if (!fa.found || !fb.found)
                    throw std::invalid_argument("Cannot find arg");
                // auto rtype = maxType(fa.v.type, fb.v.type);
                auto rtype = TypeId::kInt64;

                ret.gp = cc.newGp(rtype);
                ret.type = rtype;
                regs.insert_or_assign(st.ret, ret);
                return;
            }

            if (st.op == ":=")
            {
                var_t v;
                auto fa = findVar(st.a1);
                v.type = fa.v.type;
                if (isVarNumber(st.a1))
                {
                    v.type = TypeId::kInt64;
                    fa.found = true;
                }
                if (isVarBool(st.a1))
                {
                    v.type = TypeId::kUInt8;
                    fa.found = true;
                }
                if (!fa.found)
                    throw std::invalid_argument("Cannot find arg");

                v.gp = cc.newGp(v.type);
                regs.insert_or_assign(st.ret, v);
                return;
            }

            if (st.op == "CALL")
            {
                var_t v;
                auto ff = findFunc(st.a1);
                if (!ff.found)
                    throw std::invalid_argument("Cannot find func");
                v.type = TypeId::kInt64;
                v.gp = cc.newGp(v.type);
                regs.insert_or_assign(st.ret, v);
                return;
            }

            return;
        };

        auto movGpGp = [&](x86::Gp d, x86::Gp s) {
            if (d.size() != s.size())
                cc.movsxd(d, s);
            else
                cc.mov(d, s);
        };

        auto movGpMem = [&](x86::Gp d, x86::Mem s) {
            if (d.size() != s.size())
                cc.movsxd(d, s);
            else
                cc.mov(d, s);
        };

        // Pop Args
        if (st.op == "POPARG")
        {
            auto fa = findVar(st.ret); // must be found
            fn->setArg(argidx++, fa.v.gp);
            return;
        }

        if (st.op == "PUSHARG")
        {
            argstack.push(st.ret);
            return;
        }

        // below could use temp reg in .ret field, so w3 process them first
        procTempReg();

        if (st.op == "CALL")
        {
            auto ff = findFunc(st.a1);
            if (!ff.found)
                throw std::invalid_argument("Cannot find func to call");
            InvokeNode *in;
            std::vector<x86::Gp> gp;
            while (!argstack.empty())
            {
                auto arg = argstack.top();
                auto fa = findVar(arg);
                cvtConst(arg, fa);
                if (!fa.found)
                    throw std::invalid_argument("Cannot find arg to pass");
                if (fa.v.obj & 1)
                {
                    auto gpa = cc.newGp(fa.v.type);
                    if (fa.v.type == TypeId::kUIntPtr)
                    {
                        cc.lea(gpa, fa.v.mem);
                    }
                    else
                    {
                        cc.mov(gpa, fa.v.mem);
                    }
                    gp.push_back(gpa);
                }
                else
                {
                    gp.push_back(fa.v.gp);
                }
                argstack.pop();
            }
            auto err = cc.invoke(&in, ff.f.fnl, *ff.f.fs);
            if (err)
                throw(err);
            for (int i = 0; i < gp.size(); ++i)
                in->setArg(i, gp[i]);

            if (st.ret != "")
            {
                auto fr = findVar(st.ret);
                if (!fr.found)
                    throw std::invalid_argument("Cannot find var to get return");
                if (fr.v.obj & 1)
                {
                    auto gpa = cc.newGp(fr.v.type);
                    in->setRet(0, gpa);
                    cc.mov(fr.v.mem, gpa);
                }
                else
                {
                    in->setRet(0, fr.v.gp);
                }
            }
            return;
        }

        // Labels
        if (st.op == "LABEL")
        {
            auto l = labels.find(st.ret); // must be found
            cc.bind(l->second);
            return;
        }

        if (st.op == "RETURN")
        {
            if (st.ret != "")
            {
                auto fr = findVar(st.ret);
                cvtConst(st.ret, fr);

                if (!fr.found)
                    throw std::runtime_error("Cannot find retval");

                if (fr.v.obj == 1)
                {
                    if (TypeUtils::sizeOf(fr.v.type) > 4)
                    {
                        cc.mov(x86::rax, fr.v.mem);
                    }
                    else
                    {
                        cc.mov(x86::eax, fr.v.mem); // just use rax
                    }
                }
                else
                {
                    if (TypeUtils::sizeOf(fr.v.type) > 4)
                    {
                        cc.mov(x86::rax, fr.v.gp);
                    }
                    else
                    {
                        cc.mov(x86::eax, fr.v.gp);
                    }
                }
            }
            cc.ret();
            return;
        }

        // jmps
        auto cmpVars = [&](const std::string &a, const std::string &b) {
            bool isNuma = isVarNumber(a), isNumb = isVarNumber(b);
            auto fa = findVar(a), fb = findVar(b);

            if (isVarBool(b))
            {
                if (fa.found)
                {
                    int v = (b == "True" ? 1 : 0);
                    if (fa.v.obj & 1)
                    {
                        cc.cmp(fa.v.mem, v);
                    }
                    else
                    {
                        cc.cmp(fa.v.gp, v);
                    }
                }
                else
                {
                    throw std::invalid_argument("Cannot find var");
                }
                return;
            }

            cvtConst(st.a1, fa);
            cvtConst(st.a2, fb);

            if (!fa.found || !fb.found)
                throw std::runtime_error("Cannot find var");
            if (fa.v.obj == 0 && fb.v.obj == 0)
            {
                auto ca = fa.v.gp, cb = fb.v.gp;
                // cvtRegLower(ca, cb);
                if (ca.size() < cb.size())
                {
                    cc.movsxd(ca.r64(), ca);
                    ca = ca.r64();
                }
                if (ca.size() > cb.size())
                {
                    cc.movsxd(cb.r64(), cb);
                    cb = cb.r64();
                }
                cc.cmp(ca, cb);
                return;
            }
            if (fa.v.obj == 0 && fb.v.obj == 1)
            {
                auto ca = fa.v.gp;
                // cvtRegSize(ca, fb.v.mem.size());
                cc.cmp(ca, fb.v.mem);
                return;
            }
            if (fa.v.obj == 1 && fb.v.obj == 0)
            {
                auto cb = fb.v.gp;
                // cvtRegSize(cb, fa.v.mem.size());
                cc.cmp(fa.v.mem, cb);
                return;
            }
            if (fa.v.obj == 1 && fb.v.obj == 1)
            {
                x86::Gp gpa = cc.newGp(fa.v.type);
                // cc.movsxd(gpa, fa.v.mem);
                movGpMem(gpa, fa.v.mem);
                cc.cmp(gpa, fb.v.mem);
                return;
            }
            throw std::invalid_argument("Cannot generate cmp for args");
        };

        auto condJmp = [&]() {
            if (st.op[0] != 'j')
                return false;
            auto l = labels.find(st.ret); // must be found
            cmpVars(st.a1, st.a2);
            if (st.op == "j<")
                return cc.jl(l->second), true;
            if (st.op == "j>")
                return cc.jg(l->second), true;
            if (st.op == "j<=")
                return cc.jle(l->second), true;
            if (st.op == "j>=")
                return cc.jge(l->second), true;
            if (st.op == "j=")
                return cc.je(l->second), true;
            if (st.op == "j<>")
                return cc.jne(l->second), true;
            return false;
        };

        if (st.op == "j")
        {                                 // unconditional jmp
            auto l = labels.find(st.ret); // must be found
            cc.jmp(l->second);
            return;
        }

        if (condJmp()) // conditional jmp
            return;

        // Move
        if (st.op == ":=")
        {
            cc.comment("Move");
            auto ret = findVar(st.ret);
            auto a = findVar(st.a1);
            // bool isNuma = isVarNumber(st.a1);
            bool isBoola = isVarBool(st.a1);

            if (isBoola) // Bool
            {
                if (!ret.found)
                    throw std::invalid_argument("Error when mov");
                int imm = (st.a1 == "True" ? 1 : 0);
                if (ret.v.obj & 1)
                {
                    cc.mov(ret.v.mem, imm);
                }
                else
                {
                    cc.mov(ret.v.gp, imm);
                }
                return;
            }

            cvtConst(st.a1, a);
            if (!a.found || !ret.found)
                throw std::invalid_argument("Error when mov");
            Error err = 0;
            x86::Gp gpr;
            if (ret.v.obj & 1) // moving to mem
            {
                // cc.comment("MoveM");
                gpr = cc.newGp(ret.v.type);
            }
            else
            {
                // cc.comment("MoveR");
                gpr = ret.v.gp;
            }

            if (a.v.obj & 1)
                movGpMem(gpr, a.v.mem);
            else
                movGpGp(gpr, a.v.gp);

            if (ret.v.obj & 1)
            {
                // cc.comment("MoveB");
                err = cc.mov(ret.v.mem, gpr);
            }

            if (err)
                throw std::runtime_error("Cannot mov");
            return;
        }

        // A&L ops
        auto procAL = [&]() -> bool {
            auto fa = findVar(st.a1), fb = findVar(st.a2), fr = findVar(st.ret);
            cvtConst(st.a1, fa);
            cvtConst(st.a2, fb);
            if (!fa.found || !fb.found || !fr.found)
                throw std::invalid_argument("Cannot find op in AL");
            if (st.op == "+" || st.op == "-") // +-
            {
                auto rtype = maxType(maxType(fa.v.type, fb.v.type), fr.v.type);
                bool sub = (st.op == "-");

                auto doAddSub = [&](x86::Gp &gpa, bool sub = false) {
                    auto gpb = cc.newGp(rtype);
                    if (!(fa.v.obj & 1) && !(fb.v.obj & 1)) // all in regs
                    {
                        // cc.movsxd(gpa, fa.v.gp);
                        movGpGp(gpa, fa.v.gp);
                        // cc.movsxd(gpb, fb.v.gp);
                        movGpGp(gpb, fb.v.gp);
                        // cc.commentf("A0");
                        if (!sub)
                            cc.add(gpa, gpb);
                        else
                            cc.sub(gpa, gpb);
                    }
                    if (!(fa.v.obj & 1) && (fb.v.obj & 1))
                    {
                        // cc.movsxd(gpa, fa.v.gp);
                        movGpGp(gpa, fa.v.gp);
                        // cc.commentf("A1");
                        if (!sub)
                            cc.add(gpa, fb.v.mem);
                        else
                            cc.sub(gpa, fb.v.mem);
                    }
                    if ((fa.v.obj & 1) && !(fb.v.obj & 1))
                    {
                        // cc.movsxd(gpa, fa.v.mem);
                        // cc.movsxd(gpb, fb.v.gp);
                        movGpMem(gpa, fa.v.mem);
                        movGpGp(gpb, fb.v.gp);
                        // cc.commentf("A2");
                        if (!sub)
                            cc.add(gpa, gpb);
                        else
                            cc.sub(gpa, gpb);
                    }
                    if ((fa.v.obj & 1) && (fb.v.obj & 1)) // all mem
                    {
                        // cc.movsxd(gpa, fa.v.mem);
                        movGpMem(gpa, fa.v.mem);
                        // cc.commentf("A3");
                        if (!sub)
                            cc.add(gpa, fb.v.mem);
                        else
                            cc.sub(gpa, fb.v.mem);
                    }
                };

                if (fr.v.obj & 1) // ret in mem
                {
                    auto gpa = cc.newGp(rtype);
                    doAddSub(gpa, sub);
                    // auto gpr = cc.newGp(fr.v.type);
                    // cc.movsxd(gpr, gpa);
                    cvtRegSize(gpa, fr.v.mem.size());
                    cc.mov(fr.v.mem, gpa);
                }
                else
                {
                    doAddSub(fr.v.gp, sub);
                }
            }

            if (st.op == "*")
            {
                auto rtype = fr.v.type;
                auto doMul = [&](x86::Gp &gpa) {
                    auto gpb = cc.newGp(rtype);
                    if (!(fa.v.obj & 1) && !(fb.v.obj & 1)) // all in regs
                    {
                        movGpGp(gpa, fa.v.gp);
                        movGpGp(gpb, fb.v.gp);
                        cc.imul(gpa, gpb);
                    }
                    if (!(fa.v.obj & 1) && (fb.v.obj & 1))
                    {
                        movGpGp(gpa, fa.v.gp);
                        cc.imul(gpa, fb.v.mem);
                    }
                    if ((fa.v.obj & 1) && !(fb.v.obj & 1))
                    {
                        movGpMem(gpa, fa.v.mem);
                        movGpGp(gpb, fb.v.gp);
                        cc.imul(gpa, gpb);
                    }
                    if ((fa.v.obj & 1) && (fb.v.obj & 1)) // all mem
                    {
                        movGpMem(gpa, fa.v.mem);
                        cc.imul(gpa, fb.v.mem);
                    }
                };

                if (fr.v.obj & 1) // ret in mem
                {
                    auto gpa = cc.newGp(rtype);
                    doMul(gpa);
                    // auto gpr = cc.newGp(fr.v.type);
                    // cc.movsxd(gpr, gpa);
                    // cvtRegSize(gpa, fr.v.mem.size());
                    cc.mov(fr.v.mem, gpa);
                }
                else
                {
                    doMul(fr.v.gp);
                }
            }

            if (st.op == "/" || st.op == "%")
            {
                auto rtype = fr.v.type;
                auto doDiv = [&](x86::Gp &gpq, x86::Gp &gpr) {
                    cc.mov(gpr, 0);
                    if (!(fa.v.obj & 1) && !(fb.v.obj & 1)) // all in regs
                    {
                        movGpGp(gpq, fa.v.gp);
                        cc.idiv(gpr, gpq, fb.v.gp);
                    }
                    if (!(fa.v.obj & 1) && (fb.v.obj & 1))
                    {
                        movGpGp(gpq, fa.v.gp);
                        cc.idiv(gpr, gpq, fb.v.mem);
                    }
                    if ((fa.v.obj & 1) && !(fb.v.obj & 1))
                    {
                        movGpMem(gpq, fa.v.mem);
                        cc.idiv(gpr, gpq, fb.v.gp);
                    }
                    if ((fa.v.obj & 1) && (fb.v.obj & 1)) // all mem
                    {
                        movGpMem(gpq, fa.v.mem);
                        cc.idiv(gpr, gpq, fb.v.mem);
                    }
                };
                auto gpq = cc.newGpq(), gpr = cc.newGpq();
                doDiv(gpq, gpr);
                auto ret = (st.op == "%" ? gpr : gpq);
                if (fr.v.obj & 1) // ret in mem
                {
                    cvtRegSize(ret, fr.v.mem.size());
                    cc.mov(fr.v.mem, ret);
                }
                else
                {
                    cvtRegSize(ret, fr.v.gp.size());
                    movGpGp(fr.v.gp, ret);
                }
            }

            return false;
        };

        if (std::string("+-*/%").find(st.op) != std::string::npos && procAL())
            return;

        // @todo : Others
    }; // codeGen

    for (auto st = stbegin; st != stend; ++st)
    {
        if (!filterCP(*st))
            continue; // filtered
        codeGen(*st);
    }

    cc.endFunc();
    return true;
}

int Interpreter::proc(const stmts_t &s, const CompiledInfo_t &cinfo)
{
    using namespace asmjit;

    x86::Compiler cc(&ch);
    // cc.addDiagnosticOptions(DiagnosticOptions::kRADebugAll);

    // alloc all global variables first
    auto gid = cinfo.ids.at(-1); // must be found
    Section *ds;
    auto err = ch.newSection(&ds, ".data", SIZE_MAX, SectionFlags::kNone, 8);
    err |= cc.section(ds);
    if (err)
        throw std::runtime_error("Cannot create .data section or switch to .data");

    for (auto g : gid)
    {
        var_t gl;
        bool constant = (g.second.attr & 1);
        switch (g.second.type)
        {
        case type_bool:
            gl.type = TypeId::kUInt8;
            if (constant)
            {
                gl.mem = cc.newByteConst(ConstPoolScope::kGlobal, (g.second.initval == "True"));
            }
            else
            {
                auto data = cc.newLabel();
                cc.bind(data);
                cc.db(0);
                gl.mem = x86::ptr(data, 0, 1); // size?
            }
            break;
        case type_int:
            gl.type = TypeId::kInt32;
            if (constant)
            {
                gl.mem = cc.newInt32Const(ConstPoolScope::kGlobal, std::stoi(g.second.initval));
            }
            else
            {
                auto data = cc.newLabel();
                cc.bind(data);
                cc.dd(0);
                gl.mem = x86::ptr(data, 0, 4);
            }
            break;
        case type_real:
            gl.type = TypeId::kInt64;
            if (constant)
            {
                gl.mem = cc.newInt64Const(ConstPoolScope::kGlobal, std::stoll(g.second.initval));
            }
            else
            {
                auto data = cc.newLabel();
                cc.bind(data);
                cc.dq(0);
                gl.mem = x86::ptr(data, 0, 8);
            }
            break;
        case type_char:
            if (constant)
            {
                gl.type = TypeId::kUIntPtr;
                std::vector<char> tmp(g.second.initval.begin() + 1, g.second.initval.end() - 1);
                tmp.push_back(0);
                while (tmp.size() & 0b1111)
                    tmp.push_back(0);
                gl.mem = cc.newConst(ConstPoolScope::kGlobal, tmp.data(), tmp.size());
            }
            else
            {
                throw std::runtime_error("char var should be constant at global");
            }
            break;
        default:
            throw std::runtime_error("Currently not support this type");
        }
        gl.obj = 1; // in mem
        globals.insert(std::make_pair(g.first, gl));
    }

    // switch back to .text section
    cc.section(ch.textSection());
    // Process subprocs
    auto i = s.begin();
    auto j = i;
    while (j != s.end() && i != s.end())
    {
        if (i->op == "PROCBGN")
        {
            j = i;
            while (j != s.end() && j->op != "PROCEND")
                ++j;
            if (j == s.end())
                throw std::runtime_error("Incomplete program");

            // We have got a program block, get function info now
            auto func = std::find_if(cinfo.callable_symbols.begin(), cinfo.callable_symbols.end(),
                                     [i](callable_t cb) -> bool { return i->ret == cb.name; });

            if (func != cinfo.callable_symbols.end())
            {
                auto ididx = func - cinfo.callable_symbols.begin();
                _processFunc(cc, *func, cinfo.ids.at(ididx), i, j);
            }
            i = j;
        }
        else
        {
            i++;
        }
    }

    cc.finalize();
    return 0;
}

int Interpreter::run(const std::string &entry)
{
    using namespace asmjit;
    auto fe = std::find_if(funclist.begin(), funclist.end(), [entry](auto x) -> bool { return x.name == entry; });
    if (fe == funclist.end())
        throw std::invalid_argument("Bad entry");

    using func_t = int (*)(void);
    void *basePtr;
    Error err = rt.add(&basePtr, &ch);

    if (err)
        throw err;
    auto off = ch.labelOffset(fe->fnl);
    func_t fac = (func_t)((intptr_t)basePtr + off);
    auto ret = fac();

    rt.release(basePtr);
    // std::cerr << "The program returned:" << ret << std::endl;
    return ret;
}

} // namespace Compiler

#endif

#endif