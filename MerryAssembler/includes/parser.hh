#if !defined(_MERRY_ASM_PARSER_)
#define _MERRY_ASM_PARSER_
#include "lexer.hh"
#include "ast.hh"
#include <variant>

namespace merry{
namespace front_end{
    class Parser{
        private:
            Lexer _lexer;
        public:
            Parser(Lexer lexer) :_lexer(lexer) {}
            Ast get_ast();
            std::variant<AstNodeInst, AstNodeLabel> get_next_node();
    };
};
};

#endif // _MERRY_ASM_PARSER_