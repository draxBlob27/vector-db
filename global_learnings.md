# SafeVector<T> Implementation Notes

### Rule of Five
- **Move constructor**: Must reset source object's `m_size` and `m_capacity` to 0
- **Copy-and-swap idiom**: Single assignment operator handles both copy and move
- [copy and swap(stack overflow)](https://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom)

### Exception Safety in `push_back`
**Problem**: Updating `m_data`/`m_capacity` before inserting value = corrupted state if assignment throws

**Solution**: 
```
1. Allocate temp_data
2. Move old elements to temp_data
3. Insert new value into temp_data
4. Commit: m_data = std::move(temp_data)
```
Only update state after all risky ops succeed (strong guarantee).

### `push_back` Logic
- **Full capacity**: allocate → move → insert → commit
- **Has capacity**: `m_data[m_size] = value` (don't forget the else block!)
- **Always**: increment `m_size`

### Move Semantics
Two overloads:
- `push_back(const T& value)` - lvalue, copy into vector
- `push_back(T&& value)` - rvalue, move into vector

**Critical**: Use `std::move(value)` inside the rvalue overload. Named rvalue refs are lvalues.

### Move-Only Types
- `const T&` overload won't compile for `std::unique_ptr<int>`, etc.
- `T&&` overload works fine
- Trade-off: Keep both overloads (simple, optimal) vs. perfect forwarding (complex, supports everything)

### Interface Details
- `operator[]`: One argument only, returns `T&` for modification
- Exceptions: Use `std::out_of_range`
- `at()`: Const member returning `const T&` for const-correct read-only access

### Other Bugs
- `clear()`: Only resets size/capacity, doesn't free memory. Need `m_data.reset()`


# IO and Error Handling
1. [crc-32 implementation](https://web.archive.org/web/20061011040706/http://c.snippets.org/code/crc_32.c)
2. [binary file i/o](https://gist.github.com/molpopgen/9123133)

3. For crc calculation, we shouldnt do this
    ```
    for (int j{0}; j < dimension; j++) {//we need crc every byte(ie, dimension*sizeof(double))
        int index{(crc_32 ^ d_ptr[j]) & 0xFF};
        crc_32 = (crc_32 >> 8) ^ crc_32_tab[index];
    }
    ```
    as this only takes 128 bytes in consideration, instead we have, 128 * 8 bytes in consideration => soln => dimension * sizeof(double);

4. In cli_demo.cpp -> argv[1] is a char* (a pointer to a memory address). "create" is a string literal (also a pointer to a static memory address).
What this does: Compares two memory addresses. They will never be equal.

5. ***Can be thought upon later:***
Grouping by argc first (as you did) is efficient, but it makes the code harder to extend.
Why: If you add a generic --verbose flag later, argc changes for every command, breaking your entire logic tree.

6. [Random number genration from learncpp.com](https://www.learncpp.com/cpp-tutorial/generating-random-numbers-using-mersenne-twister/)

7. Currently saving or generating very large data would be meaning to allocate a large amount of data. Either will solve in this iteration by appending in chunks or later upon doing profiling.

8. Yet to implement crc to header info. -> **done**

8. C++ static const member in header needs inline or out-of-class definition in .cpp — otherwise: undefined reference, gives linker error.

9. to implement sanity checks for too much RAM allocation

10. Keeping singgle crc for both data and header, was kinda perfect for integrtiy, but was very expensive
to handle, as i aint using any libraries, so during append, to update crc with new header, i would have to
read all data again each time, wihch was very slow. Soln was to compromise on seucirty a bit, and implement
sepearate crcs for header and data. Thatway perf can be improved.

11. currently, one thing irking me is, that files upon interuption are left with partial data.
Will go thorugh it some other day. -> **kept it as it is**

12. Taking 12 secs to save 1M vectors with 128 dim.

* Reserve vector capacity before loading - avoid reallocations -> **done**
* Use binary write() instead of operator<< for floats (if not already) -> **done**
* Write in larger chunks - buffer multiple vectors before write() -> **done**
* Check if you're flushing too often - only flush at end -> **done**

### Timings:
```
sanilparmar@Sanils-MacBook-Air build % ./examples/myCLI create large_data.bin 128 1 --random 
0.244958
```
```
sanilparmar@Sanils-MacBook-Air build % ./examples/myCLI create large_data.bin 128 10 --random
0.429541
```
```
sanilparmar@Sanils-MacBook-Air build % ./examples/myCLI create large_data.bin 128 100 --random
1.44333
```
```
sanilparmar@Sanils-MacBook-Air build % ./examples/myCLI create large_data.bin 128 1000 --random
10.0122
```
```
sanilparmar@Sanils-MacBook-Air build % ./examples/myCLI create large_data.bin 128 10000 --random
113.9
```
```
sanilparmar@Sanils-MacBook-Air build % ./examples/myCLI create large_data.bin 128 100000 --random
743.062
```
```
sanilparmar@Sanils-MacBook-Air build % ./examples/myCLI create large_data.bin 128 1000000 --random
7684.73
```


13. Who own the responsiilgty to flush the buffer, should be handlend by flag, not indexes as they disappear after loop. -> **done**

``` 
591.142 -> copying data to buffer 
~1400~ -> buffer + write 
~1500 -> no buffer, no crc

6916.85 -> crc + buffer
7684.73 -> direct dump + crc without buffer
7621.97 -> buffer + crc + write
```

so crc calc is the bottle neck here

