#ifndef _PARSER_
#define _PARSER_

#include "lexer.hpp"

namespace MLang
{
    // the type of node
    enum NodeKind
    {
    };

    struct Scope
    {
        Scope *parent_scope; // the parent scope
        Scope *child;        // the child scope
    };

    struct Node
    {
    };

    class Parser
    {
    private:
        Scope global; // the first global scope
    public:
    };
};

#endif