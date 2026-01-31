# [crc-32 implementation](https://web.archive.org/web/20061011040706/http://c.snippets.org/code/crc_32.c)
# [binary file i/o](https://gist.github.com/molpopgen/9123133)

1. For crc calculation, we shouldnt do this
    for (int j{0}; j < dimension; j++) {//we need crc every byte(ie, dimension*sizeof(double))
                int index{(crc_32 ^ d_ptr[j]) & 0xFF};
                crc_32 = (crc_32 >> 8) ^ crc_32_tab[index];
            }
    as this only takes 128 bytes in consideration, instead we have, 128 * 8 bytes in consideration => soln => dimension * sizeof(double);

2. 