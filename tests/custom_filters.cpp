#include <string>
#include "catch.hpp"
#include "miro/engine.hpp"
using namespace miro;
using namespace std::string_literals;

std::string change_to_cheese(std::string_view text, miro::context&) {
    return "cheese";
}

TEST_CASE("custom filters", "[custom_filters]") {
    miro::engine engine{};
    miro::context ctx{};
    ctx.add_entity("what", "world");
    std::stringstream out;

    SECTION ("cheese") {
        engine.register_custom_filter("change_to_cheese", change_to_cheese);
        std::stringstream ss {"Hello {{what|change_to_cheese}}"s};
        engine.render(ss, out, ctx);
        REQUIRE( out.str() == "Hello cheese" );
    }
}


