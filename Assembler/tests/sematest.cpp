#include "../includes/sema.hpp"

using namespace masm;

int main()
{
    parser::Parser parser;
    parser.setup_lexer("test.masm");
    // parser.parse();
    sema::Sema sema(parser);
    sema.set_path(parser.get_path());
    sema.analyse();
}