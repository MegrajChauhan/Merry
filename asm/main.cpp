#include "lexer.hpp"
#include "error.hpp"

int main()
{
    masm::Lexer l("test.txt");
    l.init_lexer();

    auto r = l.next_token();
    while (r.type != masm::TOK_EOF)
    {
        std::cout << r.val << '\n';
        if (r.type == masm::TOK_ERR)
        {
            break;
        }
        r = l.next_token();
    }
}