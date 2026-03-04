#ifndef IMPORTER_HPP
#define IMPORTER_HPP
#include <string>
#include <fstream>
#include <iostream>
#include <ios>
#include <charconv>
#include <unordered_map>
#include "VectorStore.hpp"
#include "LSH_Index.hpp"

enum class ImporterError : std::int32_t {
    GLoVEFileNotFound = (-1),
    SIFT1MFilenotFound = (-2),
    SIFT1MFileCorrupted = (-3)
};

struct GloveRes {
    std::unordered_map<std::string, std::uint64_t> word_to_id;
    std::unordered_map<std::uint64_t, std::string> id_to_word;
    std::vector<Vector> vectors;
    std::vector<std::uint64_t> ids;
};

struct SiftRes {
    std::vector<std::vector<float>> queries;
    std::vector<std::vector<std::uint32_t>> truths;
    std::vector<std::uint32_t> truth_k;
    std::vector<Vector> vectors;
    std::vector<std::uint64_t> ids;
};

class Importer {
public:
    //can take dimensions from caller
    static Result<GloveRes, ImporterError> import_glove(const std::string& filename) {
        GloveRes mr;
        // mr.id_to_word.reserve(1'200'000);
        // mr.word_to_id.reserve(1'200'000);
        std::ifstream inf{};
        std::uint64_t id{0}; 

        inf.open(filename);
        if (!inf) {
            std::cerr << "Uh oh! " + filename + " Could not be opened";
            return Err<ImporterError>{ImporterError::GLoVEFileNotFound};
        }

        std::string line;
        int dims = 100;
        
        while (std::getline(inf, line)) {//getline works as flag for error as well as input for lines
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

                mr.vectors.push_back(std::move(emb));
                mr.ids.push_back(id);
                id++;
            }

            word.clear();
        }

        return Ok{mr};
    }

    static Result<SiftRes, ImporterError> import_sift1m(const std::string& data_file, const std::string& query_file, const std::string& truth_file, std::uint32_t k_imports = -1) {
        SiftRes mr;

        std::ifstream inf{}; 

        inf.open(data_file, std::ios::binary);
        if (!inf) {
            std::cerr << "Uh oh! " + data_file + " Could not be opened";
            return Err<ImporterError>{ImporterError::SIFT1MFilenotFound};
        }

        std::uint32_t dims;
        std::uint64_t cnt{0};

        while (!inf.eof()) {
            inf.read(reinterpret_cast<char*>(&dims), 4);
            if (inf.bad() || inf.fail()) {
                return Err{ImporterError::SIFT1MFileCorrupted};
            }

            if (dims != 128) {
                return Err{ImporterError::SIFT1MFileCorrupted};
            }

            std::vector<float> emb(dims);
            unsigned char* d_ptr = reinterpret_cast<unsigned char*>(&emb[0]);
            inf.read(reinterpret_cast<char*>(d_ptr), 4 * dims);
            if (inf.bad() || inf.fail()) {
                return Err{ImporterError::SIFT1MFileCorrupted};
            }

            mr.vectors.push_back(std::move(emb));
            mr.ids.push_back(cnt);
            cnt++;
            k_imports--;

            if (inf.peek() == EOF || !k_imports) {
                break;
            }
        }

        inf.close();


        //now reading query file
        inf.open(query_file, std::ios::binary);
        if (!inf) {
            std::cerr << "Uh oh! " + query_file + " Could not be opened";
            return Err<ImporterError>{ImporterError::SIFT1MFilenotFound};
        }

        while (!inf.eof()) {
            inf.read(reinterpret_cast<char*>(&dims), 4);
            if (inf.bad() || inf.fail()) {
                return Err{ImporterError::SIFT1MFileCorrupted};
            }

            if (dims != 128) {
                return Err{ImporterError::SIFT1MFileCorrupted};
            }

            std::vector<float> q(dims);
            unsigned char* d_ptr = reinterpret_cast<unsigned char*>(&q[0]);
            inf.read(reinterpret_cast<char*>(d_ptr), 4 * dims);
            if (inf.bad() || inf.fail()) {
                return Err{ImporterError::SIFT1MFileCorrupted};
            }

            mr.queries.push_back(q);
            if (inf.peek() == EOF) {
                break;
            }
        }

        inf.close();

        //now reading truth file
        inf.open(truth_file, std::ios::binary);
        if (!inf) {
            std::cerr << "Uh oh! " + truth_file + " Could not be opened";
            return Err<ImporterError>{ImporterError::SIFT1MFilenotFound};
        }

        std::uint32_t k;
        
        while (!inf.eof()) {
            inf.read(reinterpret_cast<char*>(&k), 4);
            if (inf.bad() || inf.fail()) {
                return Err{ImporterError::SIFT1MFileCorrupted};
            }

            std::vector<std::uint32_t> t(k);
            unsigned char* d_ptr = reinterpret_cast<unsigned char*>(&t[0]);
            inf.read(reinterpret_cast<char*>(d_ptr), 4 * k);
            if (inf.bad() || inf.fail()) {
                return Err{ImporterError::SIFT1MFileCorrupted};
            }

            mr.truths.push_back(t);
            mr.truth_k.push_back(k);

            if (inf.peek() == EOF) {
                break;
            }
        }

        inf.close();

        return Ok{mr};
    }
};
#endif //IMPORTER_HPP