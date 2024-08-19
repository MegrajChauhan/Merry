#include "error.hpp"

void masm::err(std::string path, size_t line, size_t col_st, size_t col_ed, std::string _while, std::string _err_name, std::string lvl, std::string msg, std::string errline, std::string suggest)
{
    // Determine color based on error level
    const char *level_color = WHITE;
    if (lvl == "error")
    {
        level_color = BOLD_RED;
    }
    else if (lvl == "warning")
    {
        level_color = BOLD_YELLOW;
    }
    else if (lvl == "info")
    {
        level_color = BOLD_BLUE;
    }

    // Print the error header
    fprintf(stderr, "%s%s [%s%s%s]: While %s%s%s:\n",
            BOLD_WHITE, _err_name.c_str(), level_color, lvl.c_str(), RESET, MAGENTA, _while.c_str(), RESET);

    // Print the file and location info
    fprintf(stderr, "  In file \"%s%s%s\", at line %lu, starting from column %lu to %lu:\n",
            BLUE, path.c_str(), RESET, line, col_st, col_ed);

    // Print the error message
    fprintf(stderr, "%s%s%s\n", BOLD_RED, msg.c_str(), RESET);

    // Print the erroneous line of code
    fprintf(stderr, "  %s%s%s\n", CYAN, errline.c_str(), RESET);

    // Print the location indicator
    fprintf(stderr, "  %*c", static_cast<int>(col_st), ' '); // Print spaces up to col_st
    if (col_ed > col_st)
    {
        fprintf(stderr, "%s^", BOLD_YELLOW);
        for (size_t i = 0; i < (col_ed - col_st); i++)
            putc('~', stderr);
        fprintf(stderr, "%s", RESET);
    }
    else
    {
        fprintf(stderr, "%s^%s", BOLD_YELLOW, RESET);
    }
    fprintf(stderr, "\n");

    // If there's a suggestion, print it in green
    if (!suggest.empty())
    {
        fprintf(stderr, "%sSuggestion: %s%s\n", BOLD_GREEN, suggest.c_str(), RESET);
    }
}

void masm::ld_err(std::string path, size_t line, std::string _while, std::string _err_name, std::string lvl, std::string msg, std::string errline)
{
    const char *level_color = WHITE;
    if (lvl == "error")
    {
        level_color = BOLD_RED;
    }
    else if (lvl == "warning")
    {
        level_color = BOLD_YELLOW;
    }
    else if (lvl == "info")
    {
        level_color = BOLD_BLUE;
    }

    // Print the error header
    fprintf(stderr, "%s%s [%s%s%s]: While %s%s%s:\n",
            BOLD_WHITE, _err_name.c_str(), level_color, lvl.c_str(), RESET, MAGENTA, _while.c_str(), RESET);

    // Print the file and location info
    fprintf(stderr, "  In file \"%s%s%s\", at line %lu:\n",
            BLUE, path.c_str(), RESET, line);

    // Print the error message
    fprintf(stderr, "%s%s%s\n", BOLD_RED, msg.c_str(), RESET);

    // Print the erroneous line of code
    if (!errline.empty())
    {
        fprintf(stderr, "  %s%s%s\n", CYAN, errline.c_str(), RESET);
    }
    fprintf(stderr, "\n");
}

void masm::fu_err(std::string path, size_t line, std::string msg, std::string _fu_line)
{
    fprintf(stderr, "%s%s%s: %s\nFrom file %s%s%s\n", BLUE, "Note", RESET, msg.c_str(), BOLD_WHITE, path.c_str(), RESET);
    if (_fu_line.empty())
        return;
    fprintf(stderr, "    %s\n", _fu_line.c_str());
}
