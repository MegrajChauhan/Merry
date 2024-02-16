#ifndef _READER_
#define _READER_

// The main task of this is to read any file given to it and not just the input file
// for reading modules as well

#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include "error.hpp"

namespace mlang
{
    // this reads line by line which is then consumed by the callee
    // it will keep track of which file it is reading, which line it recently read and what line it is currently on
    // all that above information might be useful for error displaying but we will not keep track of which module is using which

    enum ReaderErrorKind
    {
        FILE_NOT_FOUND,
        DIRECTORY,
        EMPTY,
        INVALID_EXT,
    };

    struct ReaderErrorNode
    {
        // the context will be READING so it will be unnecessary
        // reading error are all fatal
        // the only errors are mentioned above
        ReaderErrorKind kind;
        std::filesystem::path file_path;

        ReaderErrorNode(ReaderErrorKind kind, std::filesystem::path path);
    };

    class ReaderError : public Error<ReaderErrorNode>
    {
    public:
        ReaderError() = default;

        // in the case of the reader just one error is enough to stop the entire process
        // we don't even need to push new errors and get them
        void print_error(ReaderErrorNode error);
    };

    class Reader
    {
    private:
        std::string file_name;
        std::string curr_line;
        std::fstream file_handle;
        size_t current_line;
        ReaderError error;

        bool is_valid_filename()
        {
            // check if file_name is valid
            return file_name.ends_with(".ml");
        }

        bool does_exits_and_not_dir()
        {
            // check if exists and not a directory
            // also check if the file is empty or not
            std::filesystem::path path = std::filesystem::current_path() / this->file_name;
            if (!std::filesystem::exists(path))
            {
                error.print_error(ReaderErrorNode(FILE_NOT_FOUND, path));
                return false;
            }
            if (std::filesystem::is_directory(path))
            {
                error.print_error(ReaderErrorNode(DIRECTORY, path));
                return false;
            }
            if (std::filesystem::is_empty(path))
            {
                error.print_error(ReaderErrorNode(EMPTY, path));
                return false;
            }
            return true;
        }

    public:
        Reader(std::string);

        Reader() = default;

        bool open_and_setup();

        std::string next_line();

        size_t get_current_line_num();

        void set_filename(std::string);

        std::string get_file_name();
    };
};

#endif