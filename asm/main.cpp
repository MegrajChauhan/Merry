#include "lexer.hpp"
#include "error.hpp"
#include "compunit.hpp"

int main()
{
    masm::CompUnit u;
    u.set_filename("test.txt");
    u.do_comp();
}