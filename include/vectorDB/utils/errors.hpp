#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <stdexcept>
#include <string>

class ArchiveError : public std::runtime_error {
public:
    ArchiveError(const std::string& error) 
        :std::runtime_error{error}
    {}
};

class FileNotFoundError : public ArchiveError {
public:
    FileNotFoundError(const std::string& error) 
        :ArchiveError(error)
    {}
};

class CorruptedDataError : public ArchiveError {
public:
    CorruptedDataError(const std::string& error)
        :ArchiveError(error)
    {}
};

class InvalidOperationError: public ArchiveError {
public: 
    InvalidOperationError(const std::string& error)
        :ArchiveError(error)
    {}
};

class InsufficientSpaceError : public ArchiveError {
public:
    InsufficientSpaceError(const std::string& error)
        :ArchiveError(error)
    {}
};

#endif //ERRORS_HPP