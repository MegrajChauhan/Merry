#ifndef _EXPR_
#define _EXPR_

#include "lexer.hpp"
#include "symtable.hpp"
#include <stack>

namespace masm
{
    class Expr
    {
        SymbolTable *table;
        std::vector<Token> expr;
        std::stack<double> operads;
        std::stack<TokenType> opers;

    public:
        Expr() = default;

        void add_expr(std::vector<Token> _e);

        void add_table(SymbolTable *t);

        std::optional<std::string> evaluate();

        bool perform();
    };
};

#endif