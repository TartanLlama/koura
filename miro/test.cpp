#include <string>
#include <sstream>
#include "miro/engine.hpp"
using namespace miro;

int main() {
miro::engine engine{};
    miro::context ctx{};
    ctx.add_entity("what", "world");
    std::stringstream out;

    std::stringstream ss {std::string{"Hello {{what|capitalize}}"}};
    engine.render(ss, out, ctx);
    std::cout << out.str();
}

