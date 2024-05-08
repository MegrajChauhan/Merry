#ifndef _READER_
#define _READER_

#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <optional>

namespace masm
{
    // this reads line by line which is then consumed by the callee
    // it will keep track of which file it is reading, which line it recently read and what line it is currently on
    // all that above information might be useful for error displaying but we will not keep track of which module is using which
    namespace reader
    {
        enum ReaderErrorKind
        {
            FILE_NOT_FOUND,
            DIRECTORY,
            EMPTY,
            INVALID_EXT,
        };

        class Reader
        {
        private:
            std::string file_name;
            std::fstream _f;

            bool is_valid_filename()
            {
                // check if file_name is valid
                return file_name.ends_with(".masm");
            }

            void does_exits_and_not_dir()
            {
                // check if exists and not a directory
                // also check if the file is empty or not
                std::filesystem::path path = std::filesystem::current_path() / this->file_name;
                if (!std::filesystem::exists(path))
                {
                    print_error(FILE_NOT_FOUND);
                }
                if (std::filesystem::is_directory(path))
                {
                    print_error(DIRECTORY);
                }
                if (std::filesystem::is_empty(path))
                {
                    print_error(EMPTY);
                }
            }

        public:
            Reader(std::string);

            Reader() = default;

            void setup();

            std::string read();

            void set_filename(std::string);

            std::string get_file_name();

            std::string get_base_dir();

            void print_error(ReaderErrorKind);

            std::optional<std::string> get_next_line();

            void open_file();

            bool _may_not_exist_setup_();

            ~Reader()
            {
                if (_f.is_open())
                    _f.close();
            }
        };
    };
};

#endif