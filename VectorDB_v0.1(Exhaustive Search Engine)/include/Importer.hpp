#include <string>
#include <string_view>
#include <fstream>
#include <iostream>
#include <sstream>
#include <charconv>
#include <unordered_map>
#include <VectorStore.hpp>

class Importer {
private:
    std::unordered_map<std::string, int> mpp{};
    std::fstream inf{};
    std::uint64_t id{0};
    VectorStore vdb{};

public:
    void import_glove(const std::string& filename) {
        inf.open(filename);

        if (!inf) {
            std::cerr << "Uh oh! " + filename + " Could not be opened";
        }


        std::string temp;
        int dims = 100;

        while (std::getline(inf, temp)) {
            std::string_view word{temp};
            std::string temp;

            const char *pos = word.data(), *end = word.data() + word.size();
            while (*pos != ' ') {
                temp.push_back(*pos);
                pos++;
            }

            while (pos < end) {
                float val; //notaccecpting float in my comp;
                auto [newpos, ec] = std::from_chars(pos, end, val);
            }
        }
    }
};