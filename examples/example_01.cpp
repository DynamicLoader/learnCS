/**
 * @file example_01.cpp
 * @brief An example of using Integer type
 * @version 0.1
 * @date 2022-12-08
 * @example examples/example_01.cpp
 * @note The file is the solution of Question A and B (For B, using BFLOAT instead of BINT)
 */

// Define it in compiler argument(s) to use preconfigured and merged header file

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

    int N = 0;
    cin >> N;
    while (N) {
        char o = '+';
        cin >> o;
        // BINT a = BINT::make<Storage::LinkedList<8>>();
        // BINT b = a.make();
        BFLOAT a = BINT::make<Storage::LinkedList<8>>(); // using BINT::make() to get storage then convert to BFloat
        BFLOAT b = a.make();
        cin >> a >> b;
        auto x = (o == '-' ? a - b : a + b);
        cout << x << endl;
        --N;
    }
    return 0;
}
