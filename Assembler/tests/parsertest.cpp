#include "../includes/parser.hpp"

using namespace masm;

int main()
{
    parser::Parser parser;
    parser.setup_lexer("test.masm");
    parser.parse();
    // auto nodes = parser.get_nodes();
    std::vector<std::unique_ptr<nodes::Node>> nodes;
    parser.move_nodes(nodes);
}