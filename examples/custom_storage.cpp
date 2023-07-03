/**
 * @file custom_storage.cpp
 * @brief An example shows how to customize a storage backend
 * @version 1.0
 * @date 2023-01-27
 */

#include <iostream>
#include "../HRCore.h"
using namespace std;
using namespace HRCore;

class CustomStorage : public Storage::Interface {
private:
    ival_t _d[1024] = { 0 };
    size_t _count = 1;

public:
    struct data_t : public item_t { // Access protected data member in item_t via data_t
        friend class CustomStorage;
    };

    item_t at(size_t n)
    {
        data_t ret;
        if (n >= this->_count) {
            return ret;
        }

        ret.data = this->_d + n;
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
        this->_count = n;
        return true;
    }
    bool extend(size_t n)
    {
        this->_count = n;
        return true;
    }

    Interface* newObject() { return new CustomStorage(); }
    void delObject(const Interface* ptr) { delete (const CustomStorage*)ptr; }

    virtual ~CustomStorage() { } // Must be virtual
};

int main(){
    //cin >> hex; // Test hex first
    //cout << hex;
    BINT a = BINT::make<CustomStorage>();
    BINT b = a.make();
    cin >> a >> b;
    BINT c = a + b;
    cout << a << '+' << b << endl;
    cout << c << endl;
    return 0;
}
