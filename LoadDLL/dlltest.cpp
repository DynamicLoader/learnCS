#include "dlltest.h"
#include "AsDLL.h"
#include <iostream>
#include <string>

using namespace std;

class Test1Impl : public Test1 {
private:
    int _i = 0;

public:
    void test(const char* str) { cout << "Test1Impl::test: " << str << endl; }
    void set(int i) { this->_i = i; }
    int get() { return this->_i; }

    static void* onInit() { return (void*)(new Test1Impl); }
    static void onDeinit(void* p) { delete (Test1Impl*)p; }
};

void printH(const char* ch)
{
    cout << "printH: " << ch << endl;
}

DLL_EXPORT_TABLE_t DLL_EXPORT_TABLE = {
    .ClassInfo = { { .className = "Test1", .initFunc = Test1Impl::onInit, .deinitFunc = Test1Impl::onDeinit } },
    .FuncInfo = { { .funcName = "printH", .func = (void*)printH } }
};
