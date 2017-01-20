#include <string>
#include <sstream>
#include "koura/engine.hpp"
using namespace koura;

int main() {
koura::engine engine{};
    koura::context ctx{};
    ctx.add_entity("what", "world");
    std::stringstream out;

    std::stringstream ss {std::string{"Hello {{what|capitalize}}"}};
    engine.render(ss, out, ctx);
    std::cout << out.str();
}

