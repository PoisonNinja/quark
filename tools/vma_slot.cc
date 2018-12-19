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

uint32_t decode_fractal_x86(uint32_t pd, uint32_t pt)
{
    uint32_t address = (pd << 22);
    address |= (pt << 12);
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
    std::cout << std::hex << decode_fractal_x86_64(pml4, pdpt, pd, pt) << "\n";
}

void do_x86() {
    int pd, pt;
    std::cout << "pd: ";
    std::cin >> pd;
    std::cout << "pt: ";
    std::cin >> pt;
    std::cout << std::hex << decode_fractal_x86(pd, pt) << "\n";
}

int main() {
    int arch = 0;
    while (1) {
        std::cout << "Architectures:\n"
                  << "  1). x86\n"
                  << "  2). x86_64\n";
        std::cout << "Selection: ";
        std::cin >> arch;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            std::cout << "Bad input\n";
            continue;
        } else {
            break;
        }
    }
    switch (arch) {
        case 1:
            do_x86();
            break;
        case 2:
            do_x86_64();
            break;
        default:
            return 0;
   }
}
