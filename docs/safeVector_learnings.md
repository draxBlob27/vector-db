# SafeVector<T> Implementation Notes

## Rule of Five
- **Move constructor**: Must reset source object's `m_size` and `m_capacity` to 0
- **Copy-and-swap idiom**: Single assignment operator handles both copy and move

## Exception Safety in `push_back`
**Problem**: Updating `m_data`/`m_capacity` before inserting value = corrupted state if assignment throws

**Solution**: 
```
1. Allocate temp_data
2. Move old elements to temp_data
3. Insert new value into temp_data
4. Commit: m_data = std::move(temp_data)
```
Only update state after all risky ops succeed (strong guarantee).

## `push_back` Logic
- **Full capacity**: allocate → move → insert → commit
- **Has capacity**: `m_data[m_size] = value` (don't forget the else block!)
- **Always**: increment `m_size`

## Move Semantics
Two overloads:
- `push_back(const T& value)` - lvalue, copy into vector
- `push_back(T&& value)` - rvalue, move into vector

**Critical**: Use `std::move(value)` inside the rvalue overload. Named rvalue refs are lvalues.

## Move-Only Types
- `const T&` overload won't compile for `std::unique_ptr<int>`, etc.
- `T&&` overload works fine
- Trade-off: Keep both overloads (simple, optimal) vs. perfect forwarding (complex, supports everything)

## Interface Details
- `operator[]`: One argument only, returns `T&` for modification
- Exceptions: Use `std::out_of_range`
- `at()`: Const member returning `const T&` for const-correct read-only access

## Other Bugs
- `clear()`: Only resets size/capacity, doesn't free memory. Need `m_data.reset()`