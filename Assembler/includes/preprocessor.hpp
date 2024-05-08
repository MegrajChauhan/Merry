#ifndef _PREPROCESSOR_
#define _PREPROCESSOR_

#include <vector>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <thread>
#include "reader.hpp"
#include "error.hpp"

#define _CONTEXT_PREP_ "Preprocessing"

namespace masm
{
    namespace prep
    {
        struct PrepSyms
        {
            std::unordered_map<std::string, std::string> _syms_;
        };

        struct FileDetails
        {
            std::string _file_contents_; // for included files
            size_t lines;
        };

        struct _FileSyms_
        {
            std::string _file_name_;
            size_t start, end;
        };

        class Prep
        {
            PrepSyms syms;
            reader::Reader _reader_;
            std::string _file_contents_;
            std::vector<std::string> _included_files_;
            std::vector<std::string> _self_files_;
            size_t line_no;
            size_t col_no;
            std::string::iterator curr_char;
            std::unordered_map<std::string, FileDetails> _details_;
            std::vector<_FileSyms_> _syms_;
            size_t _start_ = 0;

        public:
            Prep(std::string);
            Prep() = default;

            void setup(std::string);

            void prep();

            // Read all the includes and build a tree
            void get_all_includes(std::vector<std::string>);

            void handle_defines();

            std::string get_curr_line();

            void gen_final_file();

            void _second_pass_();

            void _third_pass_();

            std::string get_the_sym(std::string);

            void remove_until_endif(std::string, std::string *);
            void add_until_endif(std::string, std::string *);

            void _clean_directives_(std::string);

            // std::string get_file();

            _FileSyms_ get_file();

            // for lexer
            _FileSyms_ get_file(size_t);

            std::string get_final_contents() { return _file_contents_; }
        };
    };
};

#endif