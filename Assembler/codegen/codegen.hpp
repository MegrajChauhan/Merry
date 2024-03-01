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
            std::vector<unsigned char> inst_bytes;
            std::vector<unsigned char> data_bytes;
            std::unordered_map<std::string, size_t> data_addrs;
            std::unordered_map<std::string, size_t> label_addrs;

        public:
            Codegen() = default;

            // probably don't use this
            Codegen(std::vector<std::unique_ptr<nodes::Node>> &);

            Codegen(sema::Sema &);

            // generate data bytes
            // the strings is also data
            void gen_data();

            // generate bytes
            void gen();
        };
    };
};

#endif