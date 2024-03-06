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
        };

        enum NodeKind
        {
            // the kind of node(Not Type but like move, define byte, string etc)
            _DEF_BYTE, // defines a byte
            _DEF_WORD,
            _DEF_DWORD,
            _DEF_QWORD,
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

            _INST_MOVSX_REG_REG8,
            _INST_MOVSX_REG_REG16,
            _INST_MOVSX_REG_REG32,
            _INST_MOVSX_REG_IMM8,
            _INST_MOVSX_REG_IMM16,
            _INST_MOVSX_REG_IMM32,

            _INST_OUTR,
            _INST_UOUTR,

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

        // we need this so
        struct NodeNop: public Base
        {};

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