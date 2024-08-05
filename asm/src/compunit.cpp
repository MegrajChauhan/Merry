#include "compunit.hpp"

void masm::CompUnit::add_parent(CompUnit *u)
{
    parent = u;
}

void masm::CompUnit::set_filename(std::string name)
{
    fname = name;
}

bool masm::CompUnit::do_comp()
{
    return p.parse(fname);
}