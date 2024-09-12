#include "error.hpp"

void masm::log(std::string _f, std::string msg, size_t l_st, size_t c_st, size_t l_ed, size_t c_ed, std::string line)
{
    std::cerr << "In File \"" << _f << "\":" << l_st << ":" << c_st;
    if (l_ed > 0)
        std::cerr << ": Spanning upto line:" << l_ed;
    if (c_ed > 0)
        std::cerr << ": [UPTO COLUMNS: " << c_ed << "]";
    std::cerr << ": " << msg << "\n";
    if (!line.empty())
        std::cerr << "\n"
                  << line << "\n";
}