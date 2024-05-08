#ifndef _ERROR_
#define _ERROR_

#include <string>
#include <iostream>

namespace masm
{
    namespace error
    {
#define _CCODE_RESET "\033[0m"
#define _CCODE_BLACK "\033[30m"
#define _CCODE_RED "\033[31m"
#define _CCODE_GREEN "\033[32m"
#define _CCODE_YELLOW "\033[33m"
#define _CCODE_BLUE "\033[34m"
#define _CCODE_MAGENTA "\033[35m"
#define _CCODE_CYAN "\033[36m"
#define _CCODE_WHITE "\033[37m"

#define _CCODE_BOLDBLACK "\033[1;30m"
#define _CCODE_BOLDRED "\033[1;31m"
#define _CCODE_BOLDGREEN "\033[1;32m"
#define _CCODE_BOLDYELLOW "\033[1;33m"
#define _CCODE_BOLDBLUE "\033[1;34m"
#define _CCODE_BOLDMAGENTA "\033[1;35m"
#define _CCODE_BOLD_CYAN "\033[1;36m"
#define _CCODE_BOLDWHITE "\033[1;37m"

        // generally errors are like this
        /*
         _file_path_:line:col: While .....: Error: _error_msg_
           The line
                   ^^^^^^ --> highlight the error
        */

        void _print_err_(std::string, size_t, size_t, std::string, std::string, std::string);
    };
};

#endif