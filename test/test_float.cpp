#include <iostream>
#include "../HRCore.h"
using namespace HRCore::Storage;
using namespace HRCore;
using namespace std;

int main()
{
    // cin >> hex; // Test hex first
    // cout << hex;
    BFLOAT a = BFLOAT::make<LinkedList<8>>();
    BFLOAT b = a.make();
    BFLOAT d = a.make();
    d = 0.000006f;
    a.precision(10);
    b.precision(10);
    cin >> a;
    cin >> b;
    if(b < d){
       cout << "Quotient may be too large!" << endl;
    }
    //cout << d << endl << endl;
    auto x = a / b;
    //x.precision(200);
    cout << "a = " << a << ", b = " << b << endl;
    cout << "a / b = " << x;

    return 0;
}