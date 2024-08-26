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
        KEY_DC,
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
        INST_MOVL,
        INST_MOVB,
        INST_MOVW,
        INST_MOVD,
        INST_MOVESXB,
        INST_MOVESXW,
        INST_MOVESXD, // there is no need for 64-bit which should be obvious
        INST_JMP,
        INST_CALL,
        INST_RET,
        INST_SVA,
        INST_SVC,
        INST_PUSHA,
        INST_POPA,
        INST_PUSH,
        INST_POP,
        INST_NOT,
        INST_INC,
        INST_DEC,
        INST_AND,
        INST_OR,
        INST_XOR,
        INST_LSHIFT,
        INST_RSHIFT,
        INST_CMP,
        INST_LEA,
        INST_LOADB,
        INST_STOREB,
        INST_LOADW,
        INST_STOREW,
        INST_LOADD,
        INST_STORED,
        INST_LOADQ,
        INST_STOREQ,
        INST_CMPXCHG,
        INST_OUTR,
        INST_UOUTR,
        INST_CIN,
        INST_COUT,
        INST_SIN,
        INST_SOUT,
        INST_IN,
        INST_INW,
        INST_IND,
        INST_INQ,
        INST_UIN,
        INST_UINW,
        INST_UIND,
        INST_UINQ,
        INST_OUT,
        INST_OUTW,
        INST_OUTD,
        INST_OUTQ,
        INST_UOUT,
        INST_UOUTW,
        INST_UOUTD,
        INST_UOUTQ,
        INST_INF,
        INST_OUTF,
        INST_INLF,
        INST_OUTLF,
        INST_EXCHGB,
        INST_EXCHGW,
        INST_EXCHGD,
        INST_EXCHGQ,
        INST_MOVEB,
        INST_MOVEW,
        INST_MOVED,
        INST_CFLAGS,
        INST_RESET,
        INST_CLZ,
        INST_CLN,
        INST_CLC,
        INST_CLO,
        INST_JNZ,
        INST_JZ,
        INST_JNE,
        INST_JE,
        INST_JNC,
        INST_JC,
        INST_JNO,
        INST_JO,
        INST_JNN,
        INST_JN,
        INST_JNG,
        INST_JG,
        INST_JNS,
        INST_JS,
        INST_JGE,
        INST_JSE,
        INST_LOOP,
        INST_INTR,
        INST_SETE,
        INST_SYSCALL,
        INST_CALLE,
        KEY_ATM,
        KEY_DEPENDS,
        KEY_DEFINED,
        KEY_NDEFINED,
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
                {"movsxb", INST_MOVESXB},
                {"movsxw", INST_MOVESXW},
                {"movsxd", INST_MOVESXD},
                {"jmp", INST_JMP},
                {"call", INST_CALL},
                {"ret", INST_RET},
                {"sva", INST_SVA},
                {"svc", INST_SVC},
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
                {"depends", KEY_DEPENDS}};

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