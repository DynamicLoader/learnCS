#include <iostream>
#include "../HRCore.h"
using namespace HRCore::Storage;
using namespace HRCore;
using namespace std;

int main()
{
    //std::cout << std::hex; // Test Hex First
    BINT c = BINT::make<LinkedList<8>>() = 100;
    // BINT g(-e);
    std::cout << "=== Begin Calc ===" << std::endl;
    BINT k = c;
    for (int i = 0; i < 50; ++i) {
        k = k * c;
        std::cout << "k: " << k << std::endl;
    }
    std::cout << "============== Sub ===============" << std::endl;
    k = c;
    BINT sub = BINT::make<LinkedList<8>>() = 0xcf0432dd;
    for (int i = 0; i < 20; ++i) {
        k = k - sub;
        std::cout << "k: " << k << std::endl;
    }

    std::cout << "============ Division ============" << std::endl;
    k = 0x1;
    BINT tmp = k << 193;
    std::cout << " tmp = " << tmp << std::endl;
    BINT tmp2 = tmp / k;
    BINT tmp3 = tmp - tmp2 * k;
    std::cout << " tmp / " << k << " = " << tmp2 << " --- " << tmp3 << std::endl;

    return 0;
}