/**
 * @file Value.hpp
 * @brief Implements of mathematic data type and calculation
 * @version 0.1
 * @date 2022-12-06
 *
 */

#ifndef __HRCORE_VALUE_HPP__
#define __HRCORE_VALUE_HPP__

#include "../HRCore.h"

namespace HRCore {

/**
 * @brief A namespace that contains mathematic data type
 * @namespace HRCore::Value
 */
namespace Value {
    /**
     * @brief Integer type
     * @class Integer
     * @details A storage class pointer is needed to construct object. Besides, a static defined construction function is provided.
     *
     */
    class Integer {
    public:
        /**
         * @brief Type defination of division result (For Integer only).
         * @note Both quotient and remainder can be accessed only once. If not using all, call `clear()` to release memory.
         */
        class DivisionResult_t {
        public:
            /// @brief Get quotient of division result
            Integer getQuotient()
            {
                auto x = this->q_data;
                this->q_data = nullptr;
                return Integer(x, this->positive, true, true);
            }

            /// @brief Get remainder of division result
            Integer getRemainder()
            {
                auto x = this->r_data;
                this->r_data = nullptr;
                return Integer(x, this->positive, true, true);
            }

            /// @brief Clear the object and release memory allocated
            void clear()
            {
                if (this->q_data)
                    this->q_data->delObject(this->q_data);
                if (this->r_data)
                    this->r_data->delObject(this->r_data);
                return;
            }

        protected:
            Storage::Interface* q_data = nullptr;
            Storage::Interface* r_data = nullptr;
            bool positive = false;
            friend class Integer;
        };

        /**
         * @brief Construct a new Integer object
         *
         * @param ptr A pointer to a object of a class that implemented Storage::Interface
         * @param positive Decide whether the Integer object is positive or negative
         * @param autoRelease Specify if the `ptr` should be release autonomously after deconstruction
         * @param sim Decide if the data should be simplified first. (Internally)
         * @throw -1 when the ptr is invalid.
         * @par Here is an example:
         * @code {.cpp}
         *      ChainList<8> sto;
         *      Integer a(&sto);
         * @endcode
         * @note The Storage::Interface implemented object should NOT be destroyed before destruction of object of this class.
         * @exception Exception::InvalidArgument is throw when ptr is nullptr
         */
        Integer(Storage::Interface* ptr, bool positive = true, bool autoRelease = false, bool sim = true)
        {
            if (!ptr)
                // throw(Exception(1));
                throw(Exception::InvalidArgument());
            this->_idata = ptr;
            this->_idata_ar = autoRelease;
            this->_isPostive = positive;
            if (sim)
                this->_simplify();
            HRCORE_DBG(std::cout << "Interface* constructor! (_idata: " << this->_idata << ")" << std::endl;)
        }

        /**
         * @brief Construct a new Integer object using given type of Storage::Interface implement.
         *
         * @tparam T Type of Storage::Interface implement
         * @return Integer New object
         *
         * @par Here is a example:
         * @code {.cpp}
         * Integer&& it = Integer::make<Storage::LinkedList<8>>();
         * @endcode
         */
        template <typename T>
        static Integer make()
        {
            return Integer(new T, true, true, false);
        }

        /// @brief Make a new object with same Interface type
        Integer make()
        {
            return Integer(this->_idata->newObject(), true, true, false);
        }

        /**
         * @brief Move a Integer object
         *
         * @param c The Integer object to be moved
         */
        Integer(Integer&& c)
        {
            HRCORE_DBG(std::cout << "Move constructor! (_idata: " << c._idata << " -> " << this->_idata << ")" << std::endl;)
            this->~Integer();
            this->_idata = c._idata;
            this->_isPostive = c._isPostive;
            this->_idata_ar = c._idata_ar;
            c._idata = nullptr;
            c._idata_ar = false;
        }

        /**
         * @brief Move operator
         *
         * @param c The Integer object to be moved
         * @return Integer& Left-value reference of moved object
         */
        Integer& operator=(Integer&& c)
        {
            HRCORE_DBG(std::cout << "Move operator! (_idata: " << c._idata << " -> " << this->_idata << ")" << std::endl;)
            this->~Integer();
            this->_idata = c._idata;
            this->_idata_ar = c._idata_ar;
            this->_isPostive = c._isPostive;
            c._idata_ar = false; // Avoid release idata; BUG detected before!
            c._idata = nullptr;
            return *this;
        }

        /**
         * @brief Copy and construct a new Integer object.
         * @note The internal data type is decided by parameter c!
         * @param c Previous Integer object to be copied.
         */
        Integer(const Integer& c)
        {
            this->~Integer(); // Release current resource
            this->_idata = c._idata->newObject();
            this->_idata->extend(c._idata->length());
            for (auto i = c._idata->begin(), j = this->_idata->begin(); i && j; i = c._idata->next(i), j = this->_idata->next(j))
                *j.data = *i.data;
            this->_idata_ar = true;
            this->_isPostive = c._isPostive;
            HRCORE_DBG(std::cout << "Copy constructor! (_idata: " << c._idata << " -> " << this->_idata << ")" << std::endl;)
        }

        ~Integer()
        {
            if (this->_idata_ar && this->_idata != nullptr) { // Avoid delete nullptr to cause exception
                HRCORE_DBG(std::cout << "~Integer! (_idata: " << this->_idata << ")" << std::endl;)
                this->_idata->delObject(this->_idata);
            }
        }

        /**
         * @brief Reload of operator= to support a ival_t be converted to Integer
         *
         * @param num Value to be converted
         * @return Integer& Return of converted object
         */
        Integer& operator=(ival_t num)
        {
            auto tmp = this->_idata->newObject();
            tmp->extend(1);
            *tmp->at(0).data = num;
            this->~Integer();
            this->_idata = tmp;
            this->_idata_ar = true;
            this->_isPostive = true;
            return *this;
        }

        /// @brief Copy operator
        Integer& operator=(const Integer& c)
        {
            *this = Integer(c);
            return *this;
        }

        /**
         * @brief Reload of operator+
         *
         * @param r The value of right side
         * @return Integer The result of (this + r)
         */
        Integer operator+(const Integer& r)
        {
            bool neg = (this->_isPostive ^ r._isPostive); // Determind if is +-
            bool bigger = _absGreater(*this, r);
            auto tmp = this->_idata->newObject();
            size_t m = std::max(this->_idata->length(), r._idata->length());
            tmp->extend(m + 1); // Prepare buffer
            const Integer& op1 = (bigger ? *this : r);
            const Integer& op2 = (bigger ? r : *this);
            bool flag = true;
            if (neg) {
                _absSub(op1, op2, tmp);
                flag = op1._isPostive;
            } else {
                _absPlus(op1, op2, tmp);
                flag = this->_isPostive;
            }
            return Integer(tmp, flag, true, true);
        }

        /**
         * @brief Reload of operator-(r)
         *
         * @param r The value of right side
         * @return Integer The result of (this - r)
         */
        Integer operator-(const Integer& r)
        {
            return this->operator+(-r);
        }

        /**
         * @brief Reload of operator*(r)
         * @param r The number to mutiply
         * @return Integer Value of ( this * r )
         */
        Integer operator*(const Integer& r)
        {
            bool flag = !(this->_isPostive ^ r._isPostive);
            // size_t m = std::max(this->_idata->length(), r._idata->length());
            auto tmp = this->_idata->newObject();
            tmp->extend(this->_idata->length() + r._idata->length() + 1);
            bool bigger = _absGreater(*this, r);
            const Integer& num1 = (bigger ? *this : r);
            const Integer& num2 = (bigger ? r : *this);

            ///@todo FFT implement
#if HRCORE_HIGHPERF
#error HRCORE_HIGHPERF is enabled but currently not support (FFT) !
#else
            auto sop = tmp->begin();
            for (auto i = num1._idata->begin(); i; i = num1._idata->next(i), sop = tmp->next(sop)) {
                auto op = sop;
                for (auto j = num2._idata->begin(); j; j = num2._idata->next(j), op = tmp->next(op)) {
                    idval_t t = (idval_t)*i.data * (idval_t)*j.data;
                    _doInc(tmp, op, t & (ival_t)0xFFFFFFFFFFFFFFFF); // Current bit
                    ival_t inc = t >> (HRCORE_UNIT_SIZE);
                    if (inc) // Increase bit
                        _doInc(tmp, tmp->next(op), inc);
                }
            }
#endif
            return Integer(tmp, flag, true, true);
        }

        /**
         * @brief A function to calculate (*this / r)
         * @return DivisionResult_t
         */
        DivisionResult_t divideBy(const Integer& r)
        {
            if (_absEqual(this->make(), r))
                throw Exception::DividedByZero();
            bool flag = !(this->_isPostive ^ r._isPostive);
            auto tmp = this->_idata->newObject();
            ///@todo FFT implement
#if HRCORE_HIGHPERF
#error HRCORE_HIGHPERF is enabled but currently not support (NTT) !
#else
            size_t ls = 0;
            Integer div(r);
            div.positive(true); // Enforce div to be positive
            int8_t last = 0;
            for (; (last = _absRelation(*this, div)) == 1; ++ls) { // Find the highest bit of result
                div = div << 1;
                HRCORE_DBGI(std::cout << ls + 1 << std::endl;)
            }
            /* if (ls == 0) { // abs(r) >= abs(this)
                *tmp->begin().data = (last == 0);
                return Integer(tmp, flag, true, true);
            } */
            // ls--;
            // div = div >> 1; // Highest bit overflowed, cut out
            size_t roffset = (ls) >> (HRCORE_UNIT_DIV);
            int rem = ls - (roffset << (HRCORE_UNIT_DIV));
            tmp->extend(roffset + 1);
            auto op = tmp->at(roffset);
            /* if (last == 0) { // Div r-shift == this;
                *op.data = 1 << (rem + 1);
                return Integer(tmp, flag, true, true);
            } */
            Integer sum(this->_idata->newObject(), true, true);
            while (roffset != 0 || rem != -1) {
                Integer dtmp = sum + div;
                HRCORE_DBGI(std::cout << "[Integer::/] sum = " << sum << " div = " << div << " dtmp = " << dtmp << std::endl;)
                if (rem < 0) {
                    op = tmp->prev(op);
                    rem = HRCORE_UNIT_SIZE - 1;
                    roffset--;
                }
                if (_absRelation(*this, dtmp) >= 0) { // The bit is valid
                    *op.data |= 1 << rem;
                    sum = dtmp;
                }
                div = div >> 1;
                --rem;
            }
            sum._isPostive = this->_isPostive;
            Integer rt = *this - sum;
            rt._idata_ar = false; // Avoid auto-release to cause nullptr
            DivisionResult_t ret;
            ret.q_data = tmp;
            ret.r_data = rt._idata;
            ret.positive = flag;
            return ret;
#endif
        }

        /**
         * @brief Reload of operator/
         *
         * @param r The number to divide
         * @return Integer Value of (this / r)
         */
        Integer operator/(const Integer& r)
        {
            auto x = this->divideBy(r);
            x.r_data->delObject(x.r_data); // Avoid memory lack
            return Integer(x.q_data, x.positive, true, true);
        }

        /**
         * @brief Reload of operator%
         *
         * @param r The number to divide
         * @return Integer Value of (this % r)
         */
        Integer operator%(const Integer& r)
        {
            auto x = this->divideBy(r);
            x.q_data->delObject(x.q_data); // Avoid memory lack
            return Integer(x.r_data, x.positive, true, true);
        }

        /**
         * @brief Reload of operator-() const
         */
        Integer operator-() const
        {
            Integer ret(*this);
            ret._isPostive = !ret._isPostive;
            return ret;
        }

        /**
         * @brief Reload of operator+() const
         */
        Integer operator+() const { return Integer(*this); }

        /// @brief Return the sign of object
        bool positive() const { return this->_isPostive; }

        /**
         * @brief Set the sign of object
         * @param p Whether positive is true or false
         */
        void positive(bool p) { this->_isPostive = p; }

        /**
         * @brief Compare this and r with operator <
         *
         * @param r The number to be put in the right side to compare
         * @return bool (this < r)
         */
        bool operator<(const Integer& r)
        {
            if (this->_isPostive ^ r._isPostive)
                return !this->_isPostive;
            return (this->_isPostive ? _absGreater(r, *this) : _absGreater(*this, r));
        }

        /**
         * @brief Compare this and r with operator >
         *
         * @param r The number to be put in the right side
         * @return bool (this > r)
         */
        bool operator>(const Integer& r)
        {
            if (this->_isPostive ^ r._isPostive)
                return this->_isPostive;
            return (this->_isPostive ? _absGreater(*this, r) : _absGreater(r, *this));
        }

        /// @brief Test if (r == this)
        bool operator==(const Integer& r)
        {
            if (this->_isPostive ^ r._isPostive)
                return false;
            return _absEqual(*this, r);
        }

        ///@brief Test if (this >= r)
        bool operator>=(const Integer& r) { return (operator>(r) || operator==(r)); }

        ///@brief Test if (this <= r)
        bool operator<=(const Integer& r) { return (operator<(r) || operator==(r)); }

        /**
         * @brief Reload of << (Left shift)
         *
         * @param n Bits to shift
         * @return Integer Result of (this << n)
         */
        Integer operator<<(size_t n)
        {
            if (!n)
                return Integer(*this);
            // Integer ret(this->_idata->newObject(),this->_isPostive,true);
            auto ret = this->_idata->newObject();
            size_t offset = n >> HRCORE_UNIT_DIV;
            size_t rem = n - (offset << HRCORE_UNIT_DIV);
            if (rem > HRCORE_UNIT_SIZE)
                throw(Exception::InternalError());
            ret->extend(this->_idata->length() + offset + 1);
            for (auto s = this->_idata->begin(), d = ret->at(offset); s && d; s = this->_idata->next(s)) {
                *d.data |= *s.data << rem;
                d = ret->next(d);
                //*d.data |= *s.data >> ((HRCORE_UNIT_SIZE)-rem); // BUG! right shift count >= width of type [-Wshift-count-overflow] -> undefined
                *d.data |= (rem ? *s.data >> ((HRCORE_UNIT_SIZE)-rem) : 0);
            }
            return Integer(ret, this->_isPostive, true, true);
        }

        /**
         * @brief Reload of >> (Logical right shift)
         *
         * @param n Bits to shift
         * @return Integer Result of (this >> n)
         */
        Integer operator>>(size_t n)
        {
            if (!n)
                return Integer(*this);
            auto ret = this->_idata->newObject();
            size_t offset = n >> (HRCORE_UNIT_DIV);
            size_t rem = n - (offset << (HRCORE_UNIT_DIV));
            if (rem > HRCORE_UNIT_SIZE)
                throw(Exception::InternalError());
            if (offset > this->_idata->length() + 1) { // Overflow! BUG
                return this->make() = 0;
            }
            ret->extend(this->_idata->length() - offset + 1);
            for (auto s = this->_idata->end(), d = ret->at(this->_idata->length() - offset - 1); d && s; s = this->_idata->prev(s)) {
                *d.data |= *s.data >> rem;
                d = ret->prev(d);
                if (d)
                    *d.data |= (rem ? *s.data << ((HRCORE_UNIT_SIZE)-rem) : 0); // Same as before
            }
            return Integer(ret, this->_isPostive, true, true);
        }

        friend std::ostream& operator<<(std::ostream& os, Integer& rhs);
        friend std::istream& operator>>(std::istream& is, Integer& rhs);

    protected:
        /**
         * @brief Get relation of l and r
         * @return int8_t -1 when l < r, 0 when l = r, and 1 when l > r
         */
        static int8_t _absRelation(const Integer& l, const Integer& r)
        {
            size_t m = l._idata->length();
            size_t n = r._idata->length();
            if (m != n)
                return (m > n ? 1 : -1);
            auto op1 = l._idata->end();
            auto op2 = r._idata->end();
            for (size_t i = 0; i < m; ++i) {
                ival_t c1 = (op1 ? *op1.data : 0);
                ival_t c2 = (op2 ? *op2.data : 0);
                if (c1 != c2)
                    return (c1 > c2 ? 1 : -1);
                op1 = l._idata->prev(op1);
                op2 = r._idata->prev(op2);
            }
            return 0; // Equal
        }

        ///@brief Examine if l > r
        static bool _absGreater(const Integer& l, const Integer& r) { return (_absRelation(l, r) == 1); }

        ///@brief Examine if l == r
        static bool _absEqual(const Integer& l, const Integer& r) { return (_absRelation(l, r) == 0); }

        static bool _isZero(const Integer& l)
        {
            for (auto x = l._idata->begin(); x; x = l._idata->next(x)) {
                if (*x.data != 0)
                    return false;
            }
            return true;
        }

    private:
        /// @brief Remove unnecessary '0' of this and fix sign
        Integer& _simplify()
        {
            size_t t = this->_idata->length();
            for (auto i = this->_idata->end(); i && *i.data == 0; i = this->_idata->prev(i))
                --t;
            this->_idata->shrink((t == 0 ? 1 : t));
            if (_isZero(*this))
                this->_isPostive = true; // 0 is to be defined as positive
            return *this;
        }

        /// @brief Process the increase on the next item
        static void _doInc(Storage::Interface* i, Storage::Interface::item_t pos, ival_t inc)
        {
            do {
                idval_t ovf = (idval_t)inc + (idval_t)*pos.data;
                *pos.data = ovf & (ival_t)0xFFFFFFFFFFFFFFFF; // Max 64 bits and force convert to fit size
                inc = ovf >> (HRCORE_UNIT_SIZE);
                pos = i->next(pos);
            } while (inc); // Still overflow
        }

        ///@brief Ignore flag and do the plus.
        static void _absPlus(const Integer& left, const Integer& right, Storage::Interface* ret)
        {
            size_t m = std::max(left._idata->length(), right._idata->length());
            auto op1 = left._idata->begin();
            auto op2 = right._idata->begin();
            auto r = ret->begin();
            for (size_t i = 0; i < m; ++i) {
                ival_t c1 = (op1 ? *op1.data : 0);
                ival_t c2 = (op2 ? *op2.data : 0);
                idval_t tmp = (idval_t)c1 + (idval_t)c2;
                _doInc(ret, r, tmp & (ival_t)0xFFFFFFFFFFFFFFFF);
                //*r.data = tmp & (ival_t)0xFFFFFFFFFFFFFFFF;
                ival_t ovf = tmp >> (HRCORE_UNIT_SIZE);
                if (ovf) // Overflow
                    _doInc(ret, ret->next(r), ovf);
                op1 = left._idata->next(op1);
                op2 = right._idata->next(op2);
                r = ret->next(r); // BUG2, Fixed
            }
        }

        /// @brief Borrow 1 from higher position
        /// @return true if borrow is ok
        static bool _doBorrow(Storage::Interface* i, Storage::Interface::item_t pos)
        {
            size_t ls = 0;
            while (pos && *pos.data < 1) {
                pos = i->next(pos);
                ++ls;
            }
            if (pos) {
                --*pos.data;
                for (size_t j = 0; j < ls; ++j) {
                    pos = i->prev(pos);
                    *pos.data = (ival_t)0xffffffffffffffff;
                }
                return true;
            }
            return false;
        }

        ///@brief Ignore flag and calculate (left - right).
        ///@note (left > right) is required!
        static void _absSub(const Integer& left, const Integer& right, Storage::Interface* ret)
        {
            HRCORE_DBG(Integer ll(left); Integer rr(right);)
            HRCORE_DBG(std::cout << "[Integer::_absSub] Perform " << ll << " - " << rr << std::endl;)
            size_t m = std::max(left._idata->length(), right._idata->length());
            auto op1 = left._idata->begin();
            auto op2 = right._idata->begin();
            auto r = ret->begin();
            for (size_t i = 0; i < m; ++i) {
                if (!op1)
                    throw(Exception::LLR()); // left > right is required!
                ival_t c1 = *op1.data;
                ival_t c2 = (op2 ? *op2.data : 0);
                if (c1 >= c2) {
                    *r.data = c1 - c2;
                } else { // Not enough to do the sub
                    if (_doBorrow(left._idata, left._idata->next(op1))) {
                        idval_t nc = (idval_t)1 << (HRCORE_UNIT_SIZE) | c1;
                        *r.data = nc - c2;
                    } else {
                        throw(Exception::LLR()); // left < right
                    }
                }
                op1 = left._idata->next(op1);
                op2 = right._idata->next(op2);
                r = ret->next(r);
            }
        }

    protected:
        Storage::Interface* _idata = nullptr;
        bool _idata_ar = false;
        bool _isPostive = true;
    }; // Class Integer

#if HRCORE_ENABLE_FP

#ifndef __HRCORE_FP_OPERATOR_HELPER__
/**
 * @brief Define a helper macro to reduce code length
 */
#define __HRCORE_FP_OPERATOR_HELPER__(x) \
    bool x(const Float& rhs)             \
    {                                    \
        Float l(*this), r(rhs);          \
        _adjust(l, r);                   \
        return l.Integer::x(r);          \
    }
#endif

    /**
     * @brief Float type
     * @class Float
     * @details The class is a child of Integer; Somewhat like (IEEE-754)
     * @note If some function is not documented, please refer to document of Integer!
     */
    class Float : private Integer {
    public:
        // Inherit constructors and functions from Integer
        using Integer::Integer;
        using Integer::make;
        using Integer::positive;

        /// @brief Reload of move constructor
        Float(Float&& c)
            : Integer(c)
        {
            this->_fpos = c._fpos;
            this->_tpos = c._tpos;
        }

        Float(Integer&& c, int fpos, size_t tpos)
            : Integer(c)
        {
            HRCORE_DBGI(std::cout << "[Float::MoveConstructor] " << fpos << std::endl;)
            this->_fpos = fpos;
            this->_tpos = tpos;
        }

        /// @brief Reload of copy constructor
        Float(const Float& c)
            : Integer(c)
        {
            this->_fpos = c._fpos;
            this->_tpos = c._tpos;
        }

        /// @brief Copy value of Integer object and convert to a Float object
        Float(const Integer& c, int fpos = 0, size_t tpos = 0)
            : Integer(c)
        {
            this->_fpos = fpos;
            this->_tpos = tpos;
        }

        /// @brief Move operator
        Float& operator=(Float&& c)
        {
            HRCORE_DBGI(std::cout << "[Float::MoveOperator] " << c._fpos << std::endl;)
            this->Integer::operator=(c);
            this->_fpos = c._fpos;
            this->_tpos = c._tpos;
            return *this;
        }

        /// @brief Copy operator
        Float& operator=(const Float& c)
        {
            this->Integer::operator=(c);
            this->_fpos = c._fpos;
            this->_tpos = c._tpos;
            return *this;
        }

        /// @brief transform IEEE-754 Floating point value into Float object
        Float& operator=(const double c)
        {
            bool flag = (c >= 0);
            ival_t m = std::abs(c);
            //*this = this->Integer::operator=(m);
            Integer ret = this->make() = m;
            Integer tim = this->make() = 10;
            Integer div = this->make() = 1;
            int dtmp = -HRCORE_FP_BITS_BASE;
            int n = 0;
            for (double x = (std::abs(c) - m); std::abs(x) > 1e-9; x -= n) {
                x *= 10;
                n = x;
                Integer tmp = this->make() = n;
                ret = ret * tim + tmp;
                div = div * tim;
                dtmp -= HRCORE_FP_BITS_PER_DIGIT;
            }
            *this = Float((ret << -dtmp) / div, dtmp, 12);
            this->_isPostive = flag;
            return *this;
        }

        /// @brief Get current precision of division and input
        size_t precision() { return this->_precision; }

        /**
         * @brief Set precision of division and input.
         * @details The 'precision' use for input generate and division.
         *
         * For input generation, the precision is the minimal precision.
         * For division, the precision is the max precision.
         *
         * **To get actual precision, please make sure that both dividend and divider are generated from input with same precision.**
         * @param n The expected precision
         */
        void precision(size_t n) { this->_precision = n; }

        /// @brief Reload of operator+
        Float operator+(const Float& s)
        {
            Float l(*this), r(s);
            _adjust(l, r);
            return Float(l.Integer::operator+(r), l._fpos, l._tpos);
        }

        /// @brief Reload of operator-
        Float operator-(const Float& s)
        {
            Float l(*this), r(s);
            _adjust(l, r);
            return Float(l.Integer::operator-(r), l._fpos, l._tpos);
        }

        /// @brief Reload of operator*
        Float operator*(const Float& s) { return Float(this->Integer::operator*(s), this->_fpos + s._fpos, this->_tpos + s._tpos); }

        /// @brief Reload of operator/ (for Integer)
        Float operator/(const Integer& s)
        {
            if (this->_tpos >= this->_precision) {
                return Float(this->Integer::operator/(s), this->_fpos, this->_tpos);
            }
            // Integer tim = this->make() = 10;
            // Integer tmp = this->make() = 1;
            size_t n = this->_precision - this->_tpos;
            auto div = Float(*this << (n * HRCORE_FP_BITS_PER_DIGIT), this->_fpos - n * HRCORE_FP_BITS_PER_DIGIT, this->_precision); // May lost precision
            HRCORE_DBG(std::cout << "[Test] " << div << std::endl;)
            return div / s;
        }

        /// @brief Reload of operator/ (for Float)
        Float operator/(const Float& s)
        {
            if (this->_tpos < s._tpos + this->_precision) {
                size_t n = s._tpos + this->_precision - this->_tpos;
                auto div = *this << (n * HRCORE_FP_BITS_PER_DIGIT);
                return Float(div / s, this->_fpos - n * HRCORE_FP_BITS_PER_DIGIT - s._fpos, this->_precision);
            } else {
                return Float(this->Integer::operator/(s), this->_fpos - s._fpos, this->_precision);
            }
        }

        /// @brief Get Interger section
        Integer getInteger()
        {
            HRCORE_DBG(std::cout << this->_fpos << std::endl;)
            auto x = (this->_fpos >= 0 ? *this << this->_fpos : *this >> -(this->_fpos));
            x.positive(this->_isPostive);
            return x;
        }

        __HRCORE_FP_OPERATOR_HELPER__(operator<)
        __HRCORE_FP_OPERATOR_HELPER__(operator>)
        __HRCORE_FP_OPERATOR_HELPER__(operator==)
        __HRCORE_FP_OPERATOR_HELPER__(operator>=)
        __HRCORE_FP_OPERATOR_HELPER__(operator<=)

        friend std::istream& operator>>(std::istream& is, Float& rhs);
        friend std::ostream& operator<<(std::ostream& os, Float& rhs);

    private:
        /// @brief Adjust fp1 and fp2 to a value that has same fpos (minimal is preferred)
        static size_t _adjust(Float& fp1, Float& fp2)
        {
            int m = std::max(fp1._fpos, fp2._fpos);
            int n = std::min(fp1._fpos, fp2._fpos);
            size_t k = m - n;
            if (!k)
                return 0;
            Float& adj = (m == fp1._fpos ? fp1 : fp2);
            adj = Float(adj << k, n, std::max(fp1._tpos, fp2._tpos));
            return k;
        }

    protected:
        int _fpos = 0;
        size_t _tpos = 0; // Float point in decimal, only negative
        size_t _precision = HRCORE_FP_DEFAULT_PRECISION; // For division
    }; // Class Float

#endif

}
}

#endif