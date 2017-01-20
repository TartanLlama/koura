#include <string>
#include "catch.hpp"
#include "koura/engine.hpp"
using namespace koura;
using namespace std::string_literals;

std::string change_to_cheese(std::string_view text, koura::context&) {
    return "cheese";
}

TEST_CASE("custom filters", "[custom_filters]") {
    koura::engine engine{};
    koura::context ctx{};
    ctx.add_entity("what", "world");
    std::stringstream out;

    SECTION ("cheese") {
        engine.register_custom_filter("change_to_cheese", change_to_cheese);
        std::stringstream ss {"Hello {{what|change_to_cheese}}"s};
        engine.render(ss, out, ctx);
        REQUIRE( out.str() == "Hello cheese" );
    }
}


