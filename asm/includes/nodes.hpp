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