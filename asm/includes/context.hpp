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
#include "merry_config.h"
#include "expr.hpp"

namespace masm
{
    enum FileType
    {
        CODE,
        DATA,
    };

    class Context
    {
        SymbolTable table;
        std::string inp_file;
        std::string curr_file;
        FileType curr_file_type;
        std::string curr_file_conts;
        std::string inp_file_conts;
        Lexer inp_file_lexer;
        std::unordered_map<std::string, bool> filelist;
        std::vector<std::string> flist;

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
        Expr evaluator; // This is okay
        std::unordered_map<std::string, size_t> data_addr; // this too
        
        Context() = default;

        std::unordered_map<std::string, size_t> *get_lbl_addr();

        std::string *get_eepe();

        std::unordered_map<std::string, std::string> *get_teepe();

        std::vector<std::string> *get_entries();

        CodeGen *get_codegen();

        virtual void init_context(std::string path);

        virtual void read_file(std::string file);

        virtual void handle_defined();
        virtual void handle_ndefined();

        virtual void start();

        virtual bool setup_for_new_file(std::string npath);

        void analyse_proc(); // check if any procedure was left undefined

        void confirm_entries();

        void make_node_analysis();
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
        void init_context(std::string path) override;

        void setup_structure(std::unordered_map<std::string, size_t> *la, std::unordered_map<std::string, std::string> *tep, SymbolTable *t, std::unordered_map<std::string, bool> *fl, std::vector<std::string> *_fl, std::vector<Node> *n, std::unordered_map<std::string, Procedure> *pl, std::unordered_map<std::string, size_t> *ll, std::vector<std::string> *e);

        void start() override;
        
        void read_file(std::string file) override;

        bool setup_for_new_file(std::string npath) override;

        void handle_defined() override;
        void handle_ndefined() override;
    };

    // The below will make sure to transform the lib paths for standard library
    // Obviously it requires that the library is installed into the system
    // This assembler certainly isn't worthy just yet
    static std::unordered_map<std::string, std::string> _std_paths = {
#ifdef _USE_LINUX_
        {"stdinit.masm", "lib/init/stdinit.masm"},
        {"stdinit.mb", "lib/init/stdinit.mb"},
        {"stdinit.mdat", "lib/init/stdinit.mdat"},
        {"stdutils.masm", "lib/utils/stdutils.masm"},
        {"stddefs.mdat", "lib/utils/stddefs.mdat"},
#endif
    };
};

#endif