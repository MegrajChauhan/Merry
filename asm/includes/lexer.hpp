#ifndef _LEXER_
#define _LEXER_

#include <string>
#include <memory>
#include <optional>
#include <unordered_map>
#include "error.hpp"
#include "nodes.hpp"

#define c *curr

namespace masm
{
    enum TokenType
    {
        MB_C, // pseudotoken
        MB_DATA,
        MB_CODE,
        IDENTIFIER,
        NUM_INT,
        NUM_FLOAT,
        STR,
        KEY_DB,
        KEY_DW,
        KEY_DD,
        KEY_DQ,
        KEY_DS,
        KEY_DF,
        KEY_DLF,
        KEY_RB,
        KEY_RW,
        KEY_RD,
        KEY_RQ,
        OPER_DOT, // a pseudotoken
        KEY_PROC,
        KEY_Ma,
        KEY_Mb,
        KEY_Mc,
        KEY_Md,
        KEY_Me,
        KEY_Mf,
        KEY_M1,
        KEY_M2,
        KEY_M3,
        KEY_M4,
        KEY_M5,
        KEY_Mm1,
        KEY_Mm2,
        KEY_Mm3,
        KEY_Mm4,
        KEY_Mm5,
        INST_NOP,
        INST_HLT,
        INST_ADD,
        INST_SUB,
        INST_MUL,
        INST_DIV,
        INST_MOD,
        INST_IADD,
        INST_ISUB,
        INST_IMUL,
        INST_IDIV,
        INST_IMOD,
        INST_ADDF,
        INST_SUBF,
        INST_MULF,
        INST_DIVF,
        INST_ADDLF,
        INST_SUBLF,
        INST_MULLF,
        INST_DIVLF,
        INST_MOV,
    };

    static std::unordered_map<std::string, TokenType>
        iden_map =
            {
                {"data", MB_DATA},
                {"code", MB_CODE},
                {"db", KEY_DB},
                {"dw", KEY_DW},
                {"dd", KEY_DD},
                {"dq", KEY_DQ},
                {"ds", KEY_DS},
                {"df", KEY_DF},
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
                {"proc", KEY_PROC}};

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

    struct Token
    {
        std::string val;
        TokenType type;
        size_t line, col;
    };

    class Lexer
    {
        std::shared_ptr<std::string> fileconts;
        std::shared_ptr<std::string> filepath;
        size_t line = 0, col = 0, offset = 0;
        std::string::iterator curr, end;
        bool ignore_dots = false;

    public:
        Lexer() = default;

        void setup_lexer(std::shared_ptr<std::string> conts, std::shared_ptr<std::string> path);

        std::optional<Token> next_token();

        char peek(size_t by = 1);

        void consume();

        void rid_until(char until);

        std::string extract_line();

        Token group_word();

        std::optional<masm::Token> group_num();

        std::string get_a_group();

        bool eof();

        size_t get_line();

        size_t get_col();

        std::string get_from_line(size_t l);

        std::optional<masm::Token> gather_quote_string();

        void set_flag_ignore_dots(bool val);
    };
};

#endif