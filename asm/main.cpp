#include "lexer.hpp"
#include "error.hpp"
#include "compunit.hpp"

int main()
{
    masm::CompUnit u;
    u.set_filename("test.txt");
    u.do_comp();
    auto r = masm::get_unit();
    auto s = masm::get_size();
}