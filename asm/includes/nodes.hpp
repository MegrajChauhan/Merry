#ifndef _NODES_
#define _NODES_

#include <memory>
#include <string>

namespace masm
{
    enum NodeKind
    {
    };

    struct Base
    {
    };

    struct Node
    {
        std::unique_ptr<Base> node;
        NodeKind kind;
        size_t st_line, ed_line;
        size_t st_col, ed_col;
    };
};

#endif