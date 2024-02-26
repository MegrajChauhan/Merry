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
                lexer.parse_err_expected_colon("after \'.data\'");
            }
            if (section == _SECTION_DATA)
            {
                lexer.parse_err_whole_line("Data section redefintiion when already in the data section.");
                break; // should not reach this point
            }
            section = _SECTION_DATA;
            next_token();
            break;
        }
        case lexer::_TT_SECTION_TEXT:
        {
            next_token();
            if (curr_tok.type != lexer::_TT_OPER_COLON)
            {
                lexer.parse_err_expected_colon("after \'.text\'");
            }
            if (section == _SECTION_TEXT)
            {
                lexer.parse_err_whole_line("Text section redefintiion when already in the text section.");
                break; // should not reach this point
            }
            section = _SECTION_TEXT;
            encountered_text = true;
            next_token();
            break;
        }
        case lexer::_TT_IDENTIFIER:
        {
            handle_identifier();
            if (nodes.back().kind != nodes::NodeKind::_LABEL)
                next_token();
            break;
        }
        case lexer::_TT_KEY_PROC:
        {
            if (section != _SECTION_TEXT)
            {
                lexer.parse_error("Declaring a procedure in the data section is not allowed");
                break;
            }
            handle_proc_declaration();
            next_token();
            break;
        }
        case lexer::_TT_INST_HLT:
        {
            if (section != _SECTION_TEXT)
            {
                lexer.parse_error("Using instructions in the data section is not allowed");
                break;
            }
            nodes.push_back(nodes::Node(nodes::_TYPE_INST, nodes::_INST_HLT, std::make_unique<nodes::Base>()));
            next_token();
            break;
        }
        case lexer::_TT_INST_MOV:
        {
            if (section != _SECTION_TEXT)
            {
                lexer.parse_error("Using instructions in the data section is not allowed");
                break;
            }
            handle_inst_mov();
            next_token();
            break;
        }
        default:
        {
            lexer.parse_error(
                "Expected an identifier name or a keyword");
        }
        }
    }
    if (!encountered_text)
    {
        std::cerr << "Parse Error: The input contains no text section which is not allowed." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void masm::parser::Parser::handle_inst_mov()
{
    // We have to identify what the variant is
    next_token();
    // This must be a register no matter what
    auto regr = nodes::_regr_iden_map.find(curr_tok.value);
    if (curr_tok.type != lexer::_TT_IDENTIFIER || regr == nodes::_regr_iden_map.end())
    {
        lexer.parse_error("Expected register name after 'mov' instruction.");
    }
    next_token();
    nodes::Node node;
    node.type = nodes::NodeType::_TYPE_INST;
    if (curr_tok.type == lexer::_TT_IDENTIFIER)
    {
        // then it could be a mov reg, reg
        // or it could be a variable as well
        auto iden2 = nodes::_regr_iden_map.find(curr_tok.value);
        if (iden2 == nodes::_regr_iden_map.end())
        {
            // it is a variable name
            node.kind = nodes::NodeKind::_INST_MOV_REG_IMM;
            node.ptr = std::make_unique<nodes::NodeInstMovRegImm>();
            auto temp = (nodes::NodeInstMovRegImm *)node.ptr.get();
            temp->is_iden = true;
            temp->dest_regr = regr->second;
            temp->value = curr_tok.value;
        }
        else
        {
            node.kind = nodes::NodeKind::_INST_MOV_REG_REG;
            node.ptr = std::make_unique<nodes::NodeInstMovRegReg>();
            auto temp = (nodes::NodeInstMovRegReg *)node.ptr.get();
            temp->src_reg = iden2->second;
            temp->dest_regr = regr->second;
        }
    }
    else if (curr_tok.type == lexer::_TT_INT)
    {
        // then we have an immediate here
        // in future based on the size of the number, we could encode mov64 instruction
        node.kind = nodes::NodeKind::_INST_MOV_REG_IMM;
        node.ptr = std::make_unique<nodes::NodeInstMovRegImm>();
        auto temp = (nodes::NodeInstMovRegImm *)node.ptr.get();
        temp->dest_regr = regr->second;
        temp->value = curr_tok.value;
    }
    else
    {
        lexer.parse_error("Expected an immediate value, register or a variable name.");
    }
    node.line = lexer.get_curr_line();
    nodes.push_back(node);
}

void masm::parser::Parser::handle_identifier()
{
    std::string name = curr_tok.value;
    next_token();

    if (curr_tok.type != lexer::_TT_OPER_COLON)
    {
        lexer.parse_err_expected_colon("after an identifier");
        return;
    }
    next_token();

    switch (curr_tok.type)
    {
    case lexer::_TT_KEY_DB:
    {
        if (section != _SECTION_DATA)
        {
            lexer.parse_error("Defining variables in the text section is not allowed");
        }
        handle_definebyte(name);
        break;
    }
    default:
    {
        // this is a label
        handle_label(name);
        break;
    }
    }
}

void masm::parser::Parser::handle_label(std::string label_name)
{
    nodes::Node node;
    node.type = nodes::_TYPE_INST;
    node.kind = nodes::_LABEL;
    node.ptr = std::make_unique<nodes::NodeLabel>();
    auto temp = (nodes::NodeLabel *)node.ptr.get();
    temp->label_name = label_name;
    node.line = lexer.get_curr_line();
    nodes.push_back(node);
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
    temp->byte_val = curr_tok.value;
    node.line = lexer.get_curr_line();
    nodes.push_back(node);
}

void masm::parser::Parser::handle_proc_declaration()
{
    next_token();
    if (curr_tok.type != lexer::_TT_IDENTIFIER)
    {
        lexer.parse_error("Expected a procedure name after the 'proc' keyword");
    }
    nodes::Node node;
    node.type = nodes::_TYPE_INST;
    node.kind = nodes::_PROC_DECLR;
    node.ptr = std::make_unique<nodes::NodeProcDeclr>();
    auto temp = (nodes::NodeProcDeclr *)node.ptr.get();
    temp->proc_name = curr_tok.value;
    node.line = lexer.get_curr_line();
    nodes.push_back(node);
}

std::vector<masm::nodes::Node> masm::parser::Parser::get_nodes()
{
    return nodes;
}