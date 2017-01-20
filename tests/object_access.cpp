#include <string>
#include "catch.hpp"
#include "koura/engine.hpp"
using namespace koura;

TEST_CASE("object access", "[object-access]") {
    koura::engine engine{};
    koura::context ctx{};
    koura::object_t what;
    what["name"] = koura::text_t{"world"};
    ctx.add_entity("what", what);
    std::stringstream out;

    std::stringstream ss = "Hello {{what.name}}";
    engine.render(ss, out, ctx);
    REQUIRE( out.str() == "Hello world" );
}

