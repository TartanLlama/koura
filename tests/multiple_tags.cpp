#include <string>
#include "catch.hpp"
#include "koura/engine.hpp"
using namespace koura;
using namespace std::string_literals;

TEST_CASE("multiple tags", "[multiple]") {
    koura::engine engine{};
    koura::context ctx{};
    ctx.add_entity("what", "world");
    ctx.add_entity("mod", "!");
    std::stringstream out;

    std::stringstream ss {"Hello {{what}}. Yes, {{what}}{{mod}}"s};
    engine.render(ss, out, ctx);
    REQUIRE( out.str() == "Hello world. Yes, world!" );
}

