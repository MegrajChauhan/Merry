#ifndef _CODEGEN_
#define _CODEGEN_

#include "../includes/sema.hpp"
#include "../includes/opcodes.hpp"

namespace masm
{
    namespace codegen
    {
        union Instruction
        {
            uint64_t whole = 0;

            struct
            {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
                uint8_t b1, b2, b3, b4, b5, b6, b7, b8;
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
                uint8_t b8, b7, b6, b5, b4, b3, b2, b1;
#endif
            } bytes;

            Instruction() = default;
        };

        class Codegen
        {
            symtable::SymTable table;
            std::vector<std::unique_ptr<nodes::Node>> inst_nodes;
            std::vector<Instruction> inst_bytes;
            std::vector<unsigned char> data_bytes;
            std::unordered_map<std::string, size_t> data_addrs;
            std::unordered_map<std::string, size_t> label_addrs;
            size_t main_proc_ind = 0;

        public:
            Codegen() = default;

            // probably don't use this
            Codegen(std::vector<std::unique_ptr<nodes::Node>> &);

            Codegen(sema::Sema &);

            // generate data bytes
            // the strings is also data
            void gen_data();

            // generate bytes
            void gen();

            size_t get_entry_addr() { return label_addrs["main"]; }

            auto get_instructions() { return inst_bytes; }

            auto get_data() { return data_bytes; }

            // label the labels
            void label_labels();

            void gen_inst_mov_reg_imm(std::unique_ptr<nodes::Node> &);

            void gen_inst_mov_reg_reg(std::unique_ptr<nodes::Node> &);
        };
    };
};

#endif