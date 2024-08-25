#ifndef _NODES_
#define _NODES_

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include "symtable.hpp"

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

        MOV_IMM,
        MOV_REG,
        MOV_VAR,

        MOVL_IMM,
        MOVL_REG,
        MOVL_VAR,

        MOVB,
        MOVW,
        MOVD,

        MOVSXB_IMM,
        MOVSXB_REG,
        MOVSXB_VAR,

        MOVSXW_IMM,
        MOVSXW_REG,
        MOVSXW_VAR,

        MOVSXD_IMM,
        MOVSXD_REG,
        MOVSXD_VAR,

        JMP,
        CALL,
        CALL_REG,
        RET,

        SVA_IMM,
        SVA_REG,
        SVA_VAR,

        SVC_IMM,
        SVC_REG,
        SVC_VAR,

        PUSHA,
        POPA,

        PUSH_IMM,
        PUSH_REG,
        PUSH_VAR,

        POP_IMM, // useless KIND just for there for some reason
        POP_REG,
        POP_VAR,

        NOT,
        INC,
        DEC,

        AND_IMM,
        AND_REG,
        OR_IMM,
        OR_REG,
        XOR_IMM,
        XOR_REG,
        LSHIFT,
        RSHIFT,

        CMP_IMM,
        CMP_REG,
        CMP_VAR,

        LEA,

        LOADB_REG,
        LOADB_VAR,
        ALOADB_VAR,

        LOADW_REG,
        LOADW_VAR,
        ALOADW_VAR,

        LOADD_REG,
        LOADD_VAR,
        ALOADD_VAR,

        LOADQ_REG,
        LOADQ_VAR,
        ALOADQ_VAR,

        STOREB_REG,
        STOREB_VAR,
        ASTOREB_VAR,

        STOREW_REG,
        STOREW_VAR,
        ASTOREW_VAR,

        STORED_REG,
        STORED_VAR,
        ASTORED_VAR,

        STOREQ_REG,
        STOREQ_VAR,
        ASTOREQ_VAR,

        CMPXCHG,

        OUTR,
        UOUTR,

        CIN,
        COUT,
        SIN,
        SOUT,
        IN,
        INW,
        IND,
        INQ,
        UIN,
        UINW,
        UIND,
        UINQ,
        OUT,
        OUTW,
        OUTD,
        OUTQ,
        UOUT,
        UOUTW,
        UOUTD,
        UOUTQ,

        INF,
        INLF,
        OUTF,
        OUTLF,

        EXCGB,
        EXCGW,
        EXCGD,
        EXCGQ,

        MOVEB,
        MOVEW,
        MOVED,

        CFLAGS,
        NRESET,
        CLZ,
        CLN,
        CLC,
        CLO,
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

    struct NodeSIO: public NodeName
    {};

    struct NodeArithmetic : public Base
    {
        Register reg;
        std::variant<Register, std::string> second_oper;
    };

    // the same as Arithmetic
    struct NodeMov : public Base
    {
        Register reg;
        std::variant<Register, std::pair<std::string, DataType>> second_oper;
    };

    // reusing this is better
    struct NodeExcg: public Base
    {
        Register r1, r2;
    };

    struct NodeSTACK : public NodeMov
    {
    };

    struct NodeCall : public Base
    {
        std::variant<Register, std::string> _oper;
    };

    struct NodePushPop : public Base
    {
        std::variant<Register, std::string> val;
    };

    struct NodeSingleRegr : public Base
    {
        Register reg;
    };

    struct NodeLogical : public NodeArithmetic
    {
    };

    struct NodeLea : public Base
    {
        Register dest, base, ind, scale;
    };

    struct NodeCmpxchg : public Base
    {
        Register reg1, reg2;
        std::string var;
    };

    struct NodeLoadStore : public NodeArithmetic
    {
    };

    struct Node
    {
        NodeKind kind;
        std::unique_ptr<Base> node;
    };

};

#endif