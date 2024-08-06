#include "parser.hpp"

bool masm::Parser::parse(std::string fname)
{
    if (!l.validate_file(fname))
        return false;
    if (!l.init_lexer())
        return false;
    Token t = l.next_token();

    // for the parser as well
    std::string path = (std::filesystem::current_path() / fname).string();

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
            n.kind = _INC_FILE;
            n.node = std::make_unique<NodeIncFile>();
            ((NodeIncFile*)n.node.get())->ind = units.size() - 1;
            n.st_col = 0;
            n.ed_col = 3;
            break;
        }
        }
        t = l.next_token();
        nodes.push_back(std::move(n));
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