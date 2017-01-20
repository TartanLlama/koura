#include <string>
#include "catch.hpp"
#include "koura/engine.hpp"
using namespace koura;
using namespace std::string_literals;

TEST_CASE("set tags", "[set]") {
    koura::engine engine{};
    koura::context ctx{};
    ctx.add_entity("what", "world");
    std::stringstream out;

    std::stringstream ss {"{% set what 'jim' %}\nHello {{what}}"s};
    engine.render(ss, out, ctx);
    REQUIRE( out.str() == "Hello jim" );
}

