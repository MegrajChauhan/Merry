#ifndef _COMPUNIT_
#define _COMPUNIT_

#include <string>
#include <vector>
#include <unordered_map>
#include "parser.hpp"

namespace masm
{
    class CompUnit
    {
        std::string fname;
        Parser p;
        CompUnit *parent = nullptr;

    public:
        CompUnit() = default;

        void add_parent(CompUnit *u);

        void set_filename(std::string name);

        bool do_comp();
    };

    enum DataType
    {
    };

    struct Variable
    {
        std::string value;
        DataType type;
    };

};

#endif