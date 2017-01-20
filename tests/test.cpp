#include <string>
#include <sstream>
#include "koura/engine.hpp"
using namespace koura;
using namespace std::string_literals;

std::string change_to_beer(std::string_view s, koura::context&) {
    return "beer";
}

int main() {
    koura::engine engine{};
    koura::context ctx{};
    ctx.add_entity("what", "world");
    ctx.add_entity("mod", "!");
    std::stringstream out;

    std::stringstream ss {"Hello {{what}}. Yes, {{what}}{{mod}}"s};
    engine.render(ss, out, ctx);
}


