#include "../includes/reader.hpp"

int main(int argc, char **argv)
{
    mlang::Reader reader(argv[1]);
    if (!reader.open_and_setup())
    {
        std::cerr << "Couldn't setup" << std::endl;
        return -1;
    }
    std::string line;
    while ((line = reader.next_line()) != "EOF")
    {
        std::cout << line<<std::endl;
    }
}