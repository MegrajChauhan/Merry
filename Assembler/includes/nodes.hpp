#ifndef _NODES_
#define _NODES_

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace masm
{
    namespace nodes
    {
        enum NodeType
        {
            _TYPE_DATA,
            _TYPE_INST,
        };

        enum DataType
        {
            _TYPE_BYTE,
            _TYPE_WORD,
            _TYPE_DWORD,
            _TYPE_QWORD,
            _TYPE_STRING,
            _TYPE_RESB,
            _TYPE_RESW,
            _TYPE_RESD,
            _TYPE_RESQ,
            _TYPE_FLOAT,
            _TYPE_LFLOAT,
        };

        enum NodeKind
        {
            // the kind of node(Not Type but like move, define byte, string etc)
            _DEF_BYTE, // defines a byte
            _DEF_WORD,
            _DEF_DWORD,
            _DEF_QWORD,
            _DEF_STRING,
            _DEF_RESB,
            _DEF_RESW,
            _DEF_RESD,
            _DEF_RESQ,
            _DEF_FLOAT,
            _DEF_LFLOAT,
            // any label that is declared as proc is a procedure while any other are just labels
            _PROC_DECLR, // procedure declaration
            _LABEL,      // A label[Unless semantically verified even procedure definition is a label]
            _INST_NOP,
            _INST_MOV_REG_IMM,
            _INST_MOV_REG_REG,
            _INST_MOV_REG_IMMQ,
            _INST_MOV_REG_MOVEB,
            _INST_MOV_REG_MOVEW,
            _INST_MOV_REG_MOVED,
            _INST_MOV_REG_REG8,
            _INST_MOV_REG_REG16,
            _INST_MOV_REG_REG32,
            _INST_MOV_REG_IMM8,
            _INST_MOV_REG_IMM16,
            _INST_MOV_REG_IMM32,
            _INST_MOVF,
            _INST_MOVLF,

            _INST_MOVSX_REG_REG8,
            _INST_MOVSX_REG_REG16,
            _INST_MOVSX_REG_REG32,
            _INST_MOVSX_REG_IMM8,
            _INST_MOVSX_REG_IMM16,
            _INST_MOVSX_REG_IMM32,

            _INST_OUTR,
            _INST_UOUTR,

            _INST_CIN,
            _INST_SIN,
            _INST_COUT,
            _INST_SOUT,
            _INST_IN,
            _INST_OUT,
            _INST_INW,
            _INST_OUTW,
            _INST_IND,
            _INST_OUTD,
            _INST_INQ,
            _INST_OUTQ,
            _INST_UIN,
            _INST_UOUT,
            _INST_UINW,
            _INST_UOUTW,
            _INST_UIND,
            _INST_UOUTD,
            _INST_UINQ,
            _INST_UOUTQ,
            _INST_INF,
            _INST_OUTF,
            _INST_INLF,
            _INST_OUTLF,

            _INST_ADD_IMM,
            _INST_ADD_REG,
            _INST_IADD_IMM,
            _INST_IADD_REG,

            _INST_SUB_IMM,
            _INST_SUB_REG,
            _INST_ISUB_IMM,
            _INST_ISUB_REG,

            _INST_MUL_IMM,
            _INST_MUL_REG,
            _INST_IMUL_IMM,
            _INST_IMUL_REG,

            _INST_DIV_IMM,
            _INST_DIV_REG,
            _INST_IDIV_IMM,
            _INST_IDIV_REG,

            _INST_MOD_IMM,
            _INST_MOD_REG,
            _INST_IMOD_IMM,
            _INST_IMOD_REG,

            _INST_FADD_REG,
            _INST_FSUB_REG,
            _INST_FMUL_REG,
            _INST_FDIV_REG,
            _INST_FADD_IMM,
            _INST_FSUB_IMM,
            _INST_FMUL_IMM,
            _INST_FDIV_IMM,

            _INST_LFADD_REG,
            _INST_LFSUB_REG,
            _INST_LFMUL_REG,
            _INST_LFDIV_REG,
            _INST_LFADD_IMM,
            _INST_LFSUB_IMM,
            _INST_LFMUL_IMM,
            _INST_LFDIV_IMM,

            // even though we have two kinds of jumps, the jmp_off won't be used
            // instead the jmp_addr variant will be used
            _INST_JMP,
            _INST_CMP_REG, // comparing two registers
            _INST_CMP_IMM, // comparing with immediates

            _INST_JNZ,
            _INST_JZ,
            _INST_JNE,
            _INST_JE,
            _INST_JNC,
            _INST_JC,
            _INST_JNO,
            _INST_JO,
            _INST_JNN,
            _INST_JN,
            _INST_JNG,
            _INST_JG,
            _INST_JNS,
            _INST_JS,
            _INST_JGE,
            _INST_JSE,

            _INST_INC,
            _INST_DEC,

            _INST_AND_IMM,
            _INST_AND_REG,
            _INST_OR_IMM,
            _INST_OR_REG,
            _INST_XOR_IMM,
            _INST_XOR_REG,
            _INST_NOT,

            _INST_LSHIFT,
            _INST_RSHIFT,

            _INST_CFLAGS,
            _INST_RESET,
            _INST_CLZ,
            _INST_CLN,
            _INST_CLC,
            _INST_CLO,
            _INST_RET,
            _INST_CALL,
            _INST_SVA,
            _INST_SVC,
            _INST_PUSH_IMM,
            _INST_PUSH_REG,
            _INST_PUSHA,
            _INST_POP,
            _INST_POPA,

            _INST_HLT, // this doesn't need its own structure
        };

        enum Registers
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

        static std::unordered_map<std::string, Registers> _regr_iden_map = {
            {"Ma", Ma},
            {"Mb", Mb},
            {"Mc", Mc},
            {"Md", Md},
            {"Me", Me},
            {"Mf", Mf},
            {"M1", M1},
            {"M2", M2},
            {"M3", M3},
            {"M4", M4},
            {"M5", M5},
            {"Mm1", Mm1},
            {"Mm2", Mm2},
            {"Mm3", Mm3},
            {"Mm4", Mm4},
            {"Mm5", Mm5},
        };

        // Define base class
        struct Base
        {
            virtual ~Base() {} // Make the base class polymorphic with a virtual destructor
        };


        struct NodeCmpRegr : public Base
        {
            Registers regr1; // the first operand register
            Registers regr2; // the second operand register
        };

        struct NodeCmpImm : public Base
        {
            Registers regr; // the register operand
            std::string val;
            bool is_iden = false; // if set, val will be treated as a variable name
        };

        // we need this so
        struct NodeNop : public Base
        {
        };

        // Define derived classes for each node type
        struct NodeDefByte : public Base
        {
            std::string byte_val;  // the byte value
            std::string byte_name; // the variable name

            NodeDefByte() {} // Initialize byte_val to 0 by default
        };

        struct NodeDefWord : public NodeDefByte
        {
        };
        struct NodeDefDword : public NodeDefByte
        {
        };
        struct NodeDefQword : public NodeDefByte
        {
        };
        struct NodeDefFloat : public NodeDefByte
        {
        };
        struct NodeDefLFloat : public NodeDefByte
        {
        };

        struct NodeJmp : public Base
        {
            std::string _jmp_label_; // the label to jump to
        };

        struct NodeCall : public NodeJmp
        {
        };

        struct NodeProcDeclr : public Base
        {
            std::string proc_name; // the proc's name

            NodeProcDeclr() {} // Default constructor
        };

        struct NodeLabel : public Base
        {
            std::string label_name;

            NodeLabel() {} // Default constructor
        };

        struct NodeInstMovRegImm : public Base
        {
            bool is_iden = false; // interpret value as a variable name
            Registers dest_regr;  // destination register
            std::string value;    // the value of to move
            // the data type is most likely INT or NUM

            NodeInstMovRegImm() {} // Default constructor
        };

        struct NodeInstMovRegReg : public Base
        {
            Registers dest_regr; // destination register
            Registers src_reg;   // the source register
            // the data type is most likely INT or NUM

            NodeInstMovRegReg() {} // Default constructor
        };

        struct NodeAndRegImm : public Base
        {
            Registers dest_regr; // destination register
            std::string value;   // the value of to move
        };

        struct NodeAndRegReg : public NodeInstMovRegReg
        {
        };

        struct NodeOrRegImm : public NodeAndRegImm
        {
        };

        struct NodeOrRegReg : public NodeInstMovRegReg
        {
        };

        struct NodeXorRegImm : public NodeAndRegImm
        {
        };

        struct NodeXorRegReg : public NodeInstMovRegReg
        {
        };

        struct NodeShifts : public NodeAndRegImm
        {
        };

        // this struct is for any instruction that uses just one register operand
        struct NodeOneRegrOperands : public Base
        {
            Registers oper_rger;
        };

        struct NodeOneImmOperand : public Base
        {
            std::string imm;
        };

        struct NodeRes : public Base
        {
            std::string name;
            size_t number;
        };

        struct NodeAddRegReg : public NodeInstMovRegReg
        {
        };

        struct NodeAddRegImm : public NodeInstMovRegImm
        {
        };

        struct NodeSubRegReg : public NodeInstMovRegReg
        {
        };

        struct NodeSubRegImm : public NodeInstMovRegImm
        {
        };

        struct NodeMulRegReg : public NodeInstMovRegReg
        {
        };

        struct NodeMulRegImm : public NodeInstMovRegImm
        {
        };

        struct NodeDivRegReg : public NodeInstMovRegReg
        {
        };

        struct NodeDivRegImm : public NodeInstMovRegImm
        {
        };

        struct NodeModRegReg : public NodeInstMovRegReg
        {
        };

        struct NodeModRegImm : public NodeInstMovRegImm
        {
        };

        // Define struct Node to hold a pointer to Base
        struct Node
        {
            NodeType type; // where this node is from
            NodeKind kind;
            std::unique_ptr<Base> ptr;
            size_t line;

            Node() {}

            Node(NodeType t, NodeKind k, std::unique_ptr<Base> p, size_t pos = 0) : type(t), kind(k), ptr(std::move(p)), line(pos) {}

            // Copy constructor
            Node(const Node &node) : type(node.type), kind(node.kind), line(node.line)
            {
                if (node.ptr)
                {
                    ptr = std::make_unique<Base>(*node.ptr);
                }
            }

            // Assignment operator
            Node &operator=(const Node &node)
            {
                if (this != &node)
                {
                    type = node.type;
                    kind = node.kind;
                    line = node.line;
                    if (node.ptr)
                    {
                        ptr = std::make_unique<Base>(*node.ptr);
                    }
                    else
                    {
                        ptr.reset();
                    }
                }
                return *this;
            }
        };
    };
};

#endif