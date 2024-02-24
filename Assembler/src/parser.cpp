#include "../includes/parser.hpp"

masm::parser::Parser::Parser(masm::lexer::Lexer lexer)
{
    this->lexer = lexer; // the lexer needs to be initialized here
    curr_tok = lexer.lex();
}

void masm::parser::Parser::setup_lexer(std::string filepath)
{
    // the return value of setup_reader doesn't really matter and I am lazy to make it void
    lexer.setup_reader(filepath);
    curr_tok = lexer.lex();
}

void masm::parser::Parser::parse()
{
    while (true)
    {
        if (curr_tok.type == lexer::_TT_EOF)
            break;
        switch (curr_tok.type)
        {
        case lexer::_TT_SECTION_DATA:
        {
            // we are currently entering the data section
            // if we are already in the data section, throw error
            next_token();
            if (curr_tok.type != lexer::_TT_OPER_COLON)
            {
                lexer.parse_err_expected_colon();
            }
            if (section == _SECTION_DATA)
            {
                lexer.parse_err_whole_line("Data section redefintiion when already in the data section.");
                break; // should not reach this point
            }
            section = _SECTION_DATA;
            break;
        }
        case lexer::_TT_SECTION_TEXT:
        {
            next_token();
            if (curr_tok.type != lexer::_TT_OPER_COLON)
            {
                lexer.parse_err_expected_colon();
            }
            if (section == _SECTION_TEXT)
            {
                lexer.parse_err_whole_line("Text section redefintiion when already in the text section.");
                break; // should not reach this point
            }
            section = _SECTION_TEXT;
            break;
        }
        case lexer::_TT_IDENTIFIER:
        {
            handle_identifier();
            break;
        }
        }
        next_token();
    }
}

void masm::parser::Parser::handle_identifier()
{
    std::string name = curr_tok.value;
    next_token();

    if (curr_tok.type != lexer::_TT_OPER_COLON)
    {
        lexer.parse_err_expected_colon();
        return;
    }

    next_token();

    switch (curr_tok.type)
    {
    case lexer::_TT_KEY_DB:
        handle_definebyte(name);
        break;
    }
}

void masm::parser::Parser::handle_definebyte(std::string name)
{
    next_token();

    if (curr_tok.type != lexer::_TT_INT)
    {
        lexer.parse_err_previous_token(curr_tok.value, std::string("Expected a number, got ") + curr_tok.value + " instead.");
    }
    nodes::Node node;

    node.type = nodes::_TYPE_DATA;
    node.kind = nodes::_DEF_BYTE;
    node.ptr = std::make_unique<nodes::NodeDefByte>();
    auto temp = (nodes::NodeDefByte *)node.ptr.get();
    temp->byte_name = name;
    temp->byte_val = (unsigned char)(std::stoll(curr_tok.value));
    nodes.push_back(node);
}

// {
//     nodes::Node curr_node;
//         case lexer::_TT_IDENTIFIER:
//         {
//             std::string name = curr_tok.value;
//             next_token();
//             if (curr_tok.type != lexer::_TT_OPER_COLON)
//             {
//                 // there must be a colon here
//                 lexer.parse_err_expected_colon();
//             }
//             next_token();
//             switch (curr_tok.type)
//             {
//             case lexer::_TT_KEY_DB:
//             {
//                 // then this is a define byte line where a byte is being defined
//                 // the next token must be a byte value
//                 next_token();
//                 curr_node.type = nodes::_TYPE_DATA;
//                 curr_node.kind = nodes::_DEF_BYTE;
//                 curr_node.details.def_byte.byte_name = name;
//                 curr_node.details.def_byte.byte_val = static_cast<unsigned char>(std::stoul(curr_tok.value));
//                 next_token();
//                 break;
//             }
//             }
//             break;
//         }
//         }
//         nodes.push_back(curr_node);
//         next_token();
//     }
// }

std::vector<masm::nodes::Node> masm::parser::Parser::get_nodes()
{
    return nodes;
}