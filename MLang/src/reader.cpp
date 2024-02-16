#include "../includes/reader.hpp"

void mlang::Reader::print_error(mlang::ReaderErrorKind error)
{
    std::filesystem::path path = std::filesystem::current_path() / file_name;
    switch (error)
    {
    case mlang::DIRECTORY:
        std::cerr << "Read Error: The given file " << path.generic_string() << " is a directory.\n";
        break;
    case mlang::EMPTY:
        std::cerr << "Read Error: The given file " << path.generic_string() << " is a empty: Cannot be empty.\n";
        break;
    case mlang::FILE_NOT_FOUND:
        std::cerr << "Read Error: The given file " << path.generic_string() << " doesn't exist.\n";
        break;
    case mlang::INVALID_EXT:
        std::cerr << "Read Error: The given file " << path.generic_string() << " has invalid extension.\n";
        break;
    }
    exit(EXIT_FAILURE);
}

mlang::Reader::Reader(std::string filename)
{
    this->file_name = filename;
}

bool mlang::Reader::setup()
{
    if (!this->is_valid_filename())
    {
        print_error(INVALID_EXT);
        return false;
    }
    if (!this->does_exits_and_not_dir())
        return false;
    return true;
}

void mlang::Reader::set_filename(std::string filename)
{
    this->file_name = filename;
}

std::string mlang::Reader::get_file_name()
{
    return file_name;
}

std::string mlang::Reader::read()
{
    std::string file_contents;
    std::fstream file(file_name, std::ios::in); // should work

    file.seekg(std::ios_base::cur, std::ios_base::end);
    // get the file's length
    size_t len = file.tellg();
    // the temporary buffer
    char buf[len];
    // rewind
    file.seekg(std::ios_base::beg); // go back to the beginning
    // read
    file.read(buf, len);
    // store the contents
    file_contents.assign(buf);

    file.close();
    return file_contents;
}
