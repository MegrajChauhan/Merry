#include "../codegen/emit.hpp"

using namespace masm;

int main()
{
    masm::parser::Parser parser;
    parser.setup_lexer("../../example/run/helloWorld.masm");
    masm::sema::Sema sema(parser);
    masm::codegen::Codegen codegen(sema);
    emit::Emit emit(codegen);
    emit.set_output_filename("a.mbin");
    emit.emit();
}