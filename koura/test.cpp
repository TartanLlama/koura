#include <string>
#include <sstream>
#include "koura/engine.hpp"
using namespace koura;

int main() {
    koura::engine engine{};
    koura::context ctx{};
    ctx.add_entity("names", sequence_t{koura::entity_t{"Quinn"}, koura::entity_t{"Dada"}, koura::entity_t{"Mama"}});
    std::ifstream file {"test"};
    engine.render(file, std::cout, ctx);
}
