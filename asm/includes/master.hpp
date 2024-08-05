#ifndef _MASTER_
#define _MASTER_

#include "compunit.hpp"

namespace masm
{
    static std::unordered_map<std::string, Variable> varlist;
    static std::vector<CompUnit> units;
};

#endif