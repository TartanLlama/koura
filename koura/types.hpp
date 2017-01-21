#ifndef KOURA_TYPES_HPP
#define KOURA_TYPES_HPP

#include <string_view>
#include <unordered_map>
#include <vector>
#include <string>

namespace koura {
    class entity;

    using number_t = int;
    using text_t = std::string;
    using object_t = std::unordered_map<std::string, entity>;
    using sequence_t = std::vector<entity>;
}

#endif
