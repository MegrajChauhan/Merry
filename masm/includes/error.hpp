#ifndef _ERR_
#define _ERR_

#include <iostream>
#include <string>

namespace masm
{
    void log(std::string _f, std::string msg, size_t l_st, size_t c_st, size_t l_ed = 0, size_t c_ed = 0, std::string line = "");
};

#endif