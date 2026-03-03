#include "Importer.hpp"
#include "LSH_index.hpp"

int main() {
    LSHIndex lsh(5, (1 << 10));

    Importer::import_sift1m()
}