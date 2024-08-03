#include "lexer.hpp"

masm::Lexer::Lexer(std::shared_ptr<std::string> _conts)
{
    _file_contents = _conts;
    iter = _file_contents.get()->begin();
}