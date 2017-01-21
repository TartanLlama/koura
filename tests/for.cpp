#include <string>
#include "catch.hpp"
#include "koura/engine.hpp"
using namespace koura;
using namespace std::string_literals;

TEST_CASE("for", "[for]") {
    koura::engine engine{};
    koura::context ctx{};
    ctx.add_entity("names", koura::sequence_t{koura::text_t{"alice"}, koura::text_t{"bob"}, koura::text_t{"carol"}});
    std::stringstream out;

    std::stringstream ss {"{%for name in names%}Hello {{name}}\n\n{%endfor%}"s};
    engine.render(ss, out, ctx);
    REQUIRE( out.str() == "Hello alice\nHello bob\nHello carol\n" );
}
