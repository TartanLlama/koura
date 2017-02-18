#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <string>
#include "koura.hpp"
using namespace koura;
using namespace std::string_literals;

TEST_CASE("built-in filters", "[builtin_filters]") {
    koura::engine engine{};
    koura::context ctx{};
    ctx.add_entity("what", "world");
    std::stringstream out;

    SECTION ("capitalize") {
        std::stringstream ss {"Hello {{what|capitalise}}"s};
        engine.render(ss, out, ctx);
        REQUIRE( out.str() == "Hello WORLD" );
    }
}

std::string change_to_cheese(std::string_view text, koura::context&) {
    return "cheese";
}

TEST_CASE("custom filters", "[custom_filters]") {
    koura::engine engine{};
    koura::context ctx{};
    ctx.add_entity("what", "world");
    std::stringstream out;

    SECTION ("cheese") {
        engine.register_custom_filter("change_to_cheese", change_to_cheese);
        std::stringstream ss {"Hello {{what|change_to_cheese}}"s};
        engine.render(ss, out, ctx);
        REQUIRE( out.str() == "Hello cheese" );
    }
}

TEST_CASE("for", "[for]") {
    koura::engine engine{};
    koura::context ctx{};
    ctx.add_entity("names", koura::sequence_t{koura::text_t{"alice"}, koura::text_t{"bob"}, koura::text_t{"carol"}});
    std::stringstream out;

    std::stringstream ss {"{%for name in names%}Hello {{name}}\n{%endfor%}"s};
    engine.render(ss, out, ctx);
    REQUIRE( out.str() == "Hello alice\nHello bob\nHello carol\n" );
}

TEST_CASE("hello world", "[hello]") {
    koura::engine engine{};
    koura::context ctx{};
    ctx.add_entity("what", "world");
    std::stringstream out;

    std::stringstream ss {"Hello {{what}}"s};
    engine.render(ss, out, ctx);
    REQUIRE( out.str() == "Hello world" );
}

TEST_CASE("set tags", "[set]") {
    koura::engine engine{};
    koura::context ctx{};
    ctx.add_entity("what", "world");
    std::stringstream out;

    std::stringstream ss {"{% set what 'jim' %}\nHello {{what}}"s};
    engine.render(ss, out, ctx);
    REQUIRE( out.str() == "Hello jim" );
}

TEST_CASE("object access", "[object-access]") {
    koura::engine engine{};
    koura::context ctx{};
    koura::object_t what;
    std::stringstream out;

    SECTION ("access text") {
        what["name"] = koura::text_t{"world"};
        ctx.add_entity("what", what);
        std::stringstream ss {"Hello {{what.name}}"s};
        engine.render(ss, out, ctx);
        REQUIRE( out.str() == "Hello world" );
    }

    SECTION ("access number") {
        what["num"] = koura::number_t{42};
        ctx.add_entity("what", what);
        std::stringstream ss {"Hello {{what.num}}"s};
        engine.render(ss, out, ctx);
        REQUIRE( out.str() == "Hello 42" );
    }

    SECTION ("access object") {
        koura::object_t huh;
        huh["name"] = koura::text_t{"world"};

        what["huh"] = huh;
        ctx.add_entity("what", what);

        std::stringstream ss {"Hello {{what.huh.name}}"s};
        engine.render(ss, out, ctx);
        REQUIRE( out.str() == "Hello world" );
    }
}

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

TEST_CASE("if", "[if]") {
    koura::engine engine{};
    koura::context ctx{};
    ctx.add_entity("name", koura::text_t{"jim"});
    std::stringstream out;

    SECTION ("simple if") {
        std::stringstream ss {"{% if name %}\n{{name}}\n{% else %}\nlol\n{% endif %}"};
        engine.render(ss, out, ctx);
        REQUIRE( out.str() == "jim" );
    }
    SECTION ("simple else") {
        std::stringstream ss {"{% if dennis %}\n{{name}}\n{% else %}\nlol\n{% endif %}"};
        engine.render(ss, out, ctx);
        REQUIRE( out.str() == "lol" );
    }
}
