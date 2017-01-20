#ifndef KOURA_FILTERS_HPP
#define KOURA_FILTERS_HPP

//C++
#include <string_view>
#include <algorithm>

namespace koura {
    namespace filters {
        inline std::string capitalize (std::string_view text, context&) {
            std::string ret {text};
            for (auto&& c : ret) {
                c = std::toupper(c);
            }
            return ret;
        }
    }
}

#endif
