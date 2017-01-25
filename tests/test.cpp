#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include "koura.hpp"
using namespace koura;
using namespace std::string_literals;

int main() {
    koura::engine engine{};
    koura::context ctx{};
    koura::object_t what;
    what["name"] = koura::text_t{"world"};
    ctx.add_entity("what", what);
    std::stringstream out;

    std::stringstream ss {"Hello {{what.name}}"s};
    engine.render(ss, std::cout, ctx);
}
