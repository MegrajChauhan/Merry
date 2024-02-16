#include "../includes/reader.hpp"

mlang::ReaderErrorNode::ReaderErrorNode(ReaderErrorKind kind, std::filesystem::path path)
{
    this->file_path = path;
    this->kind = kind;
}

void mlang::ReaderError::print_error(mlang::ReaderErrorNode error)
{
    switch (error.kind)
    {
    case mlang::DIRECTORY:
        std::cerr << "Read Error: The given file " << error.file_path.generic_string() << " is a directory.\n";
        break;
    case mlang::EMPTY:
        std::cerr << "Read Error: The given file " << error.file_path.generic_string() << " is a empty: Cannot be empty.\n";
        break;
    case mlang::FILE_NOT_FOUND:
        std::cerr << "Read Error: The given file " << error.file_path.generic_string() << " doesn't exist.\n";
        break;
    case mlang::INVALID_EXT:
        std::cerr << "Read Error: The given file " << error.file_path.generic_string() << " has invalid extension.\n";
        break;
    }
}

mlang::Reader::Reader(std::string filename)
{
    this->file_name = filename;
}

bool mlang::Reader::open_and_setup()
{
    if (!this->is_valid_filename())
    {
        this->error.print_error(ReaderErrorNode(INVALID_EXT, std::filesystem::path(std::filesystem::current_path() / this->file_name)));
        return false;
    }
    if (!this->does_exits_and_not_dir())
        return false;

    this->file_handle.open(this->file_name, std::ios::in);
    // the file must open
    this->current_line = 0;
    return true;
}

std::string mlang::Reader::next_line()
{
    if (this->file_handle.eof())
    {
        return "EOF";
    }
    this->current_line++;
    std::string read_line;
    std::getline(this->file_handle, read_line);
    return read_line;
}

size_t mlang::Reader::get_current_line_num()
{
    return this->current_line;
}

void mlang::Reader::set_filename(std::string filename)
{
    this->file_name = filename;
}

std::string mlang::Reader::get_file_name()
{
    return file_name;
}