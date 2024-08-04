#include "error.hpp"

std::string masm::get_error(ErrorKind kind)
{
    switch (kind)
    {
    case FATAL:
        return "FATAL";
    case WARNING:
        return "WARNING";
    }
}

void masm::mlog(std::string _path, size_t line, size_t col_st, size_t col_end, ErrorKind kind, std::string msg, std::string _err_line)
{
    fprintf(stdout, "%s:%lu:%lu:%s:%s\n", _path.c_str(), line, col_st, get_error(kind).c_str(), msg.c_str());
    std::string l = std::to_string(line);
    fprintf(stdout, "%s| %s\n", l.c_str(), _err_line.c_str());
    for (auto c : l)
    {
        std::cout << ' ';
    }
    std::cout << "  ";
    for (size_t i = 0; i < col_st; i++)
    {
        std::cout << ' ';
    }
    std::cout << '^';
    for (size_t i = col_st; i < col_end; i++)
    {
        std::cout << '~';
    }
    std::cout << '\n';
}