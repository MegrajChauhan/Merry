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

    public:
        CompUnit() = default;

        void set_filename(std::string name);

        bool do_comp();
    };

    struct Variable
    {
        std::string value;
        DataType type;
    };

    static std::unordered_map<std::string, Variable> varlist;
    static std::vector<CompUnit> units;
    
    std::vector<CompUnit>* get_unit();
    
    void add_unit(CompUnit *u);

    size_t get_size();

};

#endif