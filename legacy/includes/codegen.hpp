#ifndef _CODEGEN_
#define _CODEGEN_

#include <vector>
#include <unordered_map>
#include "symtable.hpp"
#include "nodes.hpp"
#include "error.hpp"
#include "merry_config.h"
#include "merry_types.h"
#include "merry_opcodes.h"
#include "expr.hpp"

namespace masm
{
    struct GenBinary
    {
        union
        {
            struct
            {
#if _MERRY_ENDIANNESS_ == _MERRY_LITTLE_ENDIAN_
                mbyte_t b8, b7, b6, b5, b4, b3, b2, b1;
#else
                mbyte_t b1, b2, b3, b5, b6, b4, b7, b8;
#endif
            } bytes;
            unsigned long long full = 0;
        };
    };

    // generates the final binary
    // but needs to generate the data too!
    class CodeGen
    {
        SymbolTable *table;
        std::vector<Node> *nodes;

        std::vector<GenBinary> code;   // the code
        std::vector<mbyte_t> data;     // the data
        std::vector<mbyte_t> str_data; // for strings
        std::vector<mbyte_t> ST;       // Symbol Table
        std::unordered_map<size_t, size_t> symd;

        std::unordered_map<std::string, size_t> *data_addr;
        std::unordered_map<std::string, size_t> *label_addr;
                    Expr e;

    public:
        CodeGen() = default;

        void setup_codegen(SymbolTable *_t, std::vector<Node> *_n, std::unordered_map<std::string, size_t> *lb, std::unordered_map<std::string, size_t> *data_addr);

        std::vector<GenBinary> *get_code();

        std::vector<mbyte_t> *get_data();

        std::vector<mbyte_t> *get_str_data();

        std::vector<mbyte_t> *get_ST();

        std::unordered_map<size_t, size_t> *get_symd();

        bool generate();

        // why would this fail??
        void generate_data();

        void generate_ST();

        void give_address_to_labels();

        void handle_arithmetic_reg_imm(msize_t op, NodeArithmetic *a, size_t _a = 0xFFFFFFFFFFFF);
        void handle_arithmetic_reg_reg(msize_t op, NodeArithmetic *a);
        void handle_arithmetic_reg_var(NodeArithmetic *a, msize_t op);

        void handle_mov_reg_imm(bool l, NodeMov *n);
        void handle_mov_reg_reg(NodeMov *n, msize_t op);
        void handle_mov_reg_var(NodeMov *n);

        void handle_jmp(msize_t op, NodeName *n);

        void handle_push_pop_reg(msize_t op, NodePushPop *n);

        void handle_push_imm(NodePushPop *n);

        void handle_push_pop_var(msize_t op, NodePushPop *n);

        void handle_single_regr(msize_t op, NodeSingleRegr *n);

        void handle_logical_reg_imm(msize_t op, NodeLogical *n);
        void handle_logical_reg_reg(msize_t op, NodeLogical *n);

        void handle_lea(NodeLea *n);

        void handle_load_store_reg_var(NodeLoadStore *n, msize_t op);
        void handle_load_store_reg_reg(NodeLoadStore *n, msize_t op);

        void handle_cmpxchg(NodeCmpxchg *n);

        void handle_excg(NodeExcg *n, msize_t op);

        void handle_one(msize_t op);

        void handle_movsx(NodeMov *n, msize_t op);

        void handle_movsx_var(NodeMov *n, msize_t op);

        void handle_sva_svc_var(NodeSTACK *n, msize_t op);

        void handle_float_var(NodeArithmetic *n, msize_t op);

        void handle_cmp_var(NodeArithmetic *n);
    };
};

#endif