#ifndef _PARSER_
#define _PARSER_

#include <vector>
#include <string>
#include <filesystem>
#include <unordered_set>
#include <fstream>
#include "symtable.hpp"
#include "nodes.hpp"

namespace masm
{
    static std::unordered_set<std::string> used_files;
    static std::string eepe = "1";
    static std::unordered_map<std::string, std::string> teepe;
    static std::vector<std::string> entries;

    class Parser
    {
        Lexer l;
        std::string fname, fconts;
        SymbolTable symtable;
        std::unordered_map<std::string, bool> proc_list;
        std::unordered_set<std::string> lbl_list;
        std::vector<Node> nodes;
        bool read_again = true;
        Token old_tok;
        std::shared_ptr<std::string> file;

    public:
        Parser() = default;

        void setup_parser(std::string filename);

        void parse();

        bool handle_defines(DataType t, bool _const = false);

        bool handle_strings();

        bool add_variable(Variable v);

        bool add_const(Variable v);

        bool handle_names(bool _proc);

        void handle_one(NodeKind k);

        bool handle_arithmetic(NodeKind k);

        bool handle_mov(NodeKind k);

        bool handle_arithmetic_float(NodeKind k);

        bool handle_movX(NodeKind k);

        // making sure that this can be re-used
        bool handle_jmp(NodeKind k);

        bool handle_call();

        bool handle_sva_svc(NodeKind k);

        bool handle_push_pop(NodeKind k);

        bool handle_single_regr(NodeKind k);

        bool handle_logical_inst(NodeKind k, bool limit);

        bool handle_cmp();

        bool handle_lea();

        bool handle_load_store(NodeKind k, bool atm = false);

        bool handle_atm();

        bool handle_cmpxchg();

        bool handle_sio(NodeKind k);

        bool handle_excg(NodeKind k);

        bool handle_intr();

    };
};

#endif