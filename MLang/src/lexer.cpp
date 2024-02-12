#include "../includes/lexer.hpp"

MLang::Token::Token(TokenType type, std::string val)
{
    this->type = type;
    this->value = val;
}

MLang::Lexer::Lexer(std::string filename)
{
    this->pos.set_filename(filename); // set the filename
    // we can't open it here
    this->filename = filename;
}

bool MLang::Lexer::open_file_for_lexing()
{
    std::filesystem::path path = std::filesystem::current_path() / this->filename; // get the path
    if (!std::filesystem::exists(path))
    {
        this->error = DOES_NOT_EXIST; // it doesn't exist
        return false;
    }
    if (std::filesystem::is_directory(path))
    {
        this->error = IS_DIR; // is a directory
        return false;
    }
    // if the file is empty
    if (std::filesystem::file_size(path) == 0)
    {
        this->error = FILE_EMPTY;
        return false;
    }
    std::fstream file;
    file.open(this->filename, std::ios::in); // open for input
    if (!file.is_open())
    {
        this->error = FAILED_TO_OPEN;
        return false;
    }
    // now we need to get the contents and then we can close the file safely

    // seek to the end
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
    this->filecontents.assign(buf);
    // close the file
    file.close();
    // set the iterator
    this->iter = this->filecontents.begin(); // set the iterator
    return true;                             // everything went smoothly
}

MLang::Token MLang::Lexer::lex()
{
    TokenType type = _TT_ERR;

    return Token(type, " "); // the lexed token
}