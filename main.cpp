#include <iostream>

#include "APInt.h"

int main() {
    APInt a(64, 123456789012345ULL);
    APInt b(64, 98765432109876ULL);

    APInt sum  = a + b;
    APInt diff = a - b;
    APInt prod = a * b;

    std::cout << "a      = ";
    a.printBinary(std::cout);
    std::cout << "\n";

    std::cout << "b      = ";
    b.printBinary(std::cout);
    std::cout << "\n";

    std::cout << "a + b  = ";
    sum.printBinary(std::cout);
    std::cout << "\n";

    std::cout << "a - b  = ";
    diff.printBinary(std::cout);
    std::cout << "\n";

    std::cout << "a * b  = ";
    prod.printBinary(std::cout);
    std::cout << "\n";

    // Demo with 128 bits.
    // (`x` and `y` are 2^64 - 1, so `x * y` requires more than 64 bits to represent.)
    APInt x(128, 0xFFFFFFFFFFFFFFFFULL);
    APInt y(128, 0xFFFFFFFFFFFFFFFFULL);
    APInt p = x * y;

    std::cout << "\n128-bit demo:\n";
    std::cout << "x      = ";
    x.printBinary(std::cout);
    std::cout << "\n";

    std::cout << "y      = ";
    y.printBinary(std::cout);
    std::cout << "\n";

    std::cout << "x * y  = ";
    p.printBinary(std::cout);
    std::cout << "\n";

    return 0;
}
