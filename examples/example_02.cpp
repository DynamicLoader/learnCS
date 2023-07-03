/**
 * @file example_02.cpp
 * @brief An example of using Float::precision
 * @version 0.1
 * @date 2022-12-08
 * @example examples/example_02.cpp
 * @note The file is the solution of Question C
 */

// Define it in compiler argument(s) to use pre-configured and merged header file
// #define HRCORE_USE_MERGE
#ifdef HRCORE_USE_MERGE
#include "../HRCore.hpp"
#else
#include "../HRCore.h"
#endif

using namespace std;
using namespace HRCore;

// Main function
int main()
{
    BFLOAT d = BFLOAT::make<Storage::LinkedList<8>>();
    d = 0.000001;
    int N = 0;
    cin >> N;
    while (N) {
        char o = '*';
        cin >> o;

        BFLOAT a = BINT::make<Storage::LinkedList<8>>();
        BFLOAT b = a.make();

        a.precision(10);
        b.precision(10);
        cin >> a >> b;
        BFLOAT x = a.make();
        if (o == '/') {
            if (Utils::abs(b) < d) {
                cout << "ERROR" << endl;
                --N;
                continue;
            }
            x = a / b;
        } else {
            x = a * b;
        }
        cout << x << endl;
        --N;
    }
    return 0;
}
