#pragma once

#include <utility>     // For std::move and std::forward
#include <type_traits> // For std::aligned_storage
#include <stdexcept>   // For std::runtime_error

// Global structure representing the absence of a value, equivalent to std::nullopt
struct NullOpt_t {};
// Global constant NullOpt to indicate the absence of a value, usable for any Optional<T>
constexpr NullOpt_t NullOpt;
//NullOpt_t NullOpt;

template<typename T>
class Optional {
private:
    using StorageType = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
    bool has_value_flag;    // Renamed to avoid conflict
    StorageType storage;

    T* data() {
        return reinterpret_cast<T*>(&storage);
    }

    const T* data() const {
        return reinterpret_cast<const T*>(&storage);
    }

public:
    // Default constructor: creates an empty Optional
    Optional() : has_value_flag(false) {}

    // NullOpt constructor: allows explicit creation of an empty Optional
    Optional(NullOpt_t) : has_value_flag(false) {}

    // Constructor from a value
    Optional(const T& value) : has_value_flag(true) {
        new (&storage) T(value);
    }

    // Move constructor
    Optional(T&& value) : has_value_flag(true) {
        new (&storage) T(std::move(value));
    }

    // Copy constructor
    Optional(const Optional& other) : has_value_flag(other.has_value_flag) {
        if (other.has_value_flag) {
            new (&storage) T(*other.data());
        }
    }

    // Move constructor
    Optional(Optional&& other) noexcept : has_value_flag(other.has_value_flag) {
        if (other.has_value_flag) {
            new (&storage) T(std::move(*other.data()));
            other.has_value_flag = false;
        }
    }

    // Destructor
    ~Optional() {
        reset();
    }

    // Copy assignment operator
    Optional& operator=(const Optional& other) {
        if (this != &other) {
            reset();
            if (other.has_value_flag) {
                new (&storage) T(*other.data());
                has_value_flag = true;
            }
        }
        return *this;
    }

    // Move assignment operator
    Optional& operator=(Optional&& other) noexcept {
        if (this != &other) {
            reset();
            if (other.has_value_flag) {
                new (&storage) T(std::move(*other.data()));
                has_value_flag = true;
                other.has_value_flag = false;
            }
        }
        return *this;
    }

    // Assignment operator with NullOpt
    Optional& operator=(NullOpt_t) noexcept {
        reset();
        return *this;
    }

    // Reset the Optional
    void reset() {
        if (has_value_flag) {
            data()->~T();
            has_value_flag = false;
        }
    }

    // Check if a value is present
    bool has_value() const noexcept {
        return has_value_flag;
    }

    // Access the value (non-const)
    T& value() {
        if (!has_value_flag) {
            throw std::runtime_error("Optional has no value");
        }
        return *data();
    }

    // Access the value (const)
    const T& value() const {
        if (!has_value_flag) {
            throw std::runtime_error("Optional has no value");
        }
        return *data();
    }

    // Boolean operator to check the presence of a value
    explicit operator bool() const noexcept {
        return has_value_flag;
    }

    // Direct access with the * operator
    T& operator*() {
        return value();
    }

    const T& operator*() const {
        return value();
    }

    // Access with the -> operator
    T* operator->() {
        return data();
    }

    const T* operator->() const {
        return data();
    }

    // Method to get a default value if none is present
    T value_or(const T& default_value) const {
        return has_value_flag ? *data() : default_value;
    }

    // == operator overload to compare with NullOpt
    bool operator==(const NullOpt_t&) const noexcept {
        return !has_value_flag; // If the Optional is empty, it is equal to NullOpt
    }

    // != operator overload to compare with NullOpt
    bool operator!=(const NullOpt_t&) const noexcept {
        return has_value_flag; // If the Optional contains a value, it is different from NullOpt
    }
};

// Global use of NullOpt to signify the absence of a value
template <typename T>
Optional<T> make_optional(const T& value) {
    return Optional<T>(value);
}

template <typename T>
Optional<T> make_optional(T&& value) {
    return Optional<T>(std::forward<T>(value));
}