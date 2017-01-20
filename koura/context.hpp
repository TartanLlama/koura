#ifndef KOURA_CONTEXT_HPP
#define KOURA_CONTEXT_HPP

//C++
#include <utility>
#include <unordered_map>
#include <string_view>

//Koura
#include "entity.hpp"
#include "types.hpp"

namespace koura {
    class context {
    public:
        template <class T>
        void add_entity (std::string_view key, T&& value) {
            m_entities.emplace(key, entity{std::forward<T>(value)});
        }
        
        auto get_entity(const std::string& key) -> entity& { return m_entities.at(key); }
            
    private:
        std::unordered_map<std::string, entity> m_entities;
    };
}

#endif
