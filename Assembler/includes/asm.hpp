#ifndef _ASM_
#define _ASM_

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>

namespace masm
{
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
};

#endif