#include "../includes/parser.hpp"

using namespace masm;

int main()
{
    parser::Parser parser;
    parser.setup_lexer("test.masm");
    parser.parse();
    auto nodes = parser.get_nodes();
}