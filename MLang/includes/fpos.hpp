#ifndef _FPOS_
#define _FPOS_

// Defines the lexer's position in the file being lexed
// will help in error handling
// But when imports are added, how will this tackle that issue?
#include <string>

/// TODO: add a limit here so that a file can't have just one line[Limit the maximum column]
#define _MLANG_COLUMN_LIM_

namespace MLang
{
    // get the current line pos, column pos
    // for a error handler to get the current status
    struct FStatus
    {
        std::string file_name;
        size_t col;
        size_t line;

        FStatus(std::string, size_t, size_t);
    };
    class FPos
    {
    private:
        std::string _file_name; // input's file's name
        size_t col = 0;         // the column position
        size_t line_no = 0;     // the current line number
    public:
        FPos() = default;

        void set_filename(std::string file_name);

        // update the position based on the current character
        void update_pos(int current_char);

        // get the current status
        FStatus get_current_status();
        // no need for destructor
    };
};

#endif