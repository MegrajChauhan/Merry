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
#ifndef _NODES_
#define _NODES_

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include "defs.hpp"
#include "lexer.hpp"

namespace masm
{
    struct Base
    {
    };

    struct NodeName : public Base
    {
        std::variant<std::string, Register> oper;
    };

    struct NodeArithmetic : public Base
    {
        bool is_signed = false; // Is signed arithmetic?
        Register reg;
        std::variant<Register, std::string, std::vector<Token>> second_oper;
    };

    struct NodeTwoRegr : public Base
    {
        Register r1, r2;
    };

    struct NodeMov : public Base
    {
        Register reg;
        std::variant<Register, std::string, std::vector<Token>> second_oper;
        bool is_float = false; // just for the mov instruction
        bool is_lbl = false;
    };

    struct NodeStack : public Base
    {
        Register reg;
        std::variant<Register, std::string, std::vector<Token>> second_oper;
        bool _is_lbl;
    };

    struct NodeLea : public Base
    {
        Register dest, base, ind, scale;
    };

    struct NodeIntr : public Base
    {
        std::variant<std::vector<Token>, std::string> val;
    };

    struct NodeCmpxchg : public Base
    {
        Register reg1, reg2;
        std::variant<Register, std::string> var;
    };

    typedef NodeName NodeIO;
    typedef NodeTwoRegr NodeExcg;
    typedef NodeMov NodeLogical;
    typedef NodeMov NodeLoadStore;

    struct Node
    {
        NodeKind kind;
        std::unique_ptr<Base> node;
        std::shared_ptr<std::string> _file;
        size_t line;
    };

};

#endif