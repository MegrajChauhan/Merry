#ifndef _NODES_
#define _NODES_

#include <string>
#include <vector>
#include <memory>
#include <variant>

namespace masm
{
    enum NodeKind
    {
        PROC_DECLR,
        LABEL,
        NOP,
        HLT,
        ADD_IMM,
        ADD_REG,
        ADD_MEM,
        SUB_IMM,
        SUB_REG,
        SUB_MEM,
        MUL_IMM,
        MUL_REG,
        MUL_MEM,
        DIV_IMM,
        DIV_REG,
        DIV_MEM,
        MOD_IMM,
        MOD_REG,
        MOD_MEM,

        IADD_IMM,
        IADD_REG,
        ISUB_IMM,
        ISUB_REG,
        IMUL_IMM,
        IMUL_REG,
        IDIV_IMM,
        IDIV_REG,
        IMOD_IMM,
        IMOD_REG,

        FADD,
        FSUB,
        FMUL,
        FDIV,
        LFADD,
        LFSUB,
        LFMUL,
        LFDIV,
    };

    enum Register
    {
        Ma,
        Mb,
        Mc,
        Md,
        Me,
        Mf,
        M1,
        M2,
        M3,
        M4,
        M5,
        Mm1,
        Mm2,
        Mm3,
        Mm4,
        Mm5,
    };

    
    struct Procedure
    {
        bool defined;
        size_t ind;
    };

    struct Base
    {
    };

    struct NodeName : public Base
    {
        std::string name;
    };

    struct NodeArithmetic : public Base
    {
        Register reg;
        std::variant<Register, std::string> second_oper;
    };

    struct Node
    {
        NodeKind kind;
        std::unique_ptr<Base> node;
        size_t line_st, line_ed, col_st;
        std::shared_ptr<std::string> file;
    };

};

#endif