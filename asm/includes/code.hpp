#ifndef _CODE_
#define _CODE_

#include "lexer.hpp"
#include "nodes.hpp"
#include "symtable.hpp"

#define regr            \
    regr_line = t.line; \
    regr_col = t.col

namespace masm
{
    class Code
    {
        // F to unique or shared pointers ;)
        std::vector<Node> *nodes;
        std::unordered_map<std::string, Procedure> *proc_list;
        std::unordered_map<std::string, size_t> *label_list;
        Lexer _l;
        std::shared_ptr<std::string> file;
        std::string fname;
        Token t;

        size_t regr_line, regr_col;

        SymbolTable *table;

    public:
        Code() = default;

        void setup_code_read(std::vector<Node> *n, std::unordered_map<std::string, Procedure> *pl, std::shared_ptr<std::string> fn, std::shared_ptr<std::string> fcont, SymbolTable *sym, std::unordered_map<std::string, size_t> *lbl_list);

        bool read_code();

        bool handle_names(bool _proc);

        bool handle_arithmetic_unsigned(NodeKind k);

        bool handle_mov(NodeKind k);

        bool handle_arithmetic_signed(NodeKind k);

        bool handle_arithmetic_float(NodeKind k);

        bool handle_movX(NodeKind k);

        // making sure that this can be re-used
        bool handle_jmp(NodeKind k);

        bool handle_call();

        bool handle_sva_svc(NodeKind k);

        bool handle_push_pop();

        bool check_var(std::string name);
        bool check_lbl(std::string name);
    };
};

#endif