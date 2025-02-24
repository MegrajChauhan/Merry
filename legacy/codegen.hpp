#ifndef _CODEGEN_
#define _CODEGEN_

#include <unordered_map>
#include <vector>
#include <string>
#include "symtable.hpp"
#include "error.hpp"
#include "expr.hpp"
#include "nodes.hpp"
#include "merry_opcodes.h"
#include "emit.hpp"

#define GET(k) (k *)node.node.get()

namespace masm
{
    bool evaluate_data(SymbolTable *table, std::unordered_map<std::string, size_t> *daddr, std::vector<uint8_t> *data, std::vector<uint8_t> *str_data);

    bool evaluate_consts(SymbolTable *table, Expr *e);

    class CodeGen
    {
        // We making this public to make things easier
    public:
        std::unordered_map<std::string, size_t> *data_addr;
        std::unordered_map<std::string, size_t> *lbl_addr;
        std::vector<uint8_t> ST, *data, *str;
        std::vector<GenBinary> code;
        std::unordered_map<size_t, size_t> symd;
        std::vector<Node> *nodes;
        SymbolTable *table;

        void setup_emit(Emit *emit);

        void gen();

        void gen_ST();

        void arithmetic_inst_imm(NodeArithmetic *n, size_t op);

        void arithmetic_inst_reg(NodeArithmetic *n, size_t op);

        void arithmetic_inst_mem(NodeArithmetic *n, size_t op);

        void float_mem(NodeArithmetic *n, size_t op);

        void mov_imm(NodeMov *n, bool _is64);
        void cmov_imm(NodeMov *n, size_t opcode);

        void mov_reg(NodeMov *n, size_t op);

        void mov_var(NodeMov *n);

        void movsx_imm(NodeMov *n, size_t op);

        void branch(NodeName *n, size_t op);

        void sva_svc(NodeStack *n, size_t op);

        void stack(NodeStack *n, size_t op);

        void logical_singles(NodeName *n, size_t op);

        void logical_inst_imm(NodeLogical *n, size_t op);

        void logical_inst_reg(NodeLogical *n, size_t op);

        void cmp_var(NodeLogical *n);

        void lea(NodeLea *n);

        void load_store_var(NodeLoadStore *n, size_t op);

        void cmpxchg(NodeCmpxchg *n, bool _reg);

        void excg(NodeExcg *n, size_t op);

        void generate_singles(size_t opcode);

        void single_regr(size_t op, Register reg);

        void atomic_reg(NodeLoadStore *n, size_t op);
    };
};

#endif