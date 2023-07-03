
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "../HRCore.h"
#include "./calculator.h"

using namespace std;
using namespace HRCore;

class CustomStorage;
class ThreadRequiredStop : public std::exception {
public:
    const char* what() const throw() override
    {
        return "Thread Rquired to Stop";
    }
};

using cpool_t = unordered_set<const CustomStorage*>;

class CustomStorage : public Storage::Interface {
private:
    struct data_t : public item_t { // Access protected data member in item_t via data_t
        friend class CustomStorage;
    };

    struct node_t {
        ival_t data[256] = { 0 }; // 2^8
    };

    vector<node_t*> _pool;
    size_t _count = 1;
    size_t _len = 1;
    cpool_t* _mem = nullptr;

public:
    item_t at(size_t n)
    {
        if (stopCalc)
            throw(ThreadRequiredStop());
        data_t ret;
        if (n >= this->_count) {
            return ret;
        }
        size_t p = n >> 8;
        size_t f = n & 0xFF;

        ret.data = this->_pool.at(p)->data + f;
        ret.index = n;
        return ret;
    }

    item_t begin() { return this->at(0); }
    item_t end() { return this->at(this->_count - 1); }
    size_t length() { return this->_count; }

    item_t prev(item_t cur)
    {
        if (!cur)
            return item_t {};
        return this->at(((data_t*)&cur)->index - 1);
    }
    item_t next(item_t cur)
    {
        if (!cur)
            return item_t {};
        return this->at(((data_t*)&cur)->index + 1);
    }

    bool shrink(size_t n)
    {
        if (stopCalc)
            throw(ThreadRequiredStop());
        if (n >= this->_count)
            return false;
        size_t p = (n >> 8) + (n & 0xFF ? 1 : 0);
        while (this->_len > p) {
            delete *(this->_pool.rbegin());
            this->_pool.pop_back();
            --this->_len;
        }
        this->_count = n;
        return true;
    }
    bool extend(size_t n)
    {
        if (stopCalc)
            throw(ThreadRequiredStop());
        if (n <= this->_count)
            return false;
        size_t p = (n >> 8) + (n & 0xFF ? 1 : 0);
        while (this->_len < p) {
            this->_pool.push_back(new node_t);
            ++this->_len;
        }
        this->_count = n;
        return true;
    }

    Interface* newObject()
    {
        auto x = new CustomStorage(this->_mem);
        this->_mem->insert(x); // Put this into bucket for exception auto cleaning
        return x;
    }
    void delObject(const Interface* ptr)
    {
        this->_mem->erase((const CustomStorage*)ptr);
        delete (const CustomStorage*)ptr;
    }

    virtual ~CustomStorage()
    {
        for (auto x = this->_pool.begin(); x != this->_pool.end(); ++x) {
            delete *x;
        }
        this->_len = 0;
        this->_count = 0;
    }
    CustomStorage(cpool_t* mem)
    {
        this->_pool.push_back(new node_t);
        this->_mem = mem;
    }

    CustomStorage(){ // dummy constructor
        throw(0);
    }
};

void _cpool_release(cpool_t& pool)
{
    for (auto x = pool.begin(); x != pool.end(); ++x) {
        delete *x;
    }
}

string easyCalculate(const char* A, const char* B, const char* C, uint8_t mode)
{
    istringstream Ain(A), Bin(B), Cin(C);
    // BINT at = BINT::make<Storage::LinkedList<1024>>();
    cpool_t pool;
    try {
        CustomStorage a(&pool);
        BINT at(&a, true, false, false);
        BINT bt = at.make(), ct = at.make();
        BINT tmp = at.make(), out = at.make();
        Ain >> at;
        Bin >> bt;
        Cin >> ct;
        char first = mode >> 4, sec = mode & 0xF;
        if (first > 1 && sec > 1) { // */
            tmp = (first == 2 ? at * bt : at / bt);
            out = (sec == 2 ? tmp * ct : tmp / ct);
        } else if (first < 2 && sec > 1) {
            tmp = (sec == 2 ? bt * ct : bt / ct);
            out = (first ? at - tmp : at + tmp);
        } else if (first > 1 && sec < 2) {
            tmp = (first == 2 ? at * bt : at / bt);
            out = (sec ? tmp - ct : tmp + ct);
        } else {
            tmp = (first ? at - bt : at + bt);
            out = (sec ? tmp - ct : tmp + ct);
        }
        ostringstream ret;
        ret << out;

        return ret.str();
    } catch (ThreadRequiredStop) {
        _cpool_release(pool);
        return "Calculation Cancelled";
    } catch (Exception::DividedByZero) {
        for (auto x = pool.begin(); x != pool.end(); ++x) {
            delete *x;
        }
        return "Divided by zero is not allow!";
    }

    return "Calculation ended UNEXPECTLY!";
}

string advCalculate(const char* input)
{

    cpool_t pool;
    try {
        istringstream A(input);
        CustomStorage st(&pool);
        BEXP<BINT, CustomStorage> exp(&st);
        A >> exp;
        ostringstream out;
        if(!exp.eval())
            return "Error during evaluating expression: Invalid expression?";
        out << exp;
        return out.str();
    } catch (ThreadRequiredStop) {
        _cpool_release(pool);
        return "Calculation Cancelled";
    } catch (Exception::ExpressionInvalidOperation) {
        _cpool_release(pool);
        return "Error during evaluating expression: Invalid operation!";
    } catch (Exception::ExpressionConvertFailed) {
         _cpool_release(pool);
        return "Error during convert into expression: Invalid expression?";
    }catch(Exception::DividedByZero){
         _cpool_release(pool);
        return "Error during evaluating expression: Divide by zero is not allowed!";
    }
}
