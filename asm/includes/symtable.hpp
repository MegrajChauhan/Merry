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
#ifndef _SYMTABLE_
#define _SYMTABLE_

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include "defs.hpp"
#include "lexer.hpp"

namespace masm
{
    struct Variable
    {
        std::string name;
        std::string value;
        std::vector<Token> expr;
        bool is_expr = false;
        DataType type;
        size_t line, le;
        std::shared_ptr<std::string> file;
    };

    // changing this might be a good idea
    struct SymbolTable
    {
        std::unordered_map<std::string, Variable> vars;   // only variables
        std::unordered_map<std::string, Variable> _const_list; // only constants
    };
};

#endif