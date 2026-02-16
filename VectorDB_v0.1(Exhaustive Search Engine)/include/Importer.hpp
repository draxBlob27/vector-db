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

struct Myres {
    std::unordered_map<std::string, std::uint64_t> word_to_id;
    std::unordered_map<std::uint64_t, std::string> id_to_word;
};

class Importer {
public:
    static Result<Myres, ImporterError> import_glove(const std::string& filename, VectorStore& vdb) {
        Myres mr;
        mr.id_to_word.reserve(1'200'000);
        mr.word_to_id.reserve(1'200'000);
        std::ifstream inf{};
        std::uint64_t id{0}; 

        inf.open(filename);
        if (!inf) {
            std::cerr << "Uh oh! " + filename + " Could not be opened";
            return Err<ImporterError>{ImporterError::FileNotFound};
        }

        std::string line;
        int dims = 100;
        
        while (std::getline(inf, line)) {
            const char *pos = line.data(), *end = line.data() + line.size();

            const char* word_start = pos;
            while (pos < end && *pos != ' ') {
                pos++;
            }

            std::string word(word_start, pos);

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
                mr.word_to_id.emplace(word, id);
                mr.id_to_word.emplace(id, word);

                vdb.insert(id, std::move(emb));
                id++;
            }

            word.clear();
        }

        return Ok{std::move(mr)};
    }
};
#endif