#if !defined(_MERRY_ASM_TOKEN_)
#define _MERRY_ASM_TOKEN_
#include "core.hh"

namespace merry{
namespace front_end{
    enum struct TokenType{
        TT_EOF,
        NUMBER,
        ID,
        COLON=':',
        INVALID,
    };
    class Token{
        private:
            core::Loc current_loc;
            TokenType _type;
            std::string _data;
        public:
            Token(TokenType type, std::string data, core::Loc _current_loc) :current_loc(_current_loc), _type(type), _data(data) {}
            void print();
            TokenType get_type() { return _type; }
            std::string get_data() { return _data; }
    };
};
};

#endif // _MERRY_ASM_TOKEN_