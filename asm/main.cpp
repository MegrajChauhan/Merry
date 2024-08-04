#include <iostream>
#include "lexer.hpp"
#include <fstream>

int main()
{
    std::string fname = "test.txt";
    std::fstream i(fname, std::ios::in);
    masm::Lexer l;
    size_t lno = 0;
    auto f = std::make_shared<std::string>(fname);
    auto li = std::make_shared<std::string>();

    while (!i.eof())
    {
        std::getline(i, *li.get());
        auto r = l.lex_line(f, li, lno);
        lno++;
        if (r.has_value())
        {
            std::cout << "No error.\n";
            for (auto t : r.value())
            {
                std::cout << "type= " << t.type << " | value= " << t.val << " | line= " << t.p.line << " | col_st= " << t.p.col_st << " | col_end= " << t.p.col_end << " | file= " << fname << '\n';
            }
        }
    }
}