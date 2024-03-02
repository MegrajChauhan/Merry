#include "../codegen/emit.hpp"

using namespace masm;

int main()
{
    masm::parser::Parser parser;
    parser.setup_lexer("test.masm");
    masm::sema::Sema sema(parser);
    masm::codegen::Codegen codegen(sema);
    emit::Emit emit(codegen);
    emit.emit();
}