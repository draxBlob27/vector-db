#include "archive.h"

void VectorArchive::save(const std::string &file_path, const std::vector<std::vector<double>> &data)
{
    uint32_t crc_32_header{0xFFFFFFFF};
    const uint64_t count{data.size()};
    if (!count) {
        throw InvalidOperationError("No data to store.");
    }

    const uint32_t dimension{static_cast<uint32_t>(data[0].size())};
    if (!dimension) {
        throw InvalidOperationError("Empty vectors.");
    }
    
    std::ofstream outf{file_path, std::ios::binary};
    if (!outf) {
        throw FileNotFoundError("Uh oh, file: " + file_path + " could not be opened for writing!\n");
    }
    
    outf.write(reinterpret_cast<const char*>(&VectorArchive::s_magic_bytes), sizeof(uint32_t));
    if (outf.bad() || outf.fail()) {
        throw InsufficientSpaceError("Insufficient space on disk.");
    }
    update_crc(crc_32_header, &s_magic_bytes, sizeof(s_magic_bytes));
    
    outf.write(reinterpret_cast<const char *>(&VectorArchive::s_version), sizeof(uint32_t));
    if (outf.bad() || outf.fail()) {
        throw InsufficientSpaceError("Insufficient space on disk.");
    }
    update_crc(crc_32_header, &s_version, sizeof(s_version));

    outf.write(reinterpret_cast<const char *>(&dimension), sizeof(uint32_t));
    if (outf.bad() || outf.fail()) {
        throw InsufficientSpaceError("Insufficient space on disk.");
    }
    update_crc(crc_32_header, &dimension, sizeof(dimension));
    
    outf.write(reinterpret_cast<const char *>(&count), sizeof(uint64_t));
    if (outf.bad() || outf.fail()) {
        throw InsufficientSpaceError("Insufficient space on disk.");
    }
    update_crc(crc_32_header, &count, sizeof(count));
    
    crc_32_header ^= 0xFFFFFFFF;
    outf.write(reinterpret_cast<char*>(&crc_32_header), sizeof(crc_32_header));
    if (outf.bad() || outf.fail()) {
        throw InsufficientSpaceError("Insufficient space on disk.");
    }

    uint32_t crc_32_data{0xFFFFFFFF};
    int buff_cnt{32}, staged_data{0};
    std::vector<double> buffer(dimension * buff_cnt);
    for (uint64_t i{0}; i < count; i++)
    {
        if (dimension != data[i].size()) {
            throw InvalidOperationError("Dimension of data mismatch.");
        }
        if (staged_data == buff_cnt) {
            unsigned const char* d_ptr = reinterpret_cast<unsigned const char*>(&buffer[0]);
            outf.write(reinterpret_cast<const char*>(d_ptr), dimension * buff_cnt * sizeof(double));
            if (outf.bad() || outf.fail()) {
                throw InsufficientSpaceError("Insufficient space on disk.");
            }
            
            update_crc(crc_32_data, d_ptr, dimension * buff_cnt * sizeof(double));
            staged_data = 0;
        }

        uint32_t beg{staged_data * dimension};
        for (int j{0}; j < dimension; j++) {
            buffer[beg + j] = data[i][j];
        }

        staged_data++;
    }

    if (staged_data) {
        unsigned const char* d_ptr = reinterpret_cast<unsigned const char*>(&buffer[0]);
        outf.write(reinterpret_cast<const char*>(d_ptr), dimension * staged_data *sizeof(double));
        if (outf.bad() || outf.fail()) {
            throw InsufficientSpaceError("Insufficient space on disk.");
        }

        update_crc(crc_32_data, d_ptr, dimension * staged_data * sizeof(double));
    }

    crc_32_data ^= 0xFFFFFFFF;
    outf.write(reinterpret_cast<char *>(&crc_32_data), sizeof(uint32_t));
    if (outf.bad() || outf.fail()) {
        throw InsufficientSpaceError("Insufficient space on disk.");
    }
}

std::vector<std::vector<double>> VectorArchive::load(const std::string &file_path, bool already_verified)
{
    std::ifstream inf{file_path, std::ios::binary};
    if (!inf) {
        throw FileNotFoundError("Uh oh, file: " + file_path + " could not be opened for reading!\n");
    }

    uint32_t calc_crc_32_header{0xFFFFFFFF};
    uint32_t magic_bytes;
    inf.read(reinterpret_cast<char *>(&magic_bytes), sizeof(uint32_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }
    if (magic_bytes != s_magic_bytes)
        throw CorruptedDataError("Magic bytes mismatch."); // placeholder;
    update_crc(calc_crc_32_header, &magic_bytes, sizeof(magic_bytes));

    uint32_t version;
    inf.read(reinterpret_cast<char *>(&version), sizeof(uint32_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }
    if (version != s_version)
        throw CorruptedDataError("Version mismatch.");
    update_crc(calc_crc_32_header, &version, sizeof(version));

    uint32_t dimension;
    inf.read(reinterpret_cast<char *>(&dimension), sizeof(uint32_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }
    update_crc(calc_crc_32_header, &dimension, sizeof(dimension));

    uint64_t count;
    inf.read(reinterpret_cast<char *>(&count), sizeof(uint64_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }
    update_crc(calc_crc_32_header, &count, sizeof(count));

    uint32_t crc_32_header;
    inf.read(reinterpret_cast<char*>(&crc_32_header), sizeof(crc_32_header));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }

    calc_crc_32_header ^= 0xFFFFFFFF;
    if (crc_32_header != calc_crc_32_header) {
        throw CorruptedDataError("Header CRC mismatch.");
    }

    uint32_t calc_crc_32_data{0xFFFFFFFF};
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
            update_crc(calc_crc_32_data, d_ptr, dimension * sizeof(double));
        }
    }

    if (!already_verified)
        calc_crc_32_data ^= 0xFFFFFFFF;

    uint32_t crc_32_data;
    inf.read(reinterpret_cast<char *>(&crc_32_data), sizeof(uint32_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }

    if (!already_verified && calc_crc_32_data != crc_32_data)
        throw CorruptedDataError("Data CRC mismatch.");

    return data;
}

VectorArchive::FileInfo VectorArchive::info(const std::string &file_path)
{
    FileInfo f{};

    std::ifstream inf{file_path, std::ios::binary};
    if (!inf) {
        throw FileNotFoundError("Uh oh, file: " + file_path + " could not be opened for reading!\n");
    }

    inf.seekg(2 * sizeof(uint32_t)); //magic bytes and version

    inf.read(reinterpret_cast<char *>(&f.dim), sizeof(uint32_t)); //read dimension
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }

    inf.read(reinterpret_cast<char *>(&f.count), sizeof(uint64_t)); //read count
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }

    f.bytes = 3 * sizeof(uint32_t) + sizeof(uint64_t) + f.dim * f.count * sizeof(double) + 2 * sizeof(uint32_t);
    return f;
}

bool VectorArchive::verify(const std::string &file_path)
{
    std::ifstream inf{file_path, std::ios::binary};
    if (!inf) {
        throw FileNotFoundError("Uh oh, file: " + file_path + " could not be opened for reading!\n");
    }

    uint32_t calc_crc_32_header{0xFFFFFFFF};

    uint32_t magic_bytes;
    inf.read(reinterpret_cast<char *>(&magic_bytes), sizeof(uint32_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }
    if (magic_bytes != s_magic_bytes)
    {
        return false;
    }
    update_crc(calc_crc_32_header, &magic_bytes, sizeof(magic_bytes));

    uint32_t version;
    inf.read(reinterpret_cast<char *>(&version), sizeof(uint32_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }
    if (version != s_version)
    {
        return false;
    }
    update_crc(calc_crc_32_header, &version, sizeof(version));

    uint32_t dimension;
    inf.read(reinterpret_cast<char *>(&dimension), sizeof(uint32_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }
    update_crc(calc_crc_32_header, &dimension, sizeof(dimension));

    uint64_t count;
    inf.read(reinterpret_cast<char *>(&count), sizeof(uint64_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }
    update_crc(calc_crc_32_header, &count, sizeof(count));
    calc_crc_32_header ^= 0xFFFFFFFF;

    uint32_t crc_32_header;
    inf.read(reinterpret_cast<char*>(&crc_32_header), sizeof(crc_32_header));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }
    if (crc_32_header != calc_crc_32_header) {
        return false;
    }

    uint32_t calc_crc_32_data{0xFFFFFFFF};
    std::vector<double> data(dimension);

    for (uint64_t i = 0; i < count; i++)
    {
        unsigned char *d_ptr = reinterpret_cast<unsigned char *>(&data[0]);
        inf.read(reinterpret_cast<char*>(d_ptr), dimension * sizeof(double));
        if (inf.fail() | inf.bad()) {
            throw ArchiveError("Could not read file.");
        }

        update_crc(calc_crc_32_data, d_ptr, dimension * sizeof(double));
    }

    calc_crc_32_data ^= 0xFFFFFFFF;

    uint32_t crc_32_data;
    inf.read(reinterpret_cast<char *>(&crc_32_data), sizeof(uint32_t));
    if (inf.fail() | inf.bad()) {
        throw ArchiveError("Could not read file.");
    }

    if (calc_crc_32_data != crc_32_data)
        return false;

    return true;
}

void VectorArchive::append(const std::string &file_path, const std::vector<std::vector<double>> &data)
{
    const uint64_t append_count{data.size()};
    if (!append_count) {
        throw InvalidOperationError("no data to append.");
    }
    const uint32_t append_dim{static_cast<uint32_t>(data[0].size())};

    std::fstream iof{file_path, std::ios::in | std::ios::out | std::ios::binary};
    if (!iof) {
        throw FileNotFoundError("Uh oh, file: " + file_path + " could not be opened for writing!\n");
    }

    uint32_t crc_32_header_update{0xFFFFFFFF};

    uint32_t magic_bytes;
    iof.read(reinterpret_cast<char *>(&magic_bytes), sizeof(uint32_t));
    if (iof.fail() | iof.bad()) {
        throw ArchiveError("Could not read file.");
    }
    update_crc(crc_32_header_update, &magic_bytes, sizeof(magic_bytes));

    uint32_t version;
    iof.read(reinterpret_cast<char *>(&version), sizeof(uint32_t));
    if (iof.fail() | iof.bad()) {
        throw ArchiveError("Could not read file.");
    }
    update_crc(crc_32_header_update, &version, sizeof(version));

    uint32_t dimension;
    iof.read(reinterpret_cast<char *>(&dimension), sizeof(uint32_t)); //read dimension
    if (iof.fail() | iof.bad()) {
        throw ArchiveError("Could not read file.");
    }
    if (dimension != append_dim)
    {
        throw InvalidOperationError("New data dimension mismatch.");
    }
    update_crc(crc_32_header_update, &dimension, sizeof(dimension));

    uint64_t count;
    iof.read(reinterpret_cast<char *>(&count), sizeof(uint64_t)); //read count
    if (iof.fail() | iof.bad()) {
        throw ArchiveError("Could not read file.");
    }
    iof.seekg(-sizeof(uint64_t), std::ios::cur);
    const uint64_t new_count{count + append_count};
    iof.write(reinterpret_cast<const char *>(&new_count), sizeof(uint64_t));
    if (iof.fail() | iof.bad()) {
        throw ArchiveError("Could not write in file.");
    }
    update_crc(crc_32_header_update, &new_count, sizeof(new_count));

    crc_32_header_update ^= 0xFFFFFFFF;

    iof.write(reinterpret_cast<char*>(&crc_32_header_update), sizeof(crc_32_header_update));
    if (iof.fail() | iof.bad()) {
        throw ArchiveError("Could not write in file.");
    }

    iof.seekg(dimension * count * sizeof(double), std::ios::cur); // data

    uint32_t crc_32_data_update;
    iof.read(reinterpret_cast<char *>(&crc_32_data_update), sizeof(uint32_t));
    if (iof.fail() | iof.bad()) {
        throw ArchiveError("Could not read file.");
    }

    crc_32_data_update ^= 0xFFFFFFFF; // undoing previos xor with ~0 before saving

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

        update_crc(crc_32_data_update, d_ptr, dimension * sizeof(double));
    }

    crc_32_data_update ^= 0xFFFFFFFF;

    iof.write(reinterpret_cast<char *>(&crc_32_data_update), sizeof(uint32_t));
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