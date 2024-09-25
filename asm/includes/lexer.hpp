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
#ifndef _LEXER_
#define _LEXER_

#include <string>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>
#include "error.hpp"
#include "defs.hpp"

#define _curr (*curr)

/**
 * Rules of an expression:
 * To be an expression, it must be within '[]'
 * The expression are normal within the '[]'
 */

namespace masm
{
    static std::unordered_map<std::string, TokenType>
        iden_map =
            {
                {"AND", OPER_AND},
                {"&", OPER_LAND},
                {"OR", OPER_OR},
                {"|", OPER_LOR},
                {"XOR", OPER_XOR},
                {"!", OPER_NOT},
                {"~", OPER_LNOT},
                {"<", OPER_LS},
                {">", OPER_RS},
                {"LT", OPER_LT},
                {"GT", OPER_GT},
                {"LE", OPER_LE},
                {"GE", OPER_GE},
                {"EQ", OPER_EQ},
                {"NEQ", OPER_NEQ},
                {"PTR", OPER_PTR},
                {"(", OPER_OPEN_PAREN},
                {")", OPER_CLOSE_PAREN},
                {"/", OPER_DIV},
                {"*", OPER_MUL},
                {"-", OPER_MINUS},
                {"+", OPER_PLUS},
                {"db", KEY_DB},
                {"dw", KEY_DW},
                {"dd", KEY_DD},
                {"dq", KEY_DQ},
                {"ds", KEY_DS},
                {"df", KEY_DF},
                {"dc", KEY_DC},
                {"dlf", KEY_DLF},
                {"rb", KEY_RB},
                {"rw", KEY_RW},
                {"rd", KEY_RD},
                {"rq", KEY_RQ},
                {"Ma", KEY_Ma},
                {"Mb", KEY_Mb},
                {"Mc", KEY_Mc},
                {"Md", KEY_Md},
                {"Me", KEY_Me},
                {"Mf", KEY_Mf},
                {"M1", KEY_M1},
                {"M2", KEY_M2},
                {"M3", KEY_M3},
                {"M4", KEY_M4},
                {"M5", KEY_M5},
                {"Mm1", KEY_Mm1},
                {"Mm2", KEY_Mm2},
                {"Mm3", KEY_Mm3},
                {"Mm4", KEY_Mm4},
                {"Mm5", KEY_Mm5},
                {"halt", INST_HLT},
                {"nop", INST_NOP},
                {"add", INST_ADD},
                {"sub", INST_SUB},
                {"mul", INST_MUL},
                {"div", INST_DIV},
                {"mod", INST_MOD},
                {"iadd", INST_IADD},
                {"isub", INST_ISUB},
                {"imul", INST_IMUL},
                {"idiv", INST_IDIV},
                {"imod", INST_IMOD},
                {"addf", INST_ADDF},
                {"subf", INST_SUBF},
                {"mulf", INST_MULF},
                {"divf", INST_DIVF},
                {"addlf", INST_ADDLF},
                {"sublf", INST_SUBLF},
                {"mullf", INST_MULLF},
                {"divlf", INST_DIVLF},
                {"mov", INST_MOV},
                {"movl", INST_MOVL},
                {"movb", INST_MOVB},
                {"movw", INST_MOVW},
                {"movd", INST_MOVD},
                {"moveb", INST_MOVEB},
                {"movew", INST_MOVEW},
                {"moved", INST_MOVED},
                {"movsxb", INST_MOVESXB},
                {"movsxw", INST_MOVESXW},
                {"movsxd", INST_MOVESXD},
                {"jmp", INST_JMP},
                {"call", INST_CALL},
                {"ret", INST_RET},
                {"sva", INST_SVA},
                {"svc", INST_SVC},
                {"sss", INST_SSS},
                {"gss", INST_GSS},
                {"popa", INST_POPA},
                {"pusha", INST_PUSHA},
                {"push", INST_PUSH},
                {"pop", INST_POP},
                {"not", INST_NOT},
                {"inc", INST_INC},
                {"dec", INST_DEC},
                {"and", INST_AND},
                {"or", INST_OR},
                {"xor", INST_XOR},
                {"lshift", INST_LSHIFT},
                {"rshift", INST_RSHIFT},
                {"cmp", INST_CMP},
                {"cmpf", INST_CMPF},
                {"cmplf", INST_CMPLF},
                {"gva", INST_GVA},
                {"lea", INST_LEA},
                {"loadb", INST_LOADB},
                {"loadw", INST_LOADW},
                {"loadd", INST_LOADD},
                {"loadq", INST_LOADQ},
                {"storeb", INST_STOREB},
                {"storew", INST_STOREW},
                {"stored", INST_STORED},
                {"storeq", INST_STOREQ},
                {"cmpxchg", INST_CMPXCHG},
                {"outr", INST_OUTR},
                {"uoutr", INST_UOUTR},
                {"cin", INST_CIN},
                {"cout", INST_COUT},
                {"sin", INST_SIN},
                {"sout", INST_SOUT},
                {"in", INST_IN},
                {"inw", INST_INW},
                {"ind", INST_IND},
                {"inq", INST_INQ},
                {"uin", INST_UIN},
                {"uinw", INST_UINW},
                {"uind", INST_UIND},
                {"uinq", INST_UINQ},
                {"out", INST_OUT},
                {"outw", INST_OUTW},
                {"outd", INST_OUTD},
                {"outq", INST_OUTQ},
                {"uout", INST_UOUT},
                {"uoutw", INST_UOUTW},
                {"uoutd", INST_UOUTD},
                {"uoutq", INST_UOUTQ},
                {"inf", INST_INF},
                {"inlf", INST_INLF},
                {"outf", INST_OUTF},
                {"outlf", INST_OUTLF},
                {"excgb", INST_EXCHGB},
                {"excgw", INST_EXCHGW},
                {"excgd", INST_EXCHGD},
                {"excgq", INST_EXCHGQ},
                {"cflags", INST_CFLAGS},
                {"reset", INST_RESET},
                {"clz", INST_CLZ},
                {"cln", INST_CLN},
                {"clc", INST_CLC},
                {"clo", INST_CLO},
                {"jnz", INST_JNZ},
                {"jz", INST_JZ},
                {"jne", INST_JNE},
                {"je", INST_JE},
                {"jnc", INST_JNC},
                {"jc", INST_JC},
                {"jno", INST_JNO},
                {"jo", INST_JO},
                {"jnn", INST_JNN},
                {"jn", INST_JN},
                {"jng", INST_JNG},
                {"jg", INST_JG},
                {"jns", INST_JNS},
                {"js", INST_JS},
                {"jge", INST_JGE},
                {"jse", INST_JSE},
                {"loop", INST_LOOP},
                {"intr", INST_INTR},
                {"sete", INST_SETE},
                {"calle", INST_CALLE},
                {"syscall", INST_SYSCALL},
                {"atm", KEY_ATM},
                {"proc", KEY_PROC},
                {"defined", KEY_DEFINED},
                {"ndefined", KEY_NDEFINED},
                {"end", KEY_END},
                {"entry", KEY_ENTRY},
                {"eepe", KEY_EEPE},
                {"teepe", KEY_TEEPE},
                {"depends", KEY_DEPENDS}
            };

    struct Token
    {
        std::string val;
        TokenType type;
        size_t line, col, le;
        std::vector<Token> expr;
    };

    static std::unordered_map<TokenType, Register>
        regr_map =
            {
                {KEY_Ma, Ma},
                {KEY_Mb, Mb},
                {KEY_Mc, Mc},
                {KEY_Md, Md},
                {KEY_Me, Me},
                {KEY_Mf, Mf},
                {KEY_M1, M1},
                {KEY_M2, M2},
                {KEY_M3, M3},
                {KEY_M4, M4},
                {KEY_M5, M5},
                {KEY_Mm1, Mm1},
                {KEY_Mm2, Mm2},
                {KEY_Mm3, Mm3},
                {KEY_Mm4, Mm4},
                {KEY_Mm5, Mm5},
    };

    class Lexer
    {
        std::shared_ptr<std::string> fileconts;
        std::shared_ptr<std::string> filepath;
        size_t line = 1, col = 0, offset = 0;
        size_t l, c;
        std::string::iterator curr, end;
        bool ignore_dots = false;

    public:
        Lexer() = default;

        void setup_lexer(std::shared_ptr<std::string> conts, std::shared_ptr<std::string> path);

        std::optional<Token> next_token(bool _is_expr = false);

        char peek(size_t by = 1);

        void consume();

        void rid_until(char until);

        std::string extract_line();

        Token group_word();

        std::optional<masm::Token> group_num();

        std::string get_a_group();

        bool eof();

        size_t get_line();

        size_t get_line_st();

        size_t get_col();

        size_t get_col_st();

        std::string get_from_line(size_t l, size_t e = 0);

        std::optional<masm::Token> gather_quote_string();

        void set_flag_ignore_dots(bool val);

        std::string read_hex(bool neg);

        std::string read_oct(bool neg);

        std::string read_bin(bool neg);
    };
};

#endif