#include "../HRCore.h"

using namespace HRCore;

int main(){
    Utils::Expression<BINT,Storage::LinkedList<8>> a;
    std::cin >> a;
    a.eval();
    std::cout << a;
    return 0;
}