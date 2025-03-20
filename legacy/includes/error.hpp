#ifndef _ERROR_
#define _ERROR_

/**
 * I apologize for the worst error handling in existence
 */

#include <iostream>

#define note(msg) std::cout << "NOTE: " << msg << "\n"
#define die(code) exit(code)

// ANSI color codes
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define BOLD_RED "\033[1m\033[31m"
#define BOLD_GREEN "\033[1m\033[32m"
#define BOLD_YELLOW "\033[1m\033[33m"
#define BOLD_BLUE "\033[1m\033[34m"
#define BOLD_WHITE "\033[1m\033[37m"

#define ERR_STR "ERROR"
#define WARN_STR "WARNING"
#define NOTICE_STR "NOTE"

#define building "Building"
#define _parsing "Parsing"
#define _lexing "Lexing"

#define syntaxerr "Syntax Error"
#define invalidtok "Invalid Token"
#define invalext "Inavlid Extension"
#define redefin "Redefinition"
#define straytok "Stray Token"
#define redeclr "Redeclaration"
#define noorder "Out of Order"

namespace masm
{
    void err(std::string path, size_t line, size_t col_st, size_t col_ed, std::string _while, std::string _err_name, std::string lvl, std::string msg, std::string errline, std::string suggest = "");

    // less detailed error
    void ld_err(std::string path, size_t line, std::string _while, std::string _err_name, std::string lvl, std::string msg, std::string errline);

    // follow-up error
    // For things like redefinition of variables.
    void fu_err(std::string path, size_t line, std::string msg, std::string _fu_line = "");
};

#endif