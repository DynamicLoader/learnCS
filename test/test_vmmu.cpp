#include <iostream>
#include "../HRCore.h"

using namespace HRCore::Storage;
using namespace std;

template <size_t T>
void print(VirtualMMU<T>& out)
{
    cout << hex;
    for (auto x = out.begin(); x; x = out.next(x)) {
        cout << *x.data << ' ' << endl;
    }
    cout << endl;
}

int main()
{
    VirtualMMU<1024> a;
    a.extend(100);
    auto b = a.begin();
    for (int i = 0; i < 100; ++i) {
        if (b)
            *b.data = 0xff4f5d00 + i;
        b = a.next(b);
    }
    *a.at(0x56).data = 0xff3;
    *a.at(0x63).data = 0xff2;
    *a.at(0x62).data = 0xff1;
    auto c = a.at(8);
    *c.data = 0xff8;
    *a.prev(c).data = 0xff7;
    *a.next(c).data = 0xff9;
    print(a);
    
    return 0;
}