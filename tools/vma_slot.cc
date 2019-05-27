#include <iostream>
#include <cstdint>
#include <limits>

uint64_t decode_fractal_x86_64(uint64_t pml4, uint64_t pdp, uint64_t pd,
                               uint64_t pt)
{
    uint64_t address = (pml4 << 39);

    if ((address & (1ll << 47)) > 0) {
        // We need to sign extend
        address |= 0xFFFF000000000000UL;
    }

    address |= pdp << 30;
    address |= pd << 21;
    address |= pt << 12;
    return address;
}

void do_x86_64() {
    // No error checking, but if you're using this, you should know what to
    // input anyways.
    int pml4, pdpt, pd, pt;
    std::cout << "pml4: ";
    std::cin >> pml4;
    std::cout << "pdpt: ";
    std::cin >> pdpt;
    std::cout << "pd: ";
    std::cin >> pd;
    std::cout << "pt: ";
    std::cin >> pt;
    std::cout << "0x" << std::hex << std::uppercase << decode_fractal_x86_64(pml4, pdpt, pd, pt) << "\n";
}

int main() {
    do_x86_64();
}
