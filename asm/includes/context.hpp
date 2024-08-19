#ifndef _CONTEXT_
#define _CONTEXT_

#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include "error.hpp"
#include "symtable.hpp"
#include "lexer.hpp"
#include "data.hpp"
#include "nodes.hpp"
#include "code.hpp"

namespace masm
{
    enum FileType
    {
        CODE,
        DATA,
    };

    class Context
    {
        std::string inp_file;
        std::string curr_file;
        FileType curr_file_type;
        SymbolTable table;
        std::string curr_file_conts;
        std::string inp_file_conts;
        std::unordered_map<std::string, bool> filelist;
        std::vector<std::string> flist;
        Lexer inp_file_lexer;

        std::vector<Node> nodes;
        std::unordered_map<std::string, Procedure> proc_list; // the list of all procedures
        std::unordered_map<std::string, size_t> labels;

    public:
        Context() = default;

        void init_context(std::string path);

        void read_file(std::string file);

        void start();

        void setup_for_new_file(std::string npath);

        void analyse_proc(); // check if any procedure was left undefined 
    };
};

#endif