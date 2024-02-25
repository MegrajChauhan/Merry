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
            _TYPE_NUM,
        };

        enum NodeKind
        {
            // the kind of node(Not Type but like move, define byte, string etc)
            _DEF_BYTE,   // defines a byte
                         // any label that is declared as proc is a procedure while any other are just labels
            _PROC_DECLR, // procedure declaration
            _LABEL,      // A label[Unless semantically verified even procedure definition is a label]
            _INST_MOV_REG_IMM,
            _INST_MOV_REG_REG,
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

        // Define derived classes for each node type
        struct NodeDefByte : public Base
        {
            std::string byte_val;       // the byte value
            std::string byte_name; // the variable name

            NodeDefByte() {} // Initialize byte_val to 0 by default
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
            Registers dest_regr; // destination register
            std::string value; // the value of to move
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

            Node() {}

            Node(NodeType t, NodeKind k, std::unique_ptr<Base> p) : type(t), kind(k), ptr(std::move(p)) {}

            // Copy constructor
            Node(const Node &node) : type(node.type), kind(node.kind)
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