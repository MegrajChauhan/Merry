#include "lexer.hpp"
#include "error.hpp"
#include "compunit.hpp"

int main()
{
    // masm::CompUnit u;
    // u.set_filename("test.txt");
    // u.do_comp();
    // auto r = masm::get_unit();
    // auto s = masm::get_size();
    masm::Lexer l;
    l.validate_file("test.txt");
    l.init_lexer();
    auto r = l.next_token();
    while(r.type != masm::TOK_ERR && r.type != masm::TOK_EOF)
    {
        r = l.next_token();
    }
}