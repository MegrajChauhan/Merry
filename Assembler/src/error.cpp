#include "../includes/error.hpp"

void masm::error::_print_err_(std::string _path, size_t _line, size_t _col, std::string _context, std::string _msg, std::string _errline)
{
    std::cerr << _CCODE_BOLDWHITE << _path << ":" << _line << ":" << _col << ":" << _CCODE_RESET << " While ";
    std::cerr << _CCODE_BOLDBLUE << _context << _CCODE_RESET << " : " << _msg << "\n";
    std::cerr << "\t" << _errline << "\n";
    std::cerr << "\t";
    for (size_t i = 0; i < _col; i++)
    {
        std::cerr << " ";
    }
    std::cerr << _CCODE_BOLDRED << "^";
    for (; _col < _errline.length(); _col++)
    {
        std::cerr << "~";
    }
    std::cerr << _CCODE_RESET << "\n";
    exit(EXIT_FAILURE);
}