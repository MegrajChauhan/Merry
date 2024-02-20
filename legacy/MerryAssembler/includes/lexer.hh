#if !defined(_MERRY_ASM_LEXER_)
#define _MERRY_ASM_LEXER_
#include "core.hh"
#include "token.hh"
#include <vector>

namespace merry{
namespace front_end{
    class Lexer{
        private:
            // Advance index by one no mater what
            void advance();
            // Skip any characters that we can skip
            void skip_whitespace();
            bool isskip() { return (_c == ' ' || _c == '\n' || _c == '\r' || _c == '\t' || _c == '\v') && _c != '\0'; }
            // Current location
            core::Loc loc;
            // Content itself
            std::string _contents;
            // Index in content
            size_t idx;
            // Current character
            char _c;

        public:
            Lexer(std::string contents, std::string file_name) :loc(core::Loc(0, 0, file_name)), _contents(contents), idx(0), _c(contents.at(0)){}
            std::vector<Token> lex_all();
            Token next_token();
            Token peek();
    };
};
};

#endif // _MERRY_ASM_LEXER_
