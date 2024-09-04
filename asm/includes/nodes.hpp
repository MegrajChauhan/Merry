#ifndef _NODES_
#define _NODES_

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include "symtable.hpp"
#include "lexer.hpp"

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
        ADD_EXPR,
        SUB_IMM,
        SUB_REG,
        SUB_MEM,
        SUB_EXPR,
        MUL_IMM,
        MUL_REG,
        MUL_MEM,
        MUL_EXPR,
        DIV_IMM,
        DIV_REG,
        DIV_MEM,
        DIV_EXPR,
        MOD_IMM,
        MOD_REG,
        MOD_MEM,
        MOD_EXPR,

        IADD_IMM,
        IADD_REG,
        IADD_VAR,
        IADD_EXPR,
        ISUB_IMM,
        ISUB_REG,
        ISUB_VAR,
        ISUB_EXPR,
        IMUL_IMM,
        IMUL_REG,
        IMUL_VAR,
        IMUL_EXPR,
        IDIV_IMM,
        IDIV_REG,
        IDIV_VAR,
        IDIV_EXPR,
        IMOD_IMM,
        IMOD_REG,
        IMOD_VAR,
        IMOD_EXPR,

        FADD,
        FSUB,
        FMUL,
        FDIV,
        LFADD,
        LFSUB,
        LFMUL,
        LFDIV,
        FADD_MEM,
        FSUB_MEM,
        FMUL_MEM,
        FDIV_MEM,
        LFADD_MEM,
        LFSUB_MEM,
        LFMUL_MEM,
        LFDIV_MEM,

        MOV_IMM,
        MOV_REG,
        MOV_VAR,
        MOV_EXPR,

        MOVL_IMM,
        MOVL_REG,
        MOVL_VAR,
        MOVL_EXPR,

        MOVB,
        MOVW,
        MOVD,

        MOVSXB_IMM,
        MOVSXB_REG,
        MOVSXB_VAR,
        MOVSXB_EXPR,

        MOVSXW_IMM,
        MOVSXW_REG,
        MOVSXW_VAR,
        MOVSXW_EXPR,

        MOVSXD_IMM,
        MOVSXD_REG,
        MOVSXD_VAR,
        MOVSXD_EXPR,

        JMP,
        CALL,
        CALL_REG,
        RET,

        SVA_IMM,
        SVA_REG,
        SVA_VAR,
        SVA_EXPR,

        SVC_IMM,
        SVC_REG,
        SVC_VAR,
        SVC_EXPR,

        PUSHA,
        POPA,

        PUSH_IMM,
        PUSH_REG,
        PUSH_VAR,
        PUSH_EXPR,

        POP_IMM, // useless KIND just for there for some reason
        POP_REG,
        POP_VAR,

        NOT,
        INC,
        DEC,

        AND_IMM,
        AND_REG,
        AND_VAR,
        AND_EXPR,
        OR_IMM,
        OR_REG,
        OR_VAR,
        OR_EXPR,
        XOR_IMM,
        XOR_REG,
        XOR_VAR,
        XOR_EXPR,
        LSHIFT,
        LSHIFT_X, // pseudo node type
        LSHIFT_VAR,
        LSHIFT_EXPR,
        RSHIFT,
        RSHIFT_X, // pseudo node type
        RSHIFT_VAR,
        RSHIFT_EXPR,

        CMP_IMM,
        CMP_REG,
        CMP_VAR,
        CMP_EXPR,

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

        JNZ,
        JZ,
        JNE,
        JE,
        JNC,
        JC,
        JNO,
        JO,
        JNN,
        JN,
        JNG,
        JG,
        JNS,
        JS,
        JGE,
        JSE,

        LOOP,
        INTR,
        INTR_EXPR,
        INTR_VAR,

        SETE,
        CALLE,
        SYSCALL,

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

    struct NodeIntr : public Base
    {
        std::variant<std::string, std::vector<Token>> val;
    };

    struct NodeSIO : public NodeName
    {
    };

    struct NodeArithmetic : public Base
    {
        Register reg;
        std::variant<Register, std::string, std::vector<Token>> second_oper;
    };

    // the same as Arithmetic
    struct NodeMov : public Base
    {
        Register reg;
        std::variant<Register, std::pair<std::string, DataType>, std::vector<Token>> second_oper;
    };

    // reusing this is better
    struct NodeExcg : public Base
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
        std::variant<Register, std::string, std::vector<Token>> val;
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
        std::shared_ptr<std::string> _file;
        size_t line;
    };

};

#endif