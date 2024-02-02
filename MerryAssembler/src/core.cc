#include "../includes/core.hh"
#include <fstream>

std::string merry::core::readFile(std::string path){
    std::ifstream file(path); // Open file for reading

    if (!file.is_open()) {
        std::cerr << "Unable to open file\n";
        exit(1);
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        (std::istreambuf_iterator<char>()));
    return content;
}

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