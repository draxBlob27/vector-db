#include "NSW_Index.hpp"
#include <shared_mutex>

class ThreadSafe_NSW_Index {
private:
    NSW_Index nsw;
    std::shared_mutex entry_mutex;

public:
};