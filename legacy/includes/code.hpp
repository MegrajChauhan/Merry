#ifndef _CODE_
#define _CODE_

#include "lexer.hpp"
#include "symtable.hpp"
#include "nodes.hpp"

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

        void handle_one(NodeKind k);

        bool handle_arithmetic_unsigned(NodeKind k);

        bool handle_mov(NodeKind k);

        bool handle_arithmetic_signed(NodeKind k);

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

        bool check_var(std::string name);
        bool check_lbl(std::string name);
        bool check_const(std::string name);
    };
};

#endif