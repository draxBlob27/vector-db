#include <iostream>
#include <vector>
#include <fstream>

class VectorArchive {
private:
    static const uint32_t s_magic_bytes{0x56454344};
    static const uint32_t s_version{1}; 
    uint32_t crc32{};
public:
    void save(std::string file_path, uint32_t dimension, uint64_t count) {
        
        std::vector<std::vector<float>> data{};

        std::ofstream outf{file_path, std::ios::binary};

        outf.write(reinterpret_cast<const char*>(&s_magic_bytes), sizeof(uint32_t));
        outf.write(reinterpret_cast<const char*>(&s_version), sizeof(uint32_t));
        outf.write(reinterpret_cast<char*>(&dimension), sizeof(uint32_t));
        outf.write(reinterpret_cast<char*>(&count), sizeof(uint64_t));

        for (int i{0}; i < count; i++) {
            outf.write(reinterpret_cast<char*>(&data[i][0]), dimension * sizeof(float));
        }

        outf.w
    }
};