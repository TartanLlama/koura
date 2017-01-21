#ifndef KOURA_ENTITY_HPP
#define KOURA_ENTITY_HPP

//C++
#include <utility>
#include <any>

//Koura
#include "types.hpp"

namespace koura {
    class entity {
    public:
        enum class type {
            text, number, object, sequence
        };

        entity() = default;
        entity(const entity&) = default;
        entity(entity&&) = default;
        entity& operator=(const entity&) = default;
        entity& operator=(entity&&) = default;
        ~entity() = default;

        entity (text_t value) : m_type{type::text}, m_value{std::move(value)} {}
        entity (number_t value) : m_type{type::number}, m_value{std::move(value)} {}
        entity (object_t value) : m_type{type::object}, m_value{std::move(value)} {}
        entity (sequence_t value) : m_type{type::sequence}, m_value{std::move(value)} {}

        auto get_type() -> type { return m_type; }

        template <class T>
        auto get_value() -> T& { return std::any_cast<T&>(m_value); }

    private:
        type m_type;
        std::any m_value;
    };
}

#endif
