#include "archive.h"

void VectorArchive::save(const std::string &file_path, const std::vector<std::vector<double>> &data)
{
    const uint64_t count{data.size()};
    if (!count) {
        throw InvalidOperationError("No data to store.");
    }

    const uint32_t dimension{data[0].size()};

    uint32_t crc_32{0xFFFFFFFF};

    std::ofstream outf{file_path, std::ios::binary};
    if (!outf) {
        throw FileNotFoundError("Uh oh, file: " + file_path + " could not be opened for writing!\n");
    }

    outf.write(reinterpret_cast<const char *>(&s_magic_bytes), sizeof(uint32_t));
    if (outf.bad() | outf.fail()) {
        throw InsufficientSpaceError("Insufficient space on disk.");
    }

    outf.write(reinterpret_cast<const char *>(&s_version), sizeof(uint32_t));
    if (outf.bad() | outf.fail()) {
        throw InsufficientSpaceError("Insufficient space on disk.");
    }
    outf.write(reinterpret_cast<const char *>(&dimension), sizeof(uint32_t));
    if (outf.bad() | outf.fail()) {
        throw InsufficientSpaceError("Insufficient space on disk.");
    }

    outf.write(reinterpret_cast<const char *>(&count), sizeof(uint64_t));
    if (outf.bad() | outf.fail()) {
        throw InsufficientSpaceError("Insufficient space on disk.");
    }

    for (uint64_t i{0}; i < count; i++)
    {
        const unsigned char *d_ptr = reinterpret_cast<const unsigned char *>(&data[i][0]);

        if (data[i].size() != dimension) {
            throw InvalidOperationError("Dimension mismatch in vector data.");
        }

        outf.write(reinterpret_cast<const char*>(d_ptr), dimension * sizeof(double));
        if (outf.bad() | outf.fail()) {
            throw InsufficientSpaceError("Insufficient space on disk.");
        }

        for (std::size_t j{0}; j < dimension * sizeof(double); j++)
        { // we need crc every byte(ie, dimension*sizeof(double))
            int index{(crc_32 ^ d_ptr[j]) & 0xFF};
            crc_32 = (crc_32 >> 8) ^ s_crc_32_tab[index];
        }
    }

    crc_32 ^= 0xFFFFFFFF;
    outf.write(reinterpret_cast<char *>(&crc_32), sizeof(uint32_t));
    if (outf.bad() | outf.fail()) {
        throw InsufficientSpaceError("Insufficient space on disk.");
    }
}

std::vector<std::vector<double>> VectorArchive::load(const std::string &file_path, bool already_verified)
{
    std::ifstream inf{file_path, std::ios::binary};
    if (!inf) {
        throw FileNotFoundError("Uh oh, file: " + file_path + " could not be opened for reading!\n");
    }

    uint32_t magic_bytes;
    inf.read(reinterpret_cast<char *>(&magic_bytes), sizeof(uint32_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }

    if (magic_bytes != s_magic_bytes)
        throw CorruptedDataError("Magic bytes mismatch."); // placeholder;

    uint32_t version;
    inf.read(reinterpret_cast<char *>(&version), sizeof(uint32_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }

    if (version != s_version)
        throw CorruptedDataError("Version mismatch.");

    uint32_t dimension;
    inf.read(reinterpret_cast<char *>(&dimension), sizeof(uint32_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }

    uint64_t count;
    inf.read(reinterpret_cast<char *>(&count), sizeof(uint64_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }

    uint32_t calc_crc_32{0xFFFFFFFF};
    std::vector<std::vector<double>> data(count, std::vector<double>(dimension));
    //to implement check for file size here

    for (uint64_t i = 0; i < count; i++)
    {
        unsigned char *d_ptr = reinterpret_cast<unsigned char *>(&data[i][0]);
        inf.read(reinterpret_cast<char*>(d_ptr), dimension * sizeof(double));
        if (inf.fail() | inf.bad()) {
            throw ArchiveError("Could not read file.");
        }

        if (!already_verified) {
            for (std::size_t j{0}; j < dimension * sizeof(double); j++)
            {
                int index{(calc_crc_32 ^ d_ptr[j]) & 0xFF};
                calc_crc_32 = (calc_crc_32 >> 8) ^ s_crc_32_tab[index];
            }
        }

    }

    if (!already_verified)
        calc_crc_32 ^= 0xFFFFFFFF;

    uint32_t crc_32;
    inf.read(reinterpret_cast<char *>(&crc_32), sizeof(uint32_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }

    if (!already_verified && calc_crc_32 != crc_32)
        throw CorruptedDataError("CRC mismatch.");

    return data;
}

VectorArchive::FileInfo VectorArchive::info(const std::string &file_path)
{
    FileInfo f{};

    std::ifstream inf{file_path, std::ios::binary};
    if (!inf) {
        throw FileNotFoundError("Uh oh, file: " + file_path + " could not be opened for reading!\n");
    }

    inf.seekg(2 * sizeof(uint32_t));

    inf.read(reinterpret_cast<char *>(&f.dim), sizeof(uint32_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }

    inf.read(reinterpret_cast<char *>(&f.count), sizeof(uint64_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }

    f.bytes = 3 * sizeof(uint32_t) + sizeof(uint64_t) + f.dim * f.count * sizeof(double) + sizeof(uint32_t);
    return f;
}

bool VectorArchive::verify(const std::string &file_path)
{
    std::ifstream inf{file_path, std::ios::binary};
    if (!inf) {
        throw FileNotFoundError("Uh oh, file: " + file_path + " could not be opened for reading!\n");
    }

    uint32_t magic_bytes;
    inf.read(reinterpret_cast<char *>(&magic_bytes), sizeof(uint32_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }

    if (magic_bytes != s_magic_bytes)
    {
        return false;
    }

    uint32_t version;
    inf.read(reinterpret_cast<char *>(&version), sizeof(uint32_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }

    if (version != s_version)
    {
        return false;
    }

    uint32_t dimension;
    inf.read(reinterpret_cast<char *>(&dimension), sizeof(uint32_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }

    uint64_t count;
    inf.read(reinterpret_cast<char *>(&count), sizeof(uint64_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }

    uint32_t calc_crc_32{0xFFFFFFFF};
    std::vector<double> data(dimension);

    for (uint64_t i = 0; i < count; i++)
    {
        unsigned char *d_ptr = reinterpret_cast<unsigned char *>(&data[0]);
        inf.read(reinterpret_cast<char*>(d_ptr), dimension * sizeof(double));
        if (inf.fail() | inf.bad()) {
            throw ArchiveError("Could not read file.");
        }

        for (std::size_t j{0}; j < dimension * sizeof(double); j++)
        {
            int index{(calc_crc_32 ^ d_ptr[j]) & 0xFF};
            calc_crc_32 = (calc_crc_32 >> 8) ^ s_crc_32_tab[index];
        }
    }

    calc_crc_32 ^= 0xFFFFFFFF;

    uint32_t crc_32;
    inf.read(reinterpret_cast<char *>(&crc_32), sizeof(uint32_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }

    if (calc_crc_32 != crc_32)
        return false;

    return true;
}

void VectorArchive::append(const std::string &file_path, const std::vector<std::vector<double>> &data)
{
    const uint64_t append_count{data.size()};
    if (!append_count) {
        throw InvalidOperationError("no data to append.");
    }
    const uint32_t append_dim{data[0].size()};

    std::fstream iof{file_path, std::ios::in | std::ios::out | std::ios::binary};
    if (!iof) {
        throw FileNotFoundError("Uh oh, file: " + file_path + " could not be opened for writing!\n");
    }

    iof.seekg(2 * sizeof(uint32_t), std::ios::beg); // magic number and version

    uint32_t dimension;
    iof.read(reinterpret_cast<char *>(&dimension), sizeof(uint32_t));
    if (iof.fail() | iof.bad()) {
        throw ArchiveError("Could not read file.");
    }

    if (dimension != append_dim)
    {
        throw InvalidOperationError("New data dimension mismatch.");
    }

    uint64_t count;
    iof.read(reinterpret_cast<char *>(&count), sizeof(uint64_t));
    if (iof.fail() | iof.bad()) {
        throw ArchiveError("Could not read file.");
    }

    iof.seekg(-sizeof(uint64_t), std::ios::cur);
    const uint64_t new_count{count + append_count};

    iof.write(reinterpret_cast<const char *>(&new_count), sizeof(uint64_t));
    if (iof.fail() | iof.bad()) {
        throw ArchiveError("Could not write in file.");
    }

    iof.seekg(dimension * count * sizeof(double), std::ios::cur); // data

    uint32_t crc_32;
    iof.read(reinterpret_cast<char *>(&crc_32), sizeof(uint32_t));
    if (iof.fail() | iof.bad()) {
        throw ArchiveError("Could not read file.");
    }

    crc_32 ^= 0xFFFFFFFF; // undoing previos xor with ~0 before saving

    iof.seekg(-sizeof(uint32_t), std::ios::cur);

    for (uint64_t i{0}; i < append_count; i++)
    {
        if (data[i].size() != dimension) {
            throw InvalidOperationError("Dimension of new data mismatch.");
        }

        const unsigned char *d_ptr{reinterpret_cast<const unsigned char *>(&data[i][0])};
        iof.write(reinterpret_cast<const char*>(d_ptr), dimension * sizeof(double));
        if (iof.fail() | iof.bad()) {
            throw ArchiveError("Could not write in file.");
        }

        for (std::size_t j{0}; j < dimension * sizeof(double); j++)
        {
            int index{(crc_32 ^ d_ptr[j]) & 0xFF};
            crc_32 = (crc_32 >> 8) ^ s_crc_32_tab[index];
        }
    }

    crc_32 ^= 0xFFFFFFFF;

    iof.write(reinterpret_cast<char *>(&crc_32), sizeof(uint32_t));
    if (iof.fail() | iof.bad()) {
        throw ArchiveError("Could not write in file.");
    }
}

std::ostream& operator<<(std::ostream& out, const VectorArchive::FileInfo& f) {
    out << "Bytes: " << f.bytes << '\n';
    out << "Dimension: " << f.dim << '\n';
    out << "Count: " << f.count << '\n';
    return out;
}