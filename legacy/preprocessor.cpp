#include "../includes/preprocessor.hpp"

masm::prep::Prep::Prep(std::string inp)
{
    reader.set_filename(inp);
}

std::string masm::prep::Prep::get_contents()
{
    return contents;
}

void masm::prep::Prep::prep()
{
    reader.open_file();
    auto line = reader.get_next_line();
    while (line.has_value())
    {
        // as the lines must start with the preprocessing symbols
        // we will make use of '$' for preprocessing
        std::string val = line.value();
        if (val[0] == '$')
        {
        }else
        {
            contents.append(val);
        }
    }
}