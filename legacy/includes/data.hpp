#ifndef _DATA_
#define _DATA_

#include "lexer.hpp"
#include "symtable.hpp"

namespace masm
{
    class Data
    {
        Lexer l;
        std::shared_ptr<std::string> file;
        std::string fname;
        SymbolTable *sym;
        bool read_again = true;
        Token old_tok; // resuse token

    public:
        Data() = default;

        void setup_for_read(SymbolTable *t, std::shared_ptr<std::string> f, std::shared_ptr<std::string> fconts);

        bool read_data();

        bool handle_defines(DataType t, bool _const = false);

        bool handle_strings();

        bool add_variable(Variable v);

        bool add_const(Variable v);
    };
};

#endif