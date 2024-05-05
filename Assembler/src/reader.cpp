#include "../includes/reader.hpp"

void masm::reader::Reader::print_error(masm::reader::ReaderErrorKind error)
{
    std::filesystem::path path = std::filesystem::current_path() / file_name;
    switch (error)
    {
    case masm::reader::DIRECTORY:
        std::cerr << "Read Error: The given file " << path.generic_string() << " is a directory.\n";
        break;
    case masm::reader::EMPTY:
        std::cerr << "Read Error: The given file " << path.generic_string() << " is a empty: Cannot be empty.\n";
        break;
    case masm::reader::FILE_NOT_FOUND:
        std::cerr << "Read Error: The given file " << path.generic_string() << " doesn't exist.\n";
        break;
    case masm::reader::INVALID_EXT:
        std::cerr << "Read Error: The given file " << path.generic_string() << " has invalid extension.\n";
        break;
    }
    exit(EXIT_FAILURE);
}

masm::reader::Reader::Reader(std::string filename)
{
    this->file_name = filename;
}

bool masm::reader::Reader::setup()
{
    if (!this->is_valid_filename())
    {
        print_error(INVALID_EXT);
    }
    this->does_exits_and_not_dir();
    return true;
}

void masm::reader::Reader::set_filename(std::string filename)
{
    this->file_name = filename;
}

std::string masm::reader::Reader::get_file_name()
{
    return file_name;
}

std::string masm::reader::Reader::read()
{
    std::string file_contents;
    std::fstream file(file_name, std::ios::in); // should work
    file.seekg(std::ios_base::cur, std::ios_base::end);
    // get the file's length
    std::streamsize len = file.tellg();
    // the temporary buffer
    // rewind
    file.seekg(std::ios_base::beg); // go back to the beginning
    // read
    file_contents.resize(len);
    file.read(&file_contents[0], len);
    // store the contents
    file.close();
    return file_contents;
}

void masm::reader::Reader::open_file()
{
    _f.open(file_name, std::ios::in);
}

std::optional<std::string> masm::reader::Reader::get_next_line()
{
    if (_f.eof())
    {
        _f.close();
        return {};
    }
    std::string line;
    std::getline(_f, line);
    return line;
}