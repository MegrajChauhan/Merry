/*
  An issue: Due to the way the assembler works, right now, it assumes every host
  to be little endian and I am too lazy to fix that(Imagine going to dozens of files and thousands of lines to find some missing lines, augh!!!)
*/
#include "worker.hpp"
#include "codegen.hpp"
#include "emit.hpp"

// The format for the VERSION is v<Major>.<Minor>.<Patch>-<State>.<update_count>
#define VERSION "Masm- v0.1.2-test.14"

static std::string version_message = "Masm: An Assembler for the Merry Virtual Machine.\n"
                                     "Latest version: ";

class Masm
{
public:
    Masm(int argc, char **argv);

    void print_help();

    void parse_args();

    std::pair<bool, std::string> get_option(std::string);

private:
    std::unordered_map<std::string, std::string> options;
    int argc;
    std::vector<std::string> given_options;
};

int main(int argc, char **argv)
{
    masm::Parser p;
    masm::Emit _e;
    // int a = 2;
    // char *v[] = {"dfds", "../stdtest/stdalloctest.masm"};
    Masm _asm(argc, argv);
    std::pair<bool, std::string> input_filename, output_filename;
    bool ed = false, dst = false, cd = false, cdf = false;
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
    ed = _asm.get_option("enable_debugging").first;
    dst = !_asm.get_option("disable_st").first;
    cd = _asm.get_option("child_debug").first;
    cdf = _asm.get_option("child_debug_wait").first;
    output_filename = _asm.get_option("output");
    input_filename = _asm.get_option("input");
    p.setup_parser(input_filename.second);
    p.parse();
    std::string _output_fname = output_filename.first ? output_filename.second : "a.mbin";
    if (!_output_fname.ends_with(".mbin"))
        _output_fname += ".mbin";
    masm::CodeGen c;
    p.add_for_codegen(&c);
    p.parser_confirm_info();
    c.gen();
    if (dst && ed)
        c.gen_ST();
    p.add_for_emit(&_e);
    c.setup_emit(&_e);
    _e.set_for_debug(ed, dst && ed, cd, cdf);
    _e.emit(_output_fname);
    return 0;
}

Masm::Masm(int argc, char **argv)
{
    this->argc = argc - 1;
    for (size_t i = 1; i < argc; i++)
    {
        given_options.push_back(argv[i]);
    }
    if (argc < 2)
    {
        std::cerr << "Error: not enough arguments\n";
        print_help();
        exit(EXIT_FAILURE);
    }
}

void Masm::print_help()
{
    std::cout << "Usage: masm <path_to_file> [OPTIONS]<Additional Arguments>\n";
    std::cout << "OPTIONS INCLUDE:\n";
    std::cout << "   -h, -help    --> Display this help\n";
    std::cout << "   -v,--version --> Display the current version\n";
    std::cout << "   -o           --> Specify the output file name";
    std::cout << "   -ed          --> Enable Debugging";
    std::cout << "   -dst         --> Disable Symbol Table and hence SYMD from being generated";
    std::cout << "   -cd          --> Enable Child Debug thus allowing child processes to be debugged as well.";
    std::cout << "   -cdw         --> Enable Child Debug Freeze that works like --freeze but for child processes\n";
}

void Masm::parse_args()
{
    for (size_t i = 0; i < argc; i++)
    {
        if (given_options[i] == "-h" || given_options[i] == "-help")
        {
            options["help"] = "";
            break;
        }
        else if (given_options[i] == "-v" || given_options[i] == "-version")
        {
            options["version"] = "";
            break;
        }
        else if (given_options[i] == "-o")
        {
            if ((i + 1) >= given_options.size())
            {
                std::cerr << "Expected output file name after -o\n";
                print_help();
                exit(EXIT_FAILURE);
            }
            options["output"] = given_options[i + 1];
            i++;
        }
        else if (given_options[i] == "-ed")
            options["enable_debugging"] = "";
        else if (given_options[i] == "-dst")
            options["disable_st"] = "";
        else if (given_options[i] == "-cd")
            options["child_debug"] = "";
        else if (given_options[i] == "-cdw")
            options["child_debug_wait"] = "";
        else
        {
            // it is a file name
            options["input"] = given_options[i];
        }
    }
}

std::pair<bool, std::string> Masm::get_option(std::string opt_name)
{
    auto x = options.find(opt_name);
    if (x != options.end())
        return std::make_pair(true, x->second);
    return std::make_pair(false, "");
}