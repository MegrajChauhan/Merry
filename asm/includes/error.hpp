#ifndef _ERROR_
#define _ERROR_

#include <iostream>
#include <string>

namespace masm
{
    enum ErrorKind
    {
        FATAL,
        WARNING,
    };

   std::string get_error(ErrorKind kind);

   void mlog(std::string _path, size_t line, size_t col_st, size_t col_end, ErrorKind kind, std::string msg, std::string _err_line);
};

#endif