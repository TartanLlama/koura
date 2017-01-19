#ifndef MIRO_ENTITY_HPP
#define MIRO_ENTITY_HPP

//C++
#include <utility>
#include <any>

//Miro
#include "types.hpp"

namespace miro {
    class entity {
    public:
        enum class type {
            text, number, object, list
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

        template <class T>
        entity (list_t<T> value) : m_type{type::list}, m_value{std::move(value)} {}        

        auto get_type() -> type { return m_type; }
        
        template <class T>
        auto get_value() -> T& { return std::any_cast<T&>(m_value); }
        
    private:
        type m_type;
        std::any m_value;
    };
}
    
#endif
