#ifndef SERIALIZER_DE_HPP
#define SERIALIZER_DE_HPP
#include <string>
#include <fstream>
#include "errors.hpp"
#include "Vector.hpp"
#include "Node.hpp"

class Serializer_De {
    template <typename T>
    static inline void stream_valid(const std::string& e_msg, T& stream) { //cannot pass string view as exceptions need to own error message to keep them alive while stack unfolding
        if (stream.bad() || stream.fail()) {
            throw InsufficientSpaceError(e_msg);
        }
    }
public:
    static inline void file_exists(const std::string& filename, std::ofstream& stream) {
        if (!stream) {
            throw FileNotFoundError("Uh oh, file: " + filename + " could not be opened for writing!\n");
        }
    }

    static inline void file_exists(const std::string& filename, std::ifstream& stream) {
        if (!stream) {
            throw FileNotFoundError("Uh oh, file: " + filename + " could not be opened for reading!\n");
        }
    }
    
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

    static inline void stream_write(const Vector& var, const std::string& e_msg, std::ofstream& outf) {
        std::size_t dims{var.size()};
        unsigned const char* dptr{reinterpret_cast<unsigned const char*>(var.data.data())};
        outf.write(reinterpret_cast<const char*>(dptr), dims * sizeof(float));
        stream_valid(e_msg, outf);
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
            stream_write(node.data_size(), "Insufficient space on disk"s, outf);
            stream_write(node.data(), "Insufficient space on disk"s, outf);
            stream_write(node.edges(), "Insufficient space on disk"s, outf);
            stream_write(node.neighbors, "Insufficient space on disk"s, outf);
        }
    }

    static inline void stream_read(std::vector<Node>& nodes, const std::string& e_msg, std::ifstream& inf) {
        for (auto& node : nodes) {
            using namespace std::string_literals;
            stream_read(node.id, "File corrupted at node id\n"s, inf);
            std::uint64_t sz;
            stream_read(sz, "file corrupted at node size"s, inf);
            node.vector.data.resize(sz);
            stream_read(node.vector, "File corrupted at node data\n"s, inf);
            std::uint32_t edges;
            // std::cout << "Edges: " << edges << '\n';
            stream_read(edges, "File corrupted at node edges\n"s, inf);
            node.neighbors.resize(edges);
            stream_read(node.neighbors, "File corrupted at node neighbors\n"s, inf);
        }
    }
};

#endif //SERIALIZER_DE_HPP