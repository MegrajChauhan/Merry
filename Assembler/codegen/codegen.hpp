#ifndef _CODEGEN_
#define _CODEGEN_

#include "../includes/sema.hpp"

namespace masm
{
    namespace codegen
    {
        class Codegen
        {
            symtable::SymTable table;
            std::vector<std::unique_ptr<nodes::Node>> inst_nodes;

        public:
            Codegen() = default;

            Codegen(std::vector<std::unique_ptr<nodes::Node>> &);

            Codegen(sema::Sema&);
        };
    };
};

#endif