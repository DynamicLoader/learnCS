#include <iostream>
#include "../HRCore.h"

using namespace HRCore::Storage;
using namespace std;

template <size_t T>
void print(LinkedList<T>& out)
{
    cout << hex;
    for (auto x = out.end(); x; x = out.prev(x)) {
        cout << *x.data << ' ' << endl;
    }
    cout << endl;
}

int main()
{
    LinkedList<2> a;
    a.extend(100);
    auto b = a.begin();
    for (int i = 0; i < 100; ++i) {
        if (b)
            *b.data = 0xff4f5d00 + i;
        b = a.next(b);
    }
    auto c = a.at(8);
    *c.data = 0xff8;
    *a.prev(c).data = 0xff7;
    *a.next(c).data = 0xff9;
    *a.at(0x56).data = 0xff3;
    *a.at(0x63).data = 0xff2;
    *a.at(0x62).data = 0xff1;
    print(a);

    return 0;
}