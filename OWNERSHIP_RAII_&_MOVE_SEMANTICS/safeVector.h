#include <memory>
#include <utility>
#include <exception>

template <typename T>
class SafeVector {
private:
    std::unique_ptr<T[]> m_data{nullptr};
    std::size_t m_size{0};
    std::size_t m_capacity{0};

public:
    SafeVector(int size = 0) //contructor
        :m_size{size}, m_capacity{size}, m_data{m_capacity ? std::make_unique<T[]>(m_capacity) : nullptr}
    {}

    ~SafeVector() {
        m_size = 0;
        m_capacity = 0;
    }

    SafeVector(const SafeVector& a)
        :m_size{a.m_size},
        m_capacity{a.m_capacity},
        m_data{m_capacity ? std::make_unique<T[]>(m_capacity) : nullptr}
    { //copy constructor
        if (m_capacity) {
            std::copy_n(a.m_data.get(), a.m_size, m_data.get());
        }
    }

    SafeVector(SafeVector&& a) noexcept
        :m_capacity{a.m_capacity},
        m_size{a.m_size},
        m_data{std::move(a.m_data)}
    { //move contructor
        a.m_capacity = 0;
        a.m_size = 0;
    }
    
    friend void swap(SafeVector& a, SafeVector& b) {
        using std::swap;
        swap(a.m_capacity, b.m_capacity);
        swap(a.m_size, b.m_size);
        swap(a.m_data, b.m_data);
    }
    
    SafeVector& operator=(SafeVector a) {//copy and move assingment
        //no noexcept as copy const can throw bad_alloc
        swap(a, *this);
        return *this;
    }
    
    void push_back(const T value) { //can use lvalue and rvalue overloading in nxt time
        if (m_size == m_capacity) {
            //alloate new memory
            //move data from current memeory to new memory
            //insert new value
        
            auto temp_data = std::make_unique<T[]>(m_capacity ? 2 * m_capacity : 1);
            std::move(m_data.get(), m_data.get() + m_size, temp_data.get());
            m_data = std::move(temp_data);
            m_capacity = m_capacity ? 2 * m_capacity : 1;
        }
        m_data[m_size] = value;
        m_size++;
    }
    
    void pop_back() {
        if (m_size > 0) {
            m_size--;
            return;
        }
        
        throw("Vector empty");
    }

    T& operator[](const int idx) const {
        if (idx < m_size && m_size >= 0) {
            return m_data[idx];
        }

        throw("Index out of bound");
    }

    std::size_t size() const {
        return m_size;
    }

    std::size_t capacity() const {
        return m_capacity;
    }

    bool empty() const {
        return m_size == 0;
    }

    void resize(const std::size_t new_size) {
        if (new_size > m_capacity) {
            auto temp_data = std::make_unique<T[]>(new_size);
            
            std::move(m_data.get(), m_data.get() + m_size, temp_data.get());
            m_data = std::move(temp_data);
            m_capacity = new_size;
        }

        m_size = new_size;
    }

    void reserve(const std::size_t new_size) {
        if (new_size > m_capacity) {
            auto temp_data = std::make_unique<T[]>(new_size);
            
            std::move(m_data.get(), m_data.get() + m_size, temp_data.get());
            m_data = std::move(temp_data);
            m_capacity = new_size;
        }
    }

    void clear() {
        m_size = 0;
        m_capacity = 0;
    }

    T* begin() {
        return m_data.get();
    }

    T* end() {
        return m_data.get() + m_size;
    }
};