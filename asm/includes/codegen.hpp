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
            unsigned long long full;
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

        std::unordered_map<std::string, size_t> data_addr;
        std::unordered_map<std::string, size_t> label_addr;

    public:
        CodeGen() = default;

        void setup_codegen(SymbolTable *_t, std::vector<Node> *_n);

        bool generate();

        // why would this fail??
        void generate_data();

        void give_address_to_labels();

        void handle_arithmetic_reg_imm(msize_t op, NodeArithmetic *a);
        void handle_arithmetic_reg_reg(msize_t op, NodeArithmetic *a);
        void handle_arithmetic_reg_var(NodeArithmetic *a, msize_t op);

        void handle_mov_reg_imm(bool l, NodeMov *n);
        void handle_mov_reg_reg(NodeMov *n, msize_t op);
        void handle_mov_reg_var(NodeMov *n);

        void handle_movsx_reg_var(NodeMov* n, NodeKind k);
    };
};

#endif