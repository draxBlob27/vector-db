#ifndef ARCHIVE_H
#define ARCHIVE_H
#include <iostream>
#include <vector>
#include <fstream>

class VectorArchive
{
private:
    static const uint32_t s_magic_bytes{0x56454344};
    static const uint32_t s_version{1};

    struct FileInfo {
        uint32_t dim;
        uint64_t count;
        uint64_t bytes;
    };
    
    static const inline uint32_t s_crc_32_tab[] = {
        0x00000000, 0x78af3c54, 0x5dd4fe21, 0x257bc275, 0x17237acb, 0x6f8c469f, 
        0x4af784ea, 0x3258b8be, 0x2e46f596, 0x56e9c9c2, 0x73920bb7, 0x0b3d37e3, 
        0x39658f5d, 0x41cab309, 0x64b1717c, 0x1c1e4d28, 0x5c8deb2c, 0x2422d778, 
        0x0159150d, 0x79f62959, 0x4bae91e7, 0x3301adb3, 0x167a6fc6, 0x6ed55392, 
        0x72cb1eba, 0x0a6422ee, 0x2f1fe09b, 0x57b0dccf, 0x65e86471, 0x1d475825, 
        0x383c9a50, 0x4093a604, 0x159150d1, 0x6d3e6c85, 0x4845aef0, 0x30ea92a4, 
        0x02b22a1a, 0x7a1d164e, 0x5f66d43b, 0x27c9e86f, 0x3bd7a547, 0x43789913, 
        0x66035b66, 0x1eac6732, 0x2cf4df8c, 0x545be3d8, 0x712021ad, 0x098f1df9, 
        0x491cbbfd, 0x31b387a9, 0x14c845dc, 0x6c677988, 0x5e3fc136, 0x2690fd62, 
        0x03eb3f17, 0x7b440343, 0x675a4e6b, 0x1ff5723f, 0x3a8eb04a, 0x42218c1e, 
        0x707934a0, 0x08d608f4, 0x2dadca81, 0x5502f6d5, 0x2b22a1a2, 0x538d9df6, 
        0x76f65f83, 0x0e5963d7, 0x3c01db69, 0x44aee73d, 0x61d52548, 0x197a191c, 
        0x05645434, 0x7dcb6860, 0x58b0aa15, 0x201f9641, 0x12472eff, 0x6ae812ab, 
        0x4f93d0de, 0x373cec8a, 0x77af4a8e, 0x0f0076da, 0x2a7bb4af, 0x52d488fb, 
        0x608c3045, 0x18230c11, 0x3d58ce64, 0x45f7f230, 0x59e9bf18, 0x2146834c, 
        0x043d4139, 0x7c927d6d, 0x4ecac5d3, 0x3665f987, 0x131e3bf2, 0x6bb107a6, 
        0x3eb3f173, 0x461ccd27, 0x63670f52, 0x1bc83306, 0x29908bb8, 0x513fb7ec, 
        0x74447599, 0x0ceb49cd, 0x10f504e5, 0x685a38b1, 0x4d21fac4, 0x358ec690, 
        0x07d67e2e, 0x7f79427a, 0x5a02800f, 0x22adbc5b, 0x623e1a5f, 0x1a91260b, 
        0x3feae47e, 0x4745d82a, 0x751d6094, 0x0db25cc0, 0x28c99eb5, 0x5066a2e1, 
        0x4c78efc9, 0x34d7d39d, 0x11ac11e8, 0x69032dbc, 0x5b5b9502, 0x23f4a956, 
        0x068f6b23, 0x7e205777, 0x56454344, 0x2eea7f10, 0x0b91bd65, 0x733e8131, 
        0x4166398f, 0x39c905db, 0x1cb2c7ae, 0x641dfbfa, 0x7803b6d2, 0x00ac8a86, 
        0x25d748f3, 0x5d7874a7, 0x6f20cc19, 0x178ff04d, 0x32f43238, 0x4a5b0e6c, 
        0x0ac8a868, 0x7267943c, 0x571c5649, 0x2fb36a1d, 0x1debd2a3, 0x6544eef7, 
        0x403f2c82, 0x389010d6, 0x248e5dfe, 0x5c2161aa, 0x795aa3df, 0x01f59f8b, 
        0x33ad2735, 0x4b021b61, 0x6e79d914, 0x16d6e540, 0x43d41395, 0x3b7b2fc1, 
        0x1e00edb4, 0x66afd1e0, 0x54f7695e, 0x2c58550a, 0x0923977f, 0x718cab2b, 
        0x6d92e603, 0x153dda57, 0x30461822, 0x48e92476, 0x7ab19cc8, 0x021ea09c, 
        0x276562e9, 0x5fca5ebd, 0x1f59f8b9, 0x67f6c4ed, 0x428d0698, 0x3a223acc, 
        0x087a8272, 0x70d5be26, 0x55ae7c53, 0x2d014007, 0x311f0d2f, 0x49b0317b, 
        0x6ccbf30e, 0x1464cf5a, 0x263c77e4, 0x5e934bb0, 0x7be889c5, 0x0347b591, 
        0x7d67e2e6, 0x05c8deb2, 0x20b31cc7, 0x581c2093, 0x6a44982d, 0x12eba479, 
        0x3790660c, 0x4f3f5a58, 0x53211770, 0x2b8e2b24, 0x0ef5e951, 0x765ad505, 
        0x44026dbb, 0x3cad51ef, 0x19d6939a, 0x6179afce, 0x21ea09ca, 0x5945359e, 
        0x7c3ef7eb, 0x0491cbbf, 0x36c97301, 0x4e664f55, 0x6b1d8d20, 0x13b2b174, 
        0x0facfc5c, 0x7703c008, 0x5278027d, 0x2ad73e29, 0x188f8697, 0x6020bac3, 
        0x455b78b6, 0x3df444e2, 0x68f6b237, 0x10598e63, 0x35224c16, 0x4d8d7042, 
        0x7fd5c8fc, 0x077af4a8, 0x220136dd, 0x5aae0a89, 0x46b047a1, 0x3e1f7bf5, 
        0x1b64b980, 0x63cb85d4, 0x51933d6a, 0x293c013e, 0x0c47c34b, 0x74e8ff1f, 
        0x347b591b, 0x4cd4654f, 0x69afa73a, 0x11009b6e, 0x235823d0, 0x5bf71f84, 
        0x7e8cddf1, 0x0623e1a5, 0x1a3dac8d, 0x629290d9, 0x47e952ac, 0x3f466ef8, 
        0x0d1ed646, 0x75b1ea12, 0x50ca2867, 0x28651433
    };

public:
    void save(const std::string& file_path, const std::vector<std::vector<double>>& data)
    {
        const uint64_t count{data.size()};
        const uint32_t dimension{data[0].size()};
        
        uint32_t crc_32{0xFFFFFFFF};


        std::ofstream outf{file_path, std::ios::binary};

        outf.write(reinterpret_cast<const char *>(&s_magic_bytes), sizeof(uint32_t));
        outf.write(reinterpret_cast<const char *>(&s_version), sizeof(uint32_t));
        outf.write(reinterpret_cast<const char *>(&dimension), sizeof(uint32_t));
        outf.write(reinterpret_cast<const char *>(&count), sizeof(uint64_t));

        for (uint64_t i{0}; i < count; i++)
        {
            const char* d_ptr = reinterpret_cast<const char *>(&data[i][0]);
            outf.write(d_ptr, dimension * sizeof(double));
            for (int j{0}; j < dimension * sizeof(double); j++) {//we need crc every byte(ie, dimension*sizeof(double))
                int index{(crc_32 ^ d_ptr[j]) & 0xFF};
                crc_32 = (crc_32 >> 8) ^ s_crc_32_tab[index];
            }
        }

        crc_32 ^= 0xFFFFFFFF;
        outf.write(reinterpret_cast<char *>(&crc_32), sizeof(uint32_t));
    }

    std::vector<std::vector<double>> load(const std::string& file_path) {
        std::ifstream inf{file_path, std::ios::binary};

        uint32_t magic_bytes;
        inf.read(reinterpret_cast<char*>(&magic_bytes), sizeof(uint32_t));

        if (magic_bytes != s_magic_bytes)
            return {{0.0}}; //placeholder;

        uint32_t version;
        inf.read(reinterpret_cast<char*>(&version), sizeof(uint32_t));

        if (version != s_version)
            return {{0.0}}; //placeholder;

        uint32_t dimension;
        inf.read(reinterpret_cast<char*>(&dimension), sizeof(uint32_t));
        
        uint64_t count;
        inf.read(reinterpret_cast<char*>(&count), sizeof(uint64_t));
        
        uint32_t calc_crc_32{0xFFFFFFFF};
        std::vector<std::vector<double>> data(count, std::vector<double>(dimension));
        for (uint64_t i = 0; i < count; i++) {
            char* d_ptr = reinterpret_cast<char*>(&data[i][0]);
            inf.read(d_ptr, dimension * sizeof(double));
            for (int j{0}; j < dimension * sizeof(double); j++) {
                int index{(calc_crc_32 ^ d_ptr[j]) & 0xFF};
                calc_crc_32 = (calc_crc_32 >> 8) ^ s_crc_32_tab[index];
            }
        }

        calc_crc_32 ^= 0xFFFFFFFF;

        uint32_t crc_32;
        inf.read(reinterpret_cast<char*>(&crc_32), sizeof(uint32_t));


        if (calc_crc_32 != crc_32)
            return {{0.0}}; //placeholder
        
        return data;
    }

    FileInfo info(const std::string& file_path) {
        FileInfo f{};

        std::ifstream inf{file_path, std::ios::binary};
        inf.seekg(2 * sizeof(uint32_t));

        inf.read(reinterpret_cast<char*>(&f.dim), sizeof(uint32_t));
        inf.read(reinterpret_cast<char*>(&f.count), sizeof(uint64_t));

        f.bytes = 3 * sizeof(uint32_t) + sizeof(uint64_t) + f.dim * f.count * sizeof(double) + sizeof(uint32_t);
        return f;
    }

    bool verify(const std::string& file_path) {
        std::ifstream inf{file_path, std::ios::binary};

        uint32_t magic_bytes;
        inf.read(reinterpret_cast<char*>(&magic_bytes), sizeof(uint32_t));
        if (magic_bytes != s_magic_bytes) {
            return false;
        }

        uint32_t version;
        inf.read(reinterpret_cast<char*>(&version), sizeof(uint32_t));
        if (version != s_version) {
            return false;
        }

        uint32_t dimension;
        inf.read(reinterpret_cast<char*>(&dimension), sizeof(uint32_t));
        
        uint64_t count;
        inf.read(reinterpret_cast<char*>(&count), sizeof(uint64_t));
        
        uint32_t calc_crc_32{0xFFFFFFFF};
        std::vector<double> data(dimension);

        for (uint64_t i = 0; i < count; i++) {
            char* d_ptr = reinterpret_cast<char*>(&data[0]);
            inf.read(d_ptr, dimension * sizeof(double));
            for (int j{0}; j < dimension * sizeof(double); j++) {
                int index{(calc_crc_32 ^ d_ptr[j]) & 0xFF};
                calc_crc_32 = (calc_crc_32 >> 8) ^ s_crc_32_tab[index];
            }
        }

        calc_crc_32 ^= 0xFFFFFFFF;

        uint32_t crc_32;
        inf.read(reinterpret_cast<char*>(&crc_32), sizeof(uint32_t));

        if (calc_crc_32 != crc_32)
            return false;
        
        return true;
    }

    void append(const std::string& file_path, const std::vector<std::vector<double>>& data) {
        const uint64_t append_count{data.size()};
        const uint32_t append_dim{data[0].size()};

        std::fstream iof{file_path, std::ios::in | std::ios::out | std::ios::binary};

        iof.seekg(2 * sizeof(uint32_t), std::ios::beg); //magic number and version

        uint32_t dimension;
        iof.read(reinterpret_cast<char*>(&dimension), sizeof(uint32_t));
        if (dimension != append_dim) {
            return; //placeholder
        }

        uint64_t count;
        iof.read(reinterpret_cast<char*>(&count), sizeof(uint64_t));
        iof.seekg(-sizeof(uint64_t), std::ios::cur);
        const uint64_t new_count{count + append_count};

        iof.write(reinterpret_cast<const char*>(&new_count), sizeof(uint64_t));
        
        iof.seekg(dimension * count * sizeof(double), std::ios::cur); //data

        uint32_t crc_32;
        iof.read(reinterpret_cast<char*>(&crc_32), sizeof(uint32_t));

        crc_32 ^= 0xFFFFFFFF; //undoing previos xor with ~0 before saving
        
        for (uint64_t i{0}; i < append_count; i++) {
            const char* d_ptr{reinterpret_cast<const char*>(&data[i][0])};
            iof.write(d_ptr, dimension * sizeof(double));

            for (int j{0}; j < dimension * sizeof(double); j++) {
                int index{(crc_32 ^ d_ptr[j]) & 0xFF};
                crc_32 = (crc_32 >> 8) ^ s_crc_32_tab[index];
            }
        }

        crc_32 ^= 0xFFFFFFFF;

        iof.write(reinterpret_cast<char*>(&crc_32), sizeof(uint32_t));
    }
};
#endif