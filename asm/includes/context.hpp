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
#include "codegen.hpp"

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
        std::unordered_map<std::string, size_t> label_addr;

        std::string eepe = "1";
        std::unordered_map<std::string, std::string> teepe;
        std::vector<std::string> entries;

        CodeGen gen;

        std::vector<bool> _end_queue;
        bool skip = false;

    public:
        Context() = default;

        std::unordered_map<std::string, size_t> *get_lbl_addr();

        std::string *get_eepe();

        std::unordered_map<std::string, std::string>* get_teepe();

        std::vector<std::string>* get_entries();

        CodeGen* get_codegen();

        void init_context(std::string path);

        void read_file(std::string file);

        virtual void handle_defined();
        virtual void handle_ndefined();

        virtual void start();

        virtual void setup_for_new_file(std::string npath);

        void analyse_proc(); // check if any procedure was left undefined

        void confirm_entries();
    };

    class ChildContext : public Context
    {
        std::string inp_file;
        std::string curr_file;
        FileType curr_file_type;
        SymbolTable *table;
        std::string curr_file_conts;
        std::string inp_file_conts;
        std::unordered_map<std::string, bool> *filelist;
        std::vector<std::string> *flist;
        Lexer inp_file_lexer;

        std::vector<Node> *nodes;
        std::unordered_map<std::string, Procedure> *proc_list; // the list of all procedures
        std::unordered_map<std::string, size_t> *labels;

        std::unordered_map<std::string, size_t> *label_addr;
        std::string *eepe;
        std::unordered_map<std::string, std::string> *teepe;
        std::vector<std::string> *entries;

        std::vector<bool> _end_queue;
        bool skip = false;

    public:
        ChildContext() = default;
        ChildContext(std::string *ee) : eepe(ee) {}

        void setup_structure(std::unordered_map<std::string, size_t> *la, std::unordered_map<std::string, std::string> *tep, SymbolTable *t, std::unordered_map<std::string, bool> *fl, std::vector<std::string> *_fl, std::vector<Node> *n, std::unordered_map<std::string, Procedure> *pl, std::unordered_map<std::string, size_t> *ll, std::vector<std::string> *e);

        void start() override;

        void setup_for_new_file(std::string npath) override;

        void handle_defined() override;
        void handle_ndefined() override;
    };
};

#endif