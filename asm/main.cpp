#include "context.hpp"
#include "emit.hpp"

// The format for the VERSION is v<Major>.<Minor>.<Patch>-<State>.<update_count>
#define VERSION "Masm- v0.0.0-test.0"

static std::string version_message = "Zeta: An Assembler for the Merry Virtual Machine.\n"
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
    masm::Context _c;
    masm::Emit _e;
    Masm _asm(argc, argv);
    std::pair<bool, std::string> input_filename, output_filename;
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
    input_filename = _asm.get_option("input");
    _c.init_context(input_filename.second);
    _c.start();
    std::string _output_fname = output_filename.first ? output_filename.second : "a.mbin";
    if (!_output_fname.ends_with(".mbin"))
        _output_fname += ".mbin";
    masm::CodeGen *g = _c.get_codegen();
    _e.emit(_output_fname, _c.get_eepe(), _c.get_teepe(), _c.get_entries(), g->get_code(), g->get_data(), g->get_str_data(), _c.get_lbl_addr());    
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
    std::cout << "   -h, -help --> Display this help\n";
    std::cout << "   -v, --version --> Display the current version\n";
    std::cout << "   -o --> Specify the output file name\n";
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