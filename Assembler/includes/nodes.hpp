#ifndef _NODES_
#define _NODES_

#include <vector>
#include <string>

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

        struct NodeDefByte
        {
            std::string byte_val;  // the defined byte
            std::string byte_name; // the variable name
            // everything is just a number so even characters will be interpreted as numbers at last
            DataType type;

            NodeDefByte() {}
        };

        struct NodeProcDeclr
        {
            std::string proc_name; // the proc's name
        };

        struct NodeLabel
        {
            std::string label_name;
        };

        struct NodeInstMovRegImm
        {
            Registers dest_regr; // destination register
            std::string value;   // the value of to move
            // the data type is most likely INT or NUM
        };

        union Details
        {
            NodeDefByte def_byte;
            NodeProcDeclr proc_declr;
            NodeLabel label;
            NodeInstMovRegImm inst_mov_reg_imm;

            Details() : def_byte() {}

            ~Details() {}
        };

        struct Node
        {
            NodeType type; // where this node is from
            NodeKind kind;
            Details details;
            Node() : details() {}
        };
    };
};

#endif