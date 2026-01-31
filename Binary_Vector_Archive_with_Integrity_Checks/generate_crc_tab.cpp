#include <iostream>
#include <iomanip>
#include <cstdint>

int main() {
    uint32_t poly = 0x56454344; //"VECD"

    for (int i = 0; i < 256; i++) {
        uint32_t crc = i;

        for (int j = 0; j < 8; j++) {
            // Check the Lowest Bit (LSB) of the *current* crc
            if (crc & 1) { 
                // If LSB is 1: Shift Right, then XOR
                crc = (crc >> 1) ^ poly; 
            } else {
                // If LSB is 0: Just Shift Right
                crc = (crc >> 1);
            }
        }

        std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << crc << ", ";
        
        if ((i + 1) % 6 == 0) 
          std::cout << "\n";
    }
    std::cout << "\n";
}