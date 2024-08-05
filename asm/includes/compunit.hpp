#ifndef _COMPUNIT_
#define _COMPUNIT_

#include "parser.hpp"

namespace masm
{
    class CompUnit
    {
        std::string fname;
        Parser p;
        CompUnit *parent = nullptr;
    public:

    };

    static std::vector<CompUnit> units;

};

#endif