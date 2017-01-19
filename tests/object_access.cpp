#include <string>
#include "catch.hpp"
#include "miro/engine.hpp"
using namespace miro;

TEST_CASE("object access", "[object-access]") {
    miro::engine engine{};
    miro::context ctx{};
    miro::object_t what;
    what["name"] = miro::text_t{"world"};
    ctx.add_entity("what", what);
    std::stringstream out;

    std::stringstream ss = "Hello {{what.name}}";
    engine.render(ss, out, ctx);
    REQUIRE( out.str() == "Hello world" );
}

