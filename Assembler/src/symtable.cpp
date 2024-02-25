#include "../includes/symtable.hpp"

void masm::symtable::SymTable::add_entry(std::string key, SymTableEntry val)
{
    symtable[key] = val;
}

void masm::symtable::SymTable::add_entry(std::string key, SymEntryType type, std::string val, nodes::DataType dtype)
{
    symtable[key] = SymTableEntry(type, val, dtype);
}

std::unordered_map<std::string, masm::symtable::SymTableEntry>::iterator masm::symtable::SymTable::find_entry(std::string key)
{
    return symtable.find(key);
}

bool masm::symtable::SymTable::is_valid(std::unordered_map<std::string, SymTableEntry>::iterator iter)
{
    return iter == symtable.end();
}