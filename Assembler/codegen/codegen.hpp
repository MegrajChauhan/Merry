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

        union FLoat32
        {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            uint8_t b1, b2, b3, b4;
            struct
            {
                unsigned long sign : 1;
                unsigned long expo : 8;
                unsigned long mantissa : 23;
            } in_grps;
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            uint8_t b4, b3, b2, b1;
            struct
            {
                unsigned long mantissa : 23;
                unsigned long expo : 8;
                unsigned long sign : 1;
            } in_grps;
#endif
            uint32_t in_int;
            float whole;
        };

        union FLoat64
        {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            uint8_t b1, b2, b3, b4, b5, b6, b7, b8;
            struct
            {
                unsigned long sign : 1;
                unsigned long expo : 11;
                unsigned long mantissa : 52;
            } in_grps;
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            uint8_t b8, b7, b6, b5, b4, b3, b2, b1;
            struct
            {
                unsigned long mantissa : 52;
                unsigned long expo : 11;
                unsigned long sign : 1;
            } in_grps;
#endif
            uint64_t in_int;
            double whole;
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
            size_t str_start_len = 0;

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

            size_t get_str_len() { return str_start_len; }

            auto get_instructions() { return inst_bytes; }

            auto get_data() { return data_bytes; }

            // label the labels
            void label_labels();

            void gen_inst_mov_reg_imm(std::unique_ptr<nodes::Node> &);

            void gen_inst_mov_reg_immq(std::unique_ptr<nodes::Node> &);

            void gen_inst_mov_reg_reg(std::unique_ptr<nodes::Node> &, size_t);

            void gen_inst_move(std::unique_ptr<nodes::Node> &, size_t);

            void gen_inst_movsx_reg_reg(std::unique_ptr<nodes::Node> &, size_t);
            void gen_inst_movsx_reg_imm(std::unique_ptr<nodes::Node> &, size_t);

            void gen_inst_cin(std::unique_ptr<nodes::Node> &);
            void gen_inst_cout(std::unique_ptr<nodes::Node> &);

            void gen_inst_sin(size_t);
            void gen_inst_sout(size_t);

            void gen_inst_movf(std::unique_ptr<nodes::Node> &);

            void gen_inst_add(std::unique_ptr<nodes::Node> &);
            void gen_inst_sub(std::unique_ptr<nodes::Node> &);
            void gen_inst_mul(std::unique_ptr<nodes::Node> &);
            void gen_inst_div(std::unique_ptr<nodes::Node> &);
            void gen_inst_mod(std::unique_ptr<nodes::Node> &);

            void gen_inst_iadd(std::unique_ptr<nodes::Node> &);
            void gen_inst_isub(std::unique_ptr<nodes::Node> &);
            void gen_inst_imul(std::unique_ptr<nodes::Node> &);
            void gen_inst_idiv(std::unique_ptr<nodes::Node> &);
            void gen_inst_imod(std::unique_ptr<nodes::Node> &);

            void gen_inst_fadd(std::unique_ptr<nodes::Node> &);
            void gen_inst_fsub(std::unique_ptr<nodes::Node> &);
            void gen_inst_fmul(std::unique_ptr<nodes::Node> &);
            void gen_inst_fdiv(std::unique_ptr<nodes::Node> &);

            void gen_inst_jmp(std::unique_ptr<nodes::Node> &);
            void gen_inst_cmp(std::unique_ptr<nodes::Node> &);
        };
    };
};

#endif