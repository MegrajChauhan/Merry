#include "../includes/preprocessor_syms.hpp"

void masm::prepsyms::PrepSyms::push_sym(std::string key, std::string value)
{
    _syms_[key] = value;
}

std::string masm::prepsyms::PrepSyms::operator[](std::string key)
{
    return _syms_[key];
}