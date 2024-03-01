#include "../codegen/codegen.hpp"

int main()
{
    masm::parser::Parser parser;
    parser.setup_lexer("test.masm");
    masm::sema::Sema sema(parser);
    masm::codegen::Codegen codegen(sema);
    codegen.gen();
}