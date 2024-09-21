/*
 * MIT License
 *
 * Copyright (c) 2024 MegrajChauhan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
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