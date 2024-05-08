#include "includes/asm.hpp"
#include "codegen/emit.hpp"

#define VERSION "Masm- v0.0.0-alpha.2a"

static std::string version_message = "Masm: An Assembler for the Merry Virtual Machine.\n"
                                     "Latest version: ";

void _(std::string, std::string);

int main(int argv, char **argc)
{
    masm::Masm _asm(argv, argc);
    std::pair<bool, std::string> output_filename;
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
    auto inp = _asm.get_option("input");
    if (!inp.first)
    {
        std::cerr << "Error: No input file provided\n";
        _asm.print_help();
        return -1;
    }
    output_filename = _asm.get_option("output");
    _(inp.second, output_filename.first ? output_filename.second : "");
}

void _(std::string _inp_file_name_, std::string _out_put_file_name_)
{
    // prepare for code generation
    masm::prep::Prep prep(_inp_file_name_);
    prep.prep(); // preprocess
    // Setup Lexer
    masm::lexer::Lexer lexer(&prep);
    // setup Parser
    masm::parser::Parser parser(&lexer);
    parser.parse(); // parse
    // prepare semantic analysis
    masm::sema::Sema sema;
    sema.setup(parser);
    sema.analyse();
    masm::codegen::Codegen gen(sema);
    gen.gen(); // generate
    masm::emit::Emit emit(gen);
    emit.set_output_filename(_out_put_file_name_);
    emit.emit();
}