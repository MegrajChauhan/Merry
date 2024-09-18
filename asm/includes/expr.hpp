#ifndef _EXPR_
#define _EXPR_

#include "lexer.hpp"
#include "symtable.hpp"
#include <stack>

namespace masm
{
    union F32
    {
        uint32_t _integer;
        float _float;
    };

    union F64
    {
        uint64_t _integer;
        double _double;
    };
    class Expr
    {
        SymbolTable *table;
        std::vector<Token> expr;
        std::stack<double> operads;
        std::stack<TokenType> opers;
        std::unordered_map<std::string, size_t> *data_addr;
        bool _was_addr = false;

    public:
        Expr() = default;

        void add_expr(std::vector<Token> _e);

        void add_table(SymbolTable *t);

        void add_addr(std::unordered_map<std::string, size_t> *addr);

        std::optional<std::string> evaluate(bool _only_const = false);

        bool perform();

        bool was_addr();
    };
};

#endif