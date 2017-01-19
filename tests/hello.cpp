#include <string>
#include "catch.hpp"
#include "miro/engine.hpp"
using namespace miro;
using namespace std::string_literals;

TEST_CASE("hello world", "[hello]") {
    miro::engine engine{};
    miro::context ctx{};
    ctx.add_entity("what", "world");
    std::stringstream out;

    std::stringstream ss {"Hello {{what}}"s};
    engine.render(ss, out, ctx);
    REQUIRE( out.str() == "Hello world" );
}

