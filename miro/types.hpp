#ifndef MIRO_TYPES_HPP
#define MIRO_TYPES_HPP

#include <string_view>
#include <unordered_map>
#include <vector>
#include <string>

namespace miro {
    class entity;

    using number_t = int;
    using text_t = std::string;
    using object_t = std::unordered_map<std::string, entity>;
        
    template <typename T>
    using list_t = std::vector<T>;
}    

#endif
