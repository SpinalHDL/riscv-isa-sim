#pragma once

#include <utility>     // Pour std::move et std::forward
#include <type_traits> // Pour std::aligned_storage
#include <stdexcept>   // Pour std::runtime_error

// Structure globale représentant l'absence de valeur, équivalent de std::nullopt
struct NullOpt_t {};
// Constante globale NullOpt pour indiquer l'absence de valeur, utilisable pour tous les Optional<T>
constexpr NullOpt_t NullOpt;
//NullOpt_t NullOpt;

template<typename T>
class Optional {
private:
    using StorageType = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
    bool has_value_flag;    // Renommé pour éviter le conflit
    StorageType storage;

    T* data() {
        return reinterpret_cast<T*>(&storage);
    }

    const T* data() const {
        return reinterpret_cast<const T*>(&storage);
    }

public:
    // Constructeur par défaut : crée un Optional vide
    Optional() : has_value_flag(false) {}

    // Constructeur NullOpt : permet la création d'un Optional vide explicitement
    Optional(NullOpt_t) : has_value_flag(false) {}

    // Constructeur à partir d'une valeur
    Optional(const T& value) : has_value_flag(true) {
        new (&storage) T(value);
    }

    // Constructeur par déplacement
    Optional(T&& value) : has_value_flag(true) {
        new (&storage) T(std::move(value));
    }

    // Constructeur de copie
    Optional(const Optional& other) : has_value_flag(other.has_value_flag) {
        if (other.has_value_flag) {
            new (&storage) T(*other.data());
        }
    }

    // Constructeur de déplacement
    Optional(Optional&& other) noexcept : has_value_flag(other.has_value_flag) {
        if (other.has_value_flag) {
            new (&storage) T(std::move(*other.data()));
            other.has_value_flag = false;
        }
    }

    // Destructeur
    ~Optional() {
        reset();
    }

    // Opérateur d'affectation par copie
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

    // Opérateur d'affectation par déplacement
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

    // Opérateur d'affectation avec NullOpt
    Optional& operator=(NullOpt_t) noexcept {
        reset();
        return *this;
    }

    // Réinitialiser l'Optional
    void reset() {
        if (has_value_flag) {
            data()->~T();
            has_value_flag = false;
        }
    }

    // Vérifie si une valeur est présente
    bool has_value() const noexcept {
        return has_value_flag;
    }

    // Accéder à la valeur (non-const)
    T& value() {
        if (!has_value_flag) {
            throw std::runtime_error("Optional has no value");
        }
        return *data();
    }

    // Accéder à la valeur (const)
    const T& value() const {
        if (!has_value_flag) {
            throw std::runtime_error("Optional has no value");
        }
        return *data();
    }

    // Opérateur booléen pour vérifier la présence de la valeur
    explicit operator bool() const noexcept {
        return has_value_flag;
    }

    // Accès direct avec l'opérateur *
    T& operator*() {
        return value();
    }

    const T& operator*() const {
        return value();
    }

    // Accès avec l'opérateur ->
    T* operator->() {
        return data();
    }

    const T* operator->() const {
        return data();
    }

    // Méthode pour obtenir une valeur par défaut si absent
    T value_or(const T& default_value) const {
        return has_value_flag ? *data() : default_value;
    }
};

// Utilisation globale de NullOpt pour signifier l'absence de valeur
template <typename T>
Optional<T> make_optional(const T& value) {
    return Optional<T>(value);
}

template <typename T>
Optional<T> make_optional(T&& value) {
    return Optional<T>(std::forward<T>(value));
}