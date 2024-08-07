#include "parser.hpp"

/**
 * Just like before. To C++ enthusiasts and clean coders: Don't mind the messy code along with the violation of
 * DRY principle. This shouldn't concern you unless you are a contributor but just in case you are offended, you are
 * free to make positive changes if you can deciper how it works or even better, write the assembler yourself for yourself.
 * PLEASE DON'T MIND IT!
 */

bool masm::Parser::parse(std::string fname)
{
    if (!l.validate_file(fname))
        return false;
    if (!l.init_lexer())
        return false;
    t = l.next_token();

    // for the parser as well
    path = (std::filesystem::current_path() / fname).string();

    while (t.type != TOK_EOF)
    {
        Node n;
        if (t.type == TOK_ERR)
            return false;
        switch (t.type)
        {
        case TOK_USE:
        {
            if (!new_file())
            {
                // the necessary messages will be printed, we just need to add some here
                note(path, l.get_line(), "Included from here.", l.extract_current_line());
                return false;
            }
            n.st_line = t.loc.line;
            n.ed_line = l.get_line();
            n.st_off = t.loc.offset;
            n.ed_off = l.get_off();
            n.kind = _INC_FILE;
            n.node = std::make_unique<NodeIncFile>();
            ((NodeIncFile *)n.node.get())->ind = units.size() - 1;
            nodes.push_back(std::move(n));
            break;
        }
        case TOK_BYTE:
        case TOK_WORD:
        case TOK_DWORD:
        case TOK_QWORD:
        {
            if (!variable_declaration())
                return false;
            break;
        }
        }
        if (read_again)
            t = l.next_token();
        else
            read_again = true;
    }
    return true;
}

bool masm::Parser::new_file()
{
    CompUnit unit;
    // Create a new compilation unit and add it to the list to be compiled
    std::string inc_file = l.extract_just_text();
    unit.set_filename(inc_file);
    add_unit(&unit);
    return true;
}

bool masm::Parser::variable_declaration()
{
    Token temp = t;
    Token id = l.next_token();
    if (id.type != TOK_ID)
    {
        if (id.type == TOK_EOF)
        {
            elog(path, l.get_line(), id.loc.col_st, l.get_col(), "FATAL", "Expected an ID but reached EOF.", l.extract_current_line());
            return false;
        }
        elog(path, l.get_line(), id.loc.col_st, l.get_col(), "FATAL", "Expected an ID got \'" + id.val + "\' instead.", l.extract_current_line());
        return false;
    }
    if (l.next_token().type != TOK_SEMI)
    {
        elog(path, l.get_line(), l.get_col() - 1, l.get_col(), "FATAL", "Syntax Error: Expected ':' here.", l.extract_current_line());
        return false;
    }
    std::vector<Token> expr;
    Token val = l.next_token();
    while (val.type == TOK_NUM || val.type == TOK_ID)
    {
        expr.push_back(val);
        val = l.next_token();
    }
    if (expr.size() == 0)
    {
        if (val.type == TOK_EOF)
        {
            elog(path, l.get_line(), val.loc.col_st, l.get_col(), "FATAL", "Expected a NUM but reached EOF.", l.extract_current_line());
            return false;
        }
        elog(path, l.get_line(), val.loc.col_st, l.get_col(), "FATAL", "Expected a NUM got \'" + id.val + "\' instead.", l.extract_current_line());
        return false;
    }
    // Everything we need
    Node n;
    n.node = std::make_unique<NodeVarDeclr>();
    auto x = (NodeVarDeclr *)(n.node.get());
    x->expr = expr;
    x->is_const = false;
    x->name = id.val;
    x->type = get_datatype(temp.type);
    read_again = false;

    n.ed_line = expr[expr.size() - 1].loc.line;
    n.st_line = temp.loc.line;
    n.st_off = temp.loc.offset;
    n.ed_off = expr[expr.size() - 1].loc.offset;

    n.kind = _VAR_DECLR;
    nodes.push_back(std::move(n));
    t = val;
    return true;
}

masm::DataType masm::Parser::get_datatype(TokenType t)
{
    switch (t)
    {
    case TOK_BYTE:
    case TOK_CBYTE:
    case TOK_RBYTE:
        return BYTE;
    case TOK_WORD:
    case TOK_CWORD:
    case TOK_RWORD:
        return WORD;
    case TOK_DWORD:
    case TOK_CDWORD:
    case TOK_RDWORD:
        return DWORD;
    case TOK_QWORD:
    case TOK_CQWORD:
    case TOK_RQWORD:
        return QWORD;
    case TOK_FLOAT:
    case TOK_CFLOAT:
        return FLOAT;
    case TOK_LFLOAT:
    case TOK_CLFLOAT:
        return LFLOAT;
    case TOK_STRING:
    case TOK_CSTRING:
        return ARRAY;
    }
}