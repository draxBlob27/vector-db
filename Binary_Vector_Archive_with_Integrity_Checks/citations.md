1. [crc-32 implementation](https://web.archive.org/web/20061011040706/http://c.snippets.org/code/crc_32.c)
2. [binary file i/o](https://gist.github.com/molpopgen/9123133)

3. For crc calculation, we shouldnt do this
    for (int j{0}; j < dimension; j++) {//we need crc every byte(ie, dimension*sizeof(double))
                int index{(crc_32 ^ d_ptr[j]) & 0xFF};
                crc_32 = (crc_32 >> 8) ^ crc_32_tab[index];
            }
    as this only takes 128 bytes in consideration, instead we have, 128 * 8 bytes in consideration => soln => dimension * sizeof(double);

3. argv[1] is a char* (a pointer to a memory address). "create" is a string     literal (also a pointer to a static memory address).
What this does: Compares two memory addresses. They will never be equal.

4. Grouping by argc first (as you did) is efficient, but it makes the code harder to extend.
Why: If you add a generic --verbose flag later, argc changes for every command, breaking your entire logic tree.

5. [random number genration](https://www.learncpp.com/cpp-tutorial/generating-random-numbers-using-mersenne-twister/)

6. Currently saving or generating very large data would be meaning to allocate a large amount of data. Either will solve in this iteration by appending in chunks or later upon doin profiling.

7. Yet to implement crc to header info.

8. C++ static const member in header needs inline or out-of-class definition in .cpp — otherwise: undefined reference, gives linker error.

9. to implement sanity checks for too much RAM allocation

10. Keeping singgle crc for both data and header, was kinda perfect for integrtiy, but was very expensiv
to handle, as i aint using any libraries, so during append, to update crc with new header, i would have to
read all data again each time, wihch was very slow. Soln was to compromise on seucirty a bit, and implement
sepearate crcs for header and data. Thatway perf can be improved.
 