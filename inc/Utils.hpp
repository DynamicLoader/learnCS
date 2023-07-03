/**
 * @file Utils.hpp
 * @brief Privide utils for HRCore
 * @version 0.1
 * @date 2022-12-10
 *
 */

#ifndef __HRCORE_UTILS_HPP__
#define __HRCORE_UTILS_HPP__

#include "../HRCore.h"

namespace HRCore {

/**
 * @brief A namespace that contains internally used functions
 */
namespace Internal {
    /// @brief Prepare next item of Storage to be writen
    bool _prepareNextStorageItem(Storage::Interface* i, Storage::Interface::item_t& r)
    {
        if (!i)
            return false;
        auto t = i->next(r);
        if (t) {
            r = t;
            return true;
        } else {
            bool ret = i->extend(i->length() << 1);
            r = i->next(r);
            return ret;
        }
    }
}

/**
 * @brief This section described I/O functions of HRCore.
 * @defgroup io_group I/O Functions
 *
 */
namespace Value {
    /**
     * @brief Reload of operator<< to support std::cout and other std::ostream with Integer
     * @ingroup io_group
     *
     * Example:
     * @code {.cpp}
     * auto a = Integer::make<LinkedList<8>>();
     * cout << a;
     * @endcode
     *
     *@note When std::hex flag is not set, Dec number is output by default.
     *
     */
    std::ostream& operator<<(std::ostream& os, Integer& rhs)
    {
        if (os.flags() & std::ios_base::hex) { // Hex
            auto t = rhs._idata->end();
            os << (rhs._isPostive ? "" : "-") << (os.flags() & std::ios_base::showbase ? "0x" : "");
            while (t) {
                auto m = rhs._idata->prev(t);
                os << *t.data << (m ? "," : "");
                t = m;
            }
        } else { // Dec
            Integer div = rhs.make() = 1000;
            Integer src = rhs;
            auto ret = rhs._idata->newObject();
            ret->extend(rhs._idata->length() * 3);
            os << (rhs._isPostive ? "" : "-");
            auto r = ret->begin();
            while (Integer::_absGreater(src, div)) {
                auto tmp = src.divideBy(div);
                *r.data = *(tmp.getRemainder()._idata->begin().data);
                Internal::_prepareNextStorageItem(ret, r);
                src = tmp.getQuotient();
            }
            *r.data = *src._idata->begin().data;
            for (bool flag = false; r; flag = true) {
                auto t = ret->prev(r);
                if (flag) { // Not highest bit; add 0 to get 3 digits
                    if (*r.data < 10) {
                        os << "00";
                    } else if (*r.data < 100) {
                        os << "0";
                    }
                }
                if (*r.data == 1000) {
#if HRCORE_ENABLE_IO_DELIMETER
                    os << "1,000";
#else
                    os << "1000";
#endif
                } else {
                    os << *r.data;
                }
#if HRCORE_ENABLE_IO_DELIMETER
                os << (t ? "," : "");
#endif
                r = t;
            }
        }

        return os;
    }

    /**
     * @brief Reload of operator>> to support std::cin and other std::istream with Integer
     * @ingroup io_group
     *
     * Example:
     * @code {.cpp}
     * auto a = Integer::make<LinkedList<8>>();
     * cin >> a;
     * @endcode
     *
     */
    std::istream& operator>>(std::istream& is, Integer& rhs)
    {
        int c = EOF;
        while ((c = is.peek()) == ' ' || c == '\n' || c == '\r') // Remove prefix invalid characters
            is.ignore();
        if (c == EOF) {
            rhs = 0;
            return is;
        }
        if (is.flags() & std::ios_base::hex) { // Hex
            ival_t data = 0;
            auto ret = rhs._idata->newObject();
            auto r = ret->begin();
            while ((c = is.peek()) != EOF) {
                if (c == ',' || c == ' ' || c == '\n') { // Item delimeter
                    is.ignore();
                    *r.data = data;
                    Internal::_prepareNextStorageItem(ret, r);
                    if (c == ',')
                        continue;
                    r = ret->prev(r);
                    break;
                }
                is >> data;
            }
            if (c == EOF) // Write back remain data
                *r.data = data;
            if (ret->length() > rhs._idata->length()) {
                rhs._idata->extend(ret->length());
            } else {
                rhs._idata->shrink(ret->length());
            }
            for (auto x = rhs._idata->begin(); x && r; x = rhs._idata->next(x), r = ret->prev(r)) // Reverse copy
                *x.data = *r.data;
            ret->delObject(ret); // Don't forget to release memory
            rhs._simplify();
        } else { // Decimal
            Integer tim = rhs.make() = 10;
            rhs = 0; // Clear original value
            bool positive = (c != '-');
            if (c == '-' || c == '+')
                is.ignore();
            while ((c = is.get()) != EOF) {
                HRCORE_DBG(std::cout << c - '0' << std::endl;)
                if (c >= '0' && c <= '9') {
                    Integer tmp = rhs.make() = c - '0';
                    rhs = rhs * tim + tmp;
                } else if (c == ',') {
                    continue;
                } else {
                    is.putback(c);
                    break;
                }
            }
            rhs.positive(positive);
        }
        return is;
    }

#if HRCORE_ENABLE_FP

    /**
     * @brief Reload of operator>> to support std::cin and other std::istream with Float
     * @ingroup io_group
     * @note Only decimal mode is supported.
     */
    std::istream& operator>>(std::istream& is, Float& rhs)
    {
        if (is.flags() & std::ios_base::hex)
            return is;
        operator>>(is, (Integer&)rhs);
        bool flag = rhs._isPostive;
        rhs._isPostive = true;
        int c = is.peek();
        Integer div = rhs.make() = 1;
        Float dim = rhs.make();
        int dtmp = -HRCORE_FP_BITS_BASE;
        size_t tpos = 0;
        Float tim = rhs.make() = 10;
        if (c == '.') {
            is.ignore();
            while ((c = is.get()) != EOF) {
                if (c >= '0' && c <= '9') {
                    Integer tmp = rhs.make() = c - '0';
                    dim = dim * tim + tmp;
                    div = div * tim;
                    dtmp -= HRCORE_FP_BITS_PER_DIGIT;
                    ++tpos;
                } else if (c == ',') {
                    continue;
                } else {
                    is.putback(c);
                    break;
                }
            }
        }
        if (tpos < rhs._precision) {
            dim = dim * tim;
            div = div * tim;
            ++tpos;
            dtmp -= HRCORE_FP_BITS_PER_DIGIT;
        }
        rhs = rhs + Float((dim << -dtmp) / div, dtmp, tpos);
        rhs._isPostive = flag;
        HRCORE_DBGI(std::cout << "[operator>> @ Float] " << rhs._fpos << std::endl;)
        return is;
    }

    /**
     * @brief Reload of operator<< to support std::cout and other std::ostream with Integer
     * @ingroup io_group
     * @note Only decimal mode is supported.
     */
    std::ostream& operator<<(std::ostream& os, Float& rhs)
    {
        if (os.flags() & std::ios_base::hex)
            return os;
        auto it = rhs.getInteger();
        if (rhs._fpos < 0) {
            // os << '.';
            auto dtmp = rhs - Float(it);
            Float tim = rhs.make() = 10;
            // Integer vd = rhs.make() = 9;
            auto ret = tim._idata->newObject();
            auto r = ret->begin();
            size_t n = 0;
            Float tmp = dtmp * tim;
            while (!Integer::_isZero(tmp)) {
                auto x = tmp.getInteger();
                *r.data = *(((Float*)&x)->_idata->begin().data);
                Internal::_prepareNextStorageItem(ret, r);
                ++n;
                HRCORE_DBG(operator<<(os, x);)
                tmp = (tmp - x) * tim; // Remove integer part and get next digit
            }
            HRCORE_DBG(os << std::endl;)
            if (n == 0) { // No digit
                operator<<(os, it);
                if (os.flags() & std::ios_base::showpoint)
                    os << ".0";
                return os;
            }
            size_t i = rhs._tpos; //(rhs._fpos + HRCORE_FP_BITS_BASE) / -HRCORE_FP_BITS_PER_DIGIT;
            if (i >= n) {
                i = n;
            } else {
                r = ret->at(i);
                if (i)
                    *ret->prev(r).data += (*r.data >= 5);
            }
            r = ret->prev(r);
            for (; i > 0; --i, r = ret->prev(r)) { // Remove last 0s
                if (*r.data != 0)
                    break;
            }
            if (i > 0 && *r.data == 10) { // Overflow! may be a number that has fixed digit
                --i;
                auto j = ret->prev(r);
                while (j) {
                    if (*j.data == 9) {
                        --i;
                        j = ret->prev(j);
                    } else {
                        ++*j.data;
                        break;
                    }
                }
            }
            Integer k = rhs.make() = 1;
            r = ret->begin();
            auto& x = *r.data;
            if (x != 0) { // Process overflow to integer
                if (x > 10)
                    throw(Exception::InternalError());
                if (x == 10) {
                    it = it + k;
                    x = 0;
                }
            }
            operator<<(os, it); // Output integer part
            if (i == 0) {
                if (os.flags() & std::ios_base::showpoint) {
                    os << ".0";
                }
            } else {
                os << '.';
                for (size_t x = 0; x < i; ++x, r = ret->next(r)) {
                    os << *r.data;
                }
            }
        } else {
            operator<<(os, it);
            if (os.flags() & std::ios_base::showpoint) {
                os << ".0";
            }
        }
        return os;
    }

#endif
}

#if HRCORE_ENABLE_UTILS
/**
 * @brief A namespace that contains utils for HRCore
 * @namespace HRCore::Utils
 */
namespace Utils {

    Value::Integer abs(const Value::Integer& s)
    {
        auto ret = s;
        ret.positive(true);
        return ret;
    }
#if HRCORE_ENABLE_FP
    Value::Float abs(const Value::Float& s)
    {
        auto ret = s;
        ret.positive(true);
        return ret;
    }
#endif
}

#if HRCORE_ENABLE_UTILS_IO

namespace Internal {
    template <typename G>
    struct _expression_isT_OK { };

    template <>
    struct _expression_isT_OK<Value::Integer> {
        using value = Value::Integer;
    };

    template <>
    struct _expression_isT_OK<Value::Float> {
        using value = Value::Float;
    };

    template <typename G>
    struct _expression_mod {
        static G mod(const G& l, const G& r)
        {
            (void)l, (void)r;
            throw(Exception::ExpressionInvalidOperation());
        }
    };

    template <>
    struct _expression_mod<Value::Integer> {
        static Value::Integer mod(Value::Integer& l, const Value::Integer& r)
        {
            return l % r;
        }
    };

    int8_t _getOp(char c)
    {
        switch (c) {
        case '+':
            return 0;
        case '-':
            return 1;
        case '*':
            return 2;
        case '/':
            return 3;
        case '%':
            return 4;
        default:
            return -1;
        }
    }

    constexpr bool _isOpPrior(int8_t left, int8_t right)
    {
        return (left >= 0 && right >= 0) && !(left < 2 && right > 1);
    }
}

namespace Utils {

    /**
     * @brief A class to get and evaluate expression
     * @ingroup io_group
     * @tparam T Data type, choose from 'Integer' and 'Float'
     * @tparam U Storage implement type
     * @note Invalid T will not be able to create a object
     */
    template <typename T, typename U>
    class Expression;

    /**
     * @brief Convert a stream into an expression
     * @details The function will reads std::istream object until EOF.
     *          If something invalid read, throw Exception::ExpressionConvertFailed.
     * @note "--" "++"" is not allowed!
     * @ingroup io_group
     */
    template <typename T, typename U>
    std::istream& operator>>(std::istream& is, Expression<T, U>& rhs);

    /**
     * @brief Output the value of an expression
     * @ingroup io_group
     */
    template <typename T, typename U>
    std::ostream& operator<<(std::ostream& os, Expression<T, U>& rhs);

    template <typename T, typename U>
    class Expression {
    private:
        using _dummy_t = typename Internal::_expression_isT_OK<T>::value; // Error here? Type of T using is not supported!
        struct node_t {
            T* data = nullptr;
            uint8_t sgn = 0xFF; // 0 - add, 1 - sub, 2 - mul, 3 - div, 4 - mod
        };
        T* _result = nullptr;
        std::deque<node_t> _data;
        Storage::Interface* _helper = nullptr;
        bool _isEval = false;
        bool _evalFailed = false;

        Storage::Interface* _newStorage()
        {
            if (this->_helper)
                return this->_helper->newObject();
            return new U;
        }

        static T _doOperation(T& left, T& right, uint8_t op)
        {
            switch (op) {
            case 0:
                return left + right;
            case 1:
                return left - right;
            case 2:
                return left * right;
            case 3:
                return left / right;
            case 4: {
                return Internal::_expression_mod<T>::mod(left, right);
            }
            default:
                throw(Exception::ExpressionInvalidOperation());
            }
        }

    public:
        Expression(U* helper = nullptr)
        {
            this->_helper = dynamic_cast<Storage::Interface*>(helper); // Error here? U is invalid!
        }
        ~Expression()
        {
            for (auto x = this->_data.begin(); x != this->_data.end(); ++x) {
                if ((*x).data)
                    delete (*x).data;
            }
        }

        /**
         * @brief Evaluate the expression
         * 
         * @return true 
         * @return false 
         */
        bool eval()
        {
            this->_isEval = true;
            this->_evalFailed = true;
            std::stack<T*> tmp;
            for (auto x = this->_data.begin(); x != this->_data.end(); ++x) {
                if ((*x).data) {
                    tmp.push((*x).data);
                    continue;
                }
                uint8_t op = (*x).sgn;
                if (!tmp.size())
                    return false;
                T* right = tmp.top();
                tmp.pop();
                if (!tmp.size())
                    return false;
                T* left = tmp.top();
                HRCORE_DBG(std::cout << *left << ' ' << int(op) << ' ' << *right << std::endl;)
                *left = _doOperation(*left, *right, op);
            }
            if (tmp.size() == 1) {
                if(this->_result)
                    delete this->_result;
                this->_result = tmp.top();
                this->_evalFailed = false;
                return true;
            }
            return false;
        }

        /**
         * @brief Get the Result object. Make sure eval() returned true first, otherwise Segment Fault!
         */
        T getResult(){
            return *this->_result;
        }

        friend std::istream& operator>><>(std::istream& is, Expression<T, U>& rhs);
        friend std::ostream& operator<< <>(std::ostream& os, Expression<T, U>& rhs);
    };

    template <typename T, typename U>
    std::istream& operator>>(std::istream& is, Expression<T, U>& rhs)
    {
        int c = EOF;
        int8_t op = -8; // Initialized
        int num = 0;
        // int numOp = 0;
        bool nextNeg = false;
        std::stack<int8_t> tmp;
        while ((c = is.peek()) != EOF) {
            typename Expression<T, U>::node_t ret;
            while (c == ' ' || c == '\n' || c == '\r') {
                is.ignore();
                c = is.peek();
            }
            c = is.get();
            HRCORE_DBG(std::cout << int(c) << std::endl;)
            if (c == EOF)
                break;
            if (c >= '0' && c <= '9') {
                is.putback(c);
                if (op == -1) // Continous number
                    throw Exception::ExpressionConvertFailed();
                T& in = *new T(rhs._newStorage(), true, true, false);
                operator>>(is, in);
                if (nextNeg) {
                    in.positive(false);
                    nextNeg = false;
                }
                ret.data = &in;
                rhs._data.push_back(ret);
                ++num;
                op = -1;
                continue;
            }
            // Process '(' and ')'
            if (c == '(') {
                tmp.push(-2); // using -2 as '('
                op = -2;
                continue;
            }
            if (c == ')') {
                while (tmp.size() && tmp.top() != -2) {
                    ret.sgn = tmp.top();
                    rhs._data.push_back(ret);
                    tmp.pop();
                }
                if (!tmp.size())
                    throw(Exception::ExpressionConvertFailed());
                tmp.pop(); // Pop '('
                op = -1;
                continue;
            }
            int8_t x = Internal::_getOp(c);
            if (x > -1) { // A operator
                if (op != -1) { // Last time is an operator
                    if (x == 1 && op != 1) {
                        nextNeg = true;
                        op = x;
                        continue;
                    } else {
                        throw(Exception::ExpressionConvertFailed());
                    }
                }
                op = x;
                while (tmp.size() && Internal::_isOpPrior(tmp.top(), x)) {
                    ret.sgn = tmp.top();
                    rhs._data.push_back(ret);
                    tmp.pop();
                }
                tmp.push(x);
                continue;
            }
            throw(Exception::ExpressionConvertFailed());
        }
        while (tmp.size()) {
            typename Expression<T, U>::node_t ret;
            ret.sgn = tmp.top();
            rhs._data.push_back(ret);
            tmp.pop();
        }
        return is;
    }

    template <typename T, typename U>
    std::ostream& operator<<(std::ostream& os, Expression<T, U>& rhs)
    {
        if (!rhs._isEval)
            rhs.eval();
        if (!rhs._evalFailed) {
            operator<<(os, *(rhs._result));
        }
        return os;
    }
} // namespace Utils
#endif

#endif

} // namespace HRCore

#endif