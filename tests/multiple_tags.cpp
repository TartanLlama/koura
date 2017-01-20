#include <string>
#include "catch.hpp"
#include "miro/engine.hpp"
using namespace miro;
using namespace std::string_literals;

TEST_CASE("multiple tags", "[multiple]") {
    miro::engine engine{};
    miro::context ctx{};
    ctx.add_entity("what", "world");
    ctx.add_entity("mod", "!");
    std::stringstream out;

    std::stringstream ss {"Hello {{what}}. Yes, {{what}}{{mod}}"s};
    engine.render(ss, out, ctx);
    REQUIRE( out.str() == "Hello world. Yes, world!" );
}

