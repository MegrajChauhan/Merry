#include "compunit.hpp"

void masm::CompUnit::set_filename(std::string name)
{
    fname = name;
}

bool masm::CompUnit::do_comp()
{
    return p.parse(fname);
}

std::vector<masm::CompUnit> *masm::get_unit()
{
    return &units;
}

void masm::add_unit(CompUnit *u)
{
    units.push_back(std::move(*u));
}

size_t masm::get_size()
{
    return units.size();
}
