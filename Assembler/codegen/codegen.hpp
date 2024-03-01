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
            struct
            {
                uint8_t b1, b2, b3, b4, b5, b6, b7, b8;
            } bytes;

            struct
            {
                uint32_t upper_32;
                uint32_t lower_32;
            } grp32;

            struct
            {
                uint16_t upper_upper_16, upper_lower_16, lower_upper_16, lower_lower_16;
            } grp16;

            uint64_t grp64;

            Instruction() = default;
            Instruction(uint64_t val) : grp64(val) {}
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

            void append_zeros(size_t len);

            void gen_inst_mov_reg_imm(std::unique_ptr<nodes::Node> &);

            void gen_inst_mov_reg_reg(std::unique_ptr<nodes::Node> &);
        };
    };
};

#endif