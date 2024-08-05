#include "error.hpp"

void masm::elog(std::string file, size_t line_no, size_t colst, size_t colend, std::string lvl, std::string msg, std::string line)
{
    fprintf(stderr, "%s:%lu:%lu: %s : %s\n", file.c_str(), line_no, colst, lvl.c_str(), msg.c_str());
    std::string l = std::to_string(line_no + 1);
    fprintf(stderr, "%s|\t%s\n", l.c_str(), line.c_str());
    for (size_t i = 0; i < l.length(); i++)
    {
        putc(' ', stderr);
    }
    fprintf(stderr, " \t");
    for (size_t i = 0; i < colst; i++)
    {
       putc(' ', stderr);
    }
    putc('^', stderr);
    for (size_t i = colst; i < colend; i++)
    {
        putc('~', stderr);
    }
    fprintf(stderr, "\n");
}