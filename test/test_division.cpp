#include <iostream>
#include "../HRCore.h"
using namespace HRCore::Storage;
using namespace HRCore;
using namespace std;

int main()
{
    cin >> hex; // Test hex first
    cout << hex;
    BINT a = BINT::make<VirtualMMU<8>>() = 1;
    BINT div = a.make() = 10;
    auto x = (a << HRCORE_UNIT_SIZE * 4) / div;
    cout << x << endl;

    return 0;
}