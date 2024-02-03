#include "../includes/core.hh"
#include <fstream>

void merry::core::Loc::update(int c){
    if (c == '\n')
    {
        col = 0;
        row++;
    }
    else if (c == '\r')
    {
        col = 0;
    }
    else if (c == '\t')
    {
        col += 4;
    }
    else
    {
        col++;
    }

}