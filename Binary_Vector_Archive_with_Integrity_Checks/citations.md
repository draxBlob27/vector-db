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

11. currently, one thing irking me is, that files upon interuption are left with partial data.
Will go thorugh it some other day.

12. Taking 12 secs to save 1M vectors with 128 dim.

Reserve vector capacity before loading - avoid reallocations
Use binary write() instead of operator<< for floats (if not already)
Write in larger chunks - buffer multiple vectors before write()
Check if you're flushing too often - only flush at end

sanilparmar@Sanils-MacBook-Air build % ./examples/myCLI create large_data.bin 128 1 --random 
0.244958
sanilparmar@Sanils-MacBook-Air build % ./examples/myCLI create large_data.bin 128 10 --random
0.429541
sanilparmar@Sanils-MacBook-Air build % ./examples/myCLI create large_data.bin 128 100 --random
1.44333
sanilparmar@Sanils-MacBook-Air build % ./examples/myCLI create large_data.bin 128 1000 --random
10.0122
sanilparmar@Sanils-MacBook-Air build % ./examples/myCLI create large_data.bin 128 10000 --random
113.9
sanilparmar@Sanils-MacBook-Air build % ./examples/myCLI create large_data.bin 128 100000 --random
743.062
sanilparmar@Sanils-MacBook-Air build % ./examples/myCLI create large_data.bin 128 1000000 --random
7684.73

13. Who own the responsiilgty to flush the buffer, should be handlend by flag, not indexes as they disappear after loop.

591.142 -> copying data to buffer
~1400~ -> buffer + write
~1500 -> no buffer, no crc

6916.85 -> crc + buffer
7684.73 -> direct dump + crc without buffer
7621.97 -> buffer + crc + write

so crc calc is the bottle neck here
