#include <iostream>
#include <iomanip>
#include <vector>
#include "../includes/clopts.hh"
#include "../includes/core.hh"
#include "../includes/token.hh"
#include "../includes/lexer.hh"
#include "../includes/parser.hh"
#include "../includes/sema.hh"
#include "../includes/ir.hh"
#include "../includes/codegen.hh"

using namespace command_line_options;

using options = clopts<
    flag<"--ast", "Prints the AST">,
    flag<"--ir", "Prints the IR">,
    option<"-f", "What format to emit code in (default: merry)",
        values<
            "merry",
            "asm",
            "elf"
        >
    >,
    positional<"file", "Path to files that should be compiled", file<>, true>,
    help<>
>;

int main(int argc, char **argv){
    auto opts = options::parse(argc, argv);

    auto file_name = opts.get<"file">()->path;
    auto file_contents = opts.get<"file">()->contents;
    file_contents += "\n";

    bool print_ast = opts.get<"--ast">();
    bool print_ir = opts.get<"--ir">();

    auto format = merry::core::Format::merry;
    if (auto format_string = opts.get<"-f">()) {
        if (*format_string == "asm") {
            format = merry::core::Format::assembly;
        } else if (*format_string == "elf") {
            format = merry::core::Format::elf_object;
        } else if (*format_string == "merry") {
            format = merry::core::Format::merry;
        }
    }

    merry::front_end::Lexer lexer(file_contents, file_name);
    merry::front_end::Parser parser(lexer);
    merry::front_end::Ast ast = parser.get_ast();
    merry::front_end::Sema sema(ast);
    ast = sema.resolve_ast();
    if(print_ast) ast.print();
    merry::back_end::IrGen irgen(ast);
    std::vector<merry::back_end::IrInst> insts = irgen.get_insts();
    if(print_ir){
        int idx = 0;
        for(merry::back_end::IrInst inst : insts){
            std::cout << idx << ": Type: " << std::hex << (int)inst.get_type()<< std::dec << std::endl;
            std::cout << "  ";
            for(uint8_t op : inst.get_operands()){
                std::cout << std::to_string(op) << " ";
            }
            std::cout << "\n";
            idx++;
        }
    }
    merry::code_gen::Gen gen;
    gen.generate(format, insts);
}