#ifndef _NODES_
#define _NODES_

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include "defs.hpp"
#include "lexer.hpp"

namespace masm
{
    struct Base
    {
    };

    struct NodeName : public Base
    {
        std::variant<std::string, Register> oper;
    };

    struct NodeArithmetic : public Base
    {
        bool is_signed = false; // Is signed arithmetic?
        Register reg;
        std::variant<Register, std::string, std::vector<Token>> second_oper;
    };

    struct NodeTwoRegr : public Base
    {
        Register r1, r2;
    };

    struct NodeMov : public Base
    {
        Register reg;
        std::variant<Register, std::string, std::vector<Token>> second_oper;
        bool is_float = false; // just for the mov instruction
        bool is_lbl = false;
    };

    struct NodeStack : public Base
    {
        Register reg;
        std::variant<Register, std::string, std::vector<Token>> second_oper;
        bool _is_lbl;
    };

    struct NodeLea : public Base
    {
        Register dest, base, ind, scale;
    };

    struct NodeIntr : public Base
    {
        std::variant<std::vector<Token>, std::string> val;
    };

    struct NodeCmpxchg : public Base
    {
        Register reg1, reg2;
        std::variant<Register, std::string> var;
    };

    typedef NodeName NodeIO;
    typedef NodeTwoRegr NodeExcg;
    typedef NodeMov NodeLogical;
    typedef NodeMov NodeLoadStore;

    struct Node
    {
        NodeKind kind;
        std::unique_ptr<Base> node;
        std::shared_ptr<std::string> _file;
        size_t line;
    };

};

#endif