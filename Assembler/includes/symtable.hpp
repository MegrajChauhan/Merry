#ifndef _SYMTABLE_
#define _SYMTABLE_

#include <unordered_map>
#include <string>
#include "nodes.hpp"

namespace masm
{
    namespace symtable
    {
        enum SymEntryType
        {
            _VAR,   // the entry is a variable
            _PROC,  // the entry is a procedure
            _LABEL, // the entry is a label
        };

        // a symtable entry
        struct SymTableEntry
        {
            SymEntryType type;
            std::string value;     // if variable, hold the value of the variable
            size_t len = 0;
            nodes::DataType dtype; // the data's type[for variables]
            bool defined = false;  // only for some type of entries
            // [in future: for procedures, they could be private to a single file or to multiple file]

            SymTableEntry() = default;

            SymTableEntry(SymEntryType t, std::string val = "", nodes::DataType type = nodes::DataType::_TYPE_BYTE, size_t len = 0) : type(t), value(val), dtype(type), len{len} {}
        };

        // the symbol table
        class SymTable
        {
            // a variable name or a procedure name will identify the entry
            std::unordered_map<std::string, SymTableEntry> symtable;

        public:
            SymTable() = default;

            void add_entry(std::string, SymTableEntry);
            void add_entry(std::string, SymEntryType, std::string, nodes::DataType);

            std::unordered_map<std::string, SymTableEntry>::iterator find_entry(std::string);

            bool is_invalid(std::unordered_map<std::string, SymTableEntry>::iterator);

            auto begin() { return symtable.begin(); }

            auto end() { return symtable.end(); }
        };
    };
};

#endif