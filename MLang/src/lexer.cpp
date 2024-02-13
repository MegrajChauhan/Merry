#include "../includes/lexer.hpp"
#include <iostream>

MLang::Token::Token(TokenType type, std::string val)
{
    this->type = type;
    this->value = val;
}

MLang::Lexer::Lexer(std::string filename)
{
    // this->pos.set_filename(filename); // set the filename
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
    this->iter = this->filecontents.begin();  // set the iterator
    this->enditer = this->filecontents.end(); // the end iterator
    return true;                              // everything went smoothly
}

MLang::Token MLang::Lexer::lex()
{
    TokenType type = _TT_ERR;
    std::string val = "";
    std::string temp;
    // check if the current character is an operator
    if (is_oper(*this->iter))
    {
        // we have ourselves an operator
        // since we do not have multi-character operators, we can ignore it for now
        temp.push_back(*this->iter);
        auto _token = MLang::_iden_map_.find(temp);
        if (_token == MLang::_iden_map_.end())
        {
            // this is an error as this is an invalid
            std::cerr << "Invalid token " << *this->iter << "\n";
            abort();
        }
        else
        {
            type = (*_token).second; // we have the type
            // as this is an operator, we do not need the string value
        }
        goto done; // we should be good to go
    }
    if (is_num(*this->iter))
    {
        
    }

done:
    return Token(type, val); // the lexed token
}

MLang::LexErr MLang::Lexer::get_error()
{
    return this->error; // the calle needs to identify the errors
}