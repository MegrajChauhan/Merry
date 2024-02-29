#ifndef _SEMA_
#define _SEMA_

#include "parser.hpp"
#include "symtable.hpp"
#include <memory>

/*
  Sema goes through the parsed nodes and creates a symbol table of data, labels and procedures.
  Then it goes through the instructions to see the instructions are valid or not.
  Then the code goes into codegen
*/

namespace masm
{
    namespace sema
    {
        class Sema
        {
            std::vector<std::unique_ptr<nodes::Node>> nodes;      // the parsed nodes
            std::vector<std::unique_ptr<nodes::Node>> inst_nodes; // the instruction nodes
            symtable::SymTable symtable;                          // the symbol table
            std::filesystem::path filepath;

        public:
            Sema() = default;

            Sema(parser::Parser &);

            void set_path(std::filesystem::path);

            void analyse();

            void analysis_error(size_t, std::string);

            void analysis_error(std::string);

            void pass_nodes(std::vector<std::unique_ptr<nodes::Node>> &dest)
            {
                dest = std::move(inst_nodes);
            }

            auto get_symtable() { return symtable; }

            // this makes sure that all the procedures have been defined
            void check_proc_declr();
        };
    };
};

#endif