#include <string>
#include "catch.hpp"
#include "koura/engine.hpp"
using namespace koura;
using namespace std::string_literals;

TEST_CASE("hello world", "[hello]") {
    koura::engine engine{};
    koura::context ctx{};
    ctx.add_entity("what", "world");
    std::stringstream out;

    std::stringstream ss {"Hello {{what}}"s};
    engine.render(ss, out, ctx);
    REQUIRE( out.str() == "Hello world" );
}

