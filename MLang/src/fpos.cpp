#include "../includes/fpos.hpp"

MLang::FStatus::FStatus(std::string name, size_t col, size_t line)
{
    this->col = col;
    this->file_name = name;
    this->line = line;
}

void MLang::FPos::update_pos(int current_char)
{
    if (current_char == '\n')
    {
        this->col = 0;   // reset the column
        this->line_no++; // we move to the next line
    }
    else
    {
        /// TODO: Add column limit test here
        this->col++;
    }
}

void MLang::FPos::set_filename(std::string file_name)
{
    this->_file_name = file_name;
}

MLang::FStatus MLang::FPos::get_current_status()
{
    return MLang::FStatus(this->_file_name, this->col, this->line_no); // get the current status
}