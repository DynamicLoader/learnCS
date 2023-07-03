#include <iostream>
#include "../HRCore.h"
using namespace HRCore::Storage;
using namespace HRCore;
using namespace std;

int main(){
    //cin >> hex; // Test hex first
    //cout << hex;
    BINT a = BINT::make<LinkedList<1>>();
    BINT b = a.make();
    cin >> a >> b;
    BINT c = a + b;
    cout << a << '+' << b << endl;
    cout << c << endl;
    auto x = a.divideBy(b);
    BINT q = x.getQuotient();
    BINT r = x.getRemainder();
    x.clear(); // Not necessary; All element were used before
    cout << "a/b = " << q << " ... " << r << endl;
    return 0;
}