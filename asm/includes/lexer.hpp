#ifndef _LEXER_
#define _LEXER_

#include <string>
#include <memory>

namespace masm
{
    class Lexer
    {
        std::shared_ptr<std::string> _file_contents = nullptr;
        std::string::iterator iter;

    public:
        Lexer() = default;
        
        Lexer(std::shared_ptr<std::string> _conts);
    };
};

#endif