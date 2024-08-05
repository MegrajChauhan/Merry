#ifndef _NODES_
#define _NODES_

#include <memory>
#include <string>

namespace masm
{
    enum NodeKind
    {
        _INC_FILE,
    };

    struct Base
    {
    };

    struct NodeIncFile: public Base
    {
        size_t ind;
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