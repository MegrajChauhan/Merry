#include "includes/asm.hpp"
#include "codegen/emit.hpp"

#define VERSION "Masm- v0.0.0-alpha.1a"

static std::string version_message = "Masm: An Assembler for the Merry Virtual Machine.\n"
                                     "Latest version: ";

int main(int argv, char **argc)
{
    masm::Masm _asm(argv, argc);
    // masm::Masm _asm(4, x);
    std::pair<bool, std::string> output_filename;
    masm::parser::Parser parser;

    _asm.parse_args();
    if (_asm.get_option("help").first)
    {
        _asm.print_help();
        return 0;
    }
    else if (_asm.get_option("version").first)
    {
        std::cout << version_message + VERSION << std::endl;
        return 0;
    }
    if (!_asm.get_option("input").first)
    {
        std::cerr << "Error: No input file provided\n";
        _asm.print_help();
        return -1;
    }
    output_filename = _asm.get_option("output");

    parser.setup_lexer(_asm.get_option("input").second);
    masm::sema::Sema sema(parser);
    masm::codegen::Codegen codegen(sema);
    masm::emit::Emit emit(codegen);
    emit.set_output_filename(output_filename.first ? output_filename.second : "");
    emit.emit();
}