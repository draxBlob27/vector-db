#ifndef SERIALIZER_DE_HPP
#define SERIALIZER_DE_HPP
#include <string>
#include <fstream>
#include "errors.hpp"
#include "Vector.hpp"
#include "Node.hpp"

class Serializer_De {
public:
    template <typename T>
    static inline void file_exists(const std::string& filename, T& stream) {
        if (!stream) {
            throw FileNotFoundError("Uh oh, file: " + filename + " could not be opened for reading!\n");
        }
    }

    template <typename T>
    static inline void stream_valid(const std::string& e_msg, T& stream) { //cannot pass string view as exceptions need to own error message to keep them alive while stack unfolding
        if (stream.bad() || stream.fail()) {
            throw InsufficientSpaceError(e_msg);
        }
    };
    
    
    template <typename T>
    static inline void corruption_check(const T& a, const T& b) {
        if (a != b) {
            throw CorruptedDataError("Data Corrupted\n");
        }
    }
    
    template <typename T>
    static inline void stream_write(const T& data, const std::string& e_msg,  std::ofstream& outf) {
        outf.write(reinterpret_cast<const char*>(&data), sizeof(data));
        stream_valid(e_msg, outf);
    }
    
    template <typename T>
    static inline void stream_read(T& var, const std::string& e_msg, std::ifstream& inf) {
        inf.read(reinterpret_cast<char*>(&var), sizeof(var));
        stream_valid(e_msg, inf);
    }

    //.data() returns a pointer to begining.
    
    static inline void stream_read(Vector& var, const std::string& e_msg, std::ifstream& inf) {
        std::size_t dims{var.size()};
        inf.read(reinterpret_cast<char*>(var.data.data()), dims * sizeof(float));
        stream_valid(e_msg, inf);
    }
    
    template <typename T>
    static inline void stream_read(std::vector<T>& vec, const std::string& e_msg, std::ifstream& inf) {
        inf.read(reinterpret_cast<char *>(vec.data()), vec.size() * sizeof(T));
        stream_valid(e_msg, inf);
    }
    
    template <typename T>
    static inline void stream_write(const std::vector<T>& data, const std::string& e_msg, std::ofstream& outf) {
        if (data.empty()) {
            throw InvalidOperationError("Empty Data passed on\n");
        }
        unsigned const char* d_ptr{reinterpret_cast<unsigned const char*>(data.data())};
        outf.write(reinterpret_cast<const char*>(d_ptr), data.size() * sizeof(T));
        stream_valid(e_msg, outf);
    }
    
    static inline void stream_write(const std::vector<Vector>& data, const std::string& e_msg, std::ofstream& outf) {
        if (data.empty()) {
            throw InvalidOperationError("Empty Data passed on\n");
        }
    
        for (const auto& emb : data) {
            using namespace std::string_literals;
            stream_write(emb.data, "Insufficient space on disk"s, outf);
            stream_valid(e_msg, outf);
        }
    }

    static inline void stream_write(const std::vector<Node>& nodes, const std::string& e_msg, std::ofstream& outf) {
        for (const auto& node : nodes) {
            using namespace std::string_literals;
            stream_write(node.getId(), "Insufficient space on disk"s, outf);
            stream_write(node.data(), "Insufficient space on disk"s, outf);
            stream_write(node.edges(), "Insufficient space on disk"s, outf);
            stream_write(node.neighbors, "Insufficient space on disk"s, outf);
        }
    }
};

#endif //SERIALIZER_DE_HPP