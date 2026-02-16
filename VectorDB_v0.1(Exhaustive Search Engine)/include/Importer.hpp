#ifndef IMPORTER_HPP
#define IMPORTER_HPP
#include <string>
#include <string_view>
#include <fstream>
#include <iostream>
#include <sstream>
#include <charconv>
#include <unordered_map>
#include <VectorStore.hpp>

enum class ImporterError : std::int32_t {
    FileNotFound = (-1)
};

class Importer {
public:
    static Result<std::unordered_map<std::string, uint64_t>, ImporterError> import_glove(const std::string& filename, VectorStore& vdb) {
        std::unordered_map<std::string, std::uint64_t> mpp{};
        std::ifstream inf{};
        std::uint64_t id{0}; 

        inf.open(filename);
        if (!inf) {
            std::cerr << "Uh oh! " + filename + " Could not be opened";
            return Err<ImporterError>{ImporterError::FileNotFound};
        }


        std::string temp;
        int dims = 100;

        while (std::getline(inf, temp)) {
            std::string_view line{temp};
            std::string word;

            const char *pos = line.data(), *end = line.data() + line.size();
            while (pos < end && *pos != ' ') {
                word.push_back(*pos);
                pos++;
            }

            pos++; //for skipping one space.

            Vector emb;
            emb.data.reserve(dims);

            while (pos < end) {
                float val;
                auto [newpos, ec] = std::from_chars(pos, end, val);

                if (ec == std::errc()) {
                    emb.data.push_back(val);
                    pos = newpos;
                    while (pos < end && *pos == ' ') pos++;
                } else {
                    break;
                }
            }

            if (emb.data.size() == dims) {
                mpp[word] = id++;
                vdb.insert(mpp[word], std::move(emb));
            }
        }

        return Ok{mpp};
    }
};
#endif