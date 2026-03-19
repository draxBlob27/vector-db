#ifndef RESULT_HPP
#define RESULT_HPP
#include <variant>

template <typename T>
class Ok {
    T value;
    
    public:
    
    explicit Ok(T value) //disallows implicit conversion. To not get surprised by compiler implicit conversions
    :value{std::move(value)}
    {}
    
    T copy_value() const {
        return value;
    }
    
    T&& take_value() {
        return std::move(value);
    }
};

template <typename T>
class Err {
    T value;
    
    public:
    explicit Err(T value)
    :value{std::move(value)}
    {}
    
    T copy_value() const {
        return value;
    }
    
    T&& take_value() {
        return std::move(value);
    }
};

template <typename OkT, typename ErrT>
class Result {
    std::variant<Ok<OkT>, Err<ErrT>> variant;
    
    public:
    Result(Ok<OkT> value)
    :variant(std::move(value))
    {}
    
    Result(Err<ErrT> value)
    :variant(std::move(value))
    {}
    
    bool is_ok() const {
        return std::holds_alternative<Ok<OkT>>(variant);
    }
    bool is_err() const {
        return std::holds_alternative<Err<ErrT>>(variant);
    }
    OkT ok_value() const {
        return std::get<Ok<OkT>>(variant).copy_value(); //returns a copy, throws upon wrong call.
    }
    ErrT err_value() const{
        return std::get<Err<ErrT>>(variant).copy_value();
    }
    
    OkT&& take_ok_value() {
        return std::get<Ok<OkT>>(variant).take_value(); //returns ownership, throws upon wrong call, after operation class value is invalid/empty;
    }
    ErrT&& take_err_value() {
        return std::get<Err<ErrT>>(variant).take_value();
    }
};

struct Unit{}; //replaces void return type with empty class type, inspried from article linked.
template<typename ErrT>
class Result<Unit, ErrT> {
    std::variant<Ok<Unit>, Err<ErrT>> variant;
    
    public:
    Result(Ok<Unit> value)
    :variant(std::move(value))
    {}
    
    Result(Err<ErrT> value)
    :variant(std::move(value))
    {}
    
    bool is_ok() const {
        return std::holds_alternative<Ok<Unit>>(variant);
    }
    bool is_err() const {
        return std::holds_alternative<Err<ErrT>>(variant);
    }
    
    Unit ok_value() const {
        return Unit{}; 
    }
    ErrT err_value() const{
        return std::get<Err<ErrT>>(variant).copy_value();
    }
    
    Unit&& take_ok_value() = delete;
    // {
    //     return Unit{};
    // }

    ErrT&& take_err_value() {
        return std::get<Err<ErrT>>(variant).take_value();
    }
};
#endif //RESULT_HPP