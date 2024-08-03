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
    printf("%s:%lu:%lu:%s:%s\n", _path, line, col_st, get_error(kind), msg);
    std::string l = std::to_string(line);
    printf("%s| %s\n", l, _err_line);
    for (auto c : l)
    {
        std::cerr << ' ';
    }
    std::cerr << '  ';
    for (size_t i = 0; i < col_st; i++)
    {
        std::cerr << ' ';
    }
    std::cerr << '^';
    for (size_t i = col_st; i < col_end; i++)
    {
        std::cerr << '~';
    }
    std::cerr << '\n';
}