#include <string>
#include "catch.hpp"
#include "miro/engine.hpp"
using namespace miro;
using namespace std::string_literals;

TEST_CASE("built-in filters", "[builtin_filters]") {
    miro::engine engine{};
    miro::context ctx{};
    ctx.add_entity("what", "world");
    std::stringstream out;

    SECTION ("capitalize") {
        std::stringstream ss {"Hello {{what|capitalize}}"s};
        engine.render(ss, out, ctx);
        REQUIRE( out.str() == "Hello WORLD" );
    }
}

