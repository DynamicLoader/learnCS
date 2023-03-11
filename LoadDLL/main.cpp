#include "LoadDLL.h"
#include "dlltest.h"
#include <iostream>

using namespace std;

int main()
{

    LoadDLL d1("./d1.dll");
    if (!d1)
        exit(-1);
    d1.getFunc<void (*)(const char*)>("printH")("Hello,DLL!");
    Test1& t = d1.newClassObj<Test1>("Test1");
    if (!(&t))
        exit(-1);
    t.test("1234");
    cout << "Original i=" << t.get() << endl;
    t.set(-1);
    cout << "Now i=" << t.get() << endl;

    return 0;
}