#ifndef _ERROR_
#define _ERROR_

#include <iostream>

namespace masm
{
#define interr(msg) std::cerr << "Internal Error: " << msg << "\n"
#define log(msg) std::cerr << msg << '\n'

    void elog(std::string file, size_t line_no, size_t colst, size_t colend, std::string lvl, std::string msg, std::string line);

    void note(std::string file, size_t line_no, std::string msg, std::string line);
};

#endif