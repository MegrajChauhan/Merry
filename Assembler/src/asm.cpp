#include "../includes/asm.hpp"

masm::Masm::Masm(int argc, char **argv)
{
    this->argc = argc-1;
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

void masm::Masm::print_help()
{
    std::cout << "Usage: masm <path_to_file> [OPTIONS]<Additional Arguments>\n";
    std::cout << "OPTIONS INCLUDE:\n";
    std::cout << "  -h, -help --> Display this help\n";
    std::cout << "   -v, --version --> Display the current version\n";
    std::cout << "   -o --> Specify the output file name\n";
}

void masm::Masm::parse_args()
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

std::pair<bool, std::string> masm::Masm::get_option(std::string opt_name)
{
    auto x = options.find(opt_name);
    if (x != options.end())
        return std::make_pair(true, x->second);
    return std::make_pair(false, "");
}