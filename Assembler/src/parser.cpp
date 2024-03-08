/*




Big apologies to experienced C++ programmers for writing such code in C++.
Forgive the absurd redundancy.
Logical errors.
The utterly wrong use of the standard library
Ignoring the better ways for doing this
I am in a rush so this has to be messy




*/

#include "../includes/parser.hpp"

masm::parser::Parser::Parser(masm::lexer::Lexer &lexer)
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

void masm::parser::Parser::move_nodes(std::vector<std::unique_ptr<nodes::Node>> &dest)
{
    dest = std::move(nodes);
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
            next_token(); // i can't remember the reason why I moved this line back into each case statements instead of having just one at the end
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
            if (nodes.back()->kind != nodes::NodeKind::_LABEL)
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
        case lexer::_TT_INST_NOP:
        {
            if (section != _SECTION_TEXT)
            {
                lexer.parse_error("Using instructions in the data section is not allowed");
                break;
            }
            nodes.push_back(std::make_unique<nodes::Node>(nodes::Node(nodes::_TYPE_INST, nodes::_INST_NOP, std::make_unique<nodes::Base>())));
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
            nodes.push_back(std::make_unique<nodes::Node>(nodes::Node(nodes::_TYPE_INST, nodes::_INST_HLT, std::make_unique<nodes::Base>())));
            next_token();
            break;
        }
        case lexer::_TT_INST_MOVQ:
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
        case lexer::_TT_INST_MOVB:
        case lexer::_TT_INST_MOVW:
        case lexer::_TT_INST_MOVD:
        {
            if (section != _SECTION_TEXT)
            {
                lexer.parse_error("Using instructions in the data section is not allowed");
                break;
            }
            handle_inst_movX();
            next_token();
            break;
        }
        case lexer::_TT_INST_MOVEB:
        case lexer::_TT_INST_MOVEW:
        case lexer::_TT_INST_MOVED:
        {
            if (section != _SECTION_TEXT)
            {
                lexer.parse_error("Using instructions in the data section is not allowed");
                break;
            }
            handle_inst_moveX();
            next_token();
            break;
        }
        case lexer::_TT_INST_MOVSXB:
        case lexer::_TT_INST_MOVSXW:
        case lexer::_TT_INST_MOVSXD:
        {
            if (section != _SECTION_TEXT)
            {
                lexer.parse_error("Using instructions in the data section is not allowed");
                break;
            }
            handle_inst_movsx();
            next_token();
            break;
        }
        case lexer::_TT_INST_OUTR:
        {
            if (section != _SECTION_TEXT)
            {
                lexer.parse_error("Using instructions in the data section is not allowed");
                break;
            }
            nodes.push_back(std::make_unique<nodes::Node>(nodes::NodeType::_TYPE_INST, nodes::NodeKind::_INST_OUTR, std::move(std::make_unique<nodes::Base>()), lexer.get_curr_line()));
            next_token();
            break;
        }
        case lexer::_TT_INST_UOUTR:
        {
            if (section != _SECTION_TEXT)
            {
                lexer.parse_error("Using instructions in the data section is not allowed");
                break;
            }
            nodes.push_back(std::make_unique<nodes::Node>(nodes::NodeType::_TYPE_INST, nodes::NodeKind::_INST_UOUTR, std::move(std::make_unique<nodes::Base>()), lexer.get_curr_line()));
            next_token();
            break;
        }
        case lexer::_TT_INST_CIN:
        {
            if (section != _SECTION_TEXT)
            {
                lexer.parse_error("Using instructions in the data section is not allowed");
                break;
            }
            handle_inst_cin();
            next_token();
            break;
        }
        case lexer::_TT_INST_SIN:
        {
            if (section != _SECTION_TEXT)
            {
                lexer.parse_error("Using instructions in the data section is not allowed");
                break;
            }
            handle_inst_sin();
            next_token();
            break;
        }
        case lexer::_TT_INST_COUT:
        {
            if (section != _SECTION_TEXT)
            {
                lexer.parse_error("Using instructions in the data section is not allowed");
                break;
            }
            handle_inst_cout();
            next_token();
            break;
        }
        case lexer::_TT_INST_SOUT:
        {
            if (section != _SECTION_TEXT)
            {
                lexer.parse_error("Using instructions in the data section is not allowed");
                break;
            }
            handle_inst_sout();
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

void masm::parser::Parser::handle_inst_movsx()
{
    size_t len = curr_tok.type == lexer::_TT_INST_MOVSXB ? 1 : curr_tok.type == lexer::_TT_INST_MOVSXW ? 2
                                                                                                       : 4;
    next_token();
    nodes::NodeKind kind;
    auto regr = nodes::_regr_iden_map.find(curr_tok.value);
    if (regr == nodes::_regr_iden_map.end())
    {
        lexer.parse_error("Expected register name after 'movsxX' instruction.");
    }
    next_token();
    std::unique_ptr<nodes::Base> ptr;
    if (curr_tok.type == lexer::_TT_IDENTIFIER)
    {
        auto iden2 = nodes::_regr_iden_map.find(curr_tok.value);
        if (iden2 == nodes::_regr_iden_map.end())
        {
            kind = len == 1 ? nodes::NodeKind::_INST_MOVSX_REG_IMM8 : len == 2 ? nodes::NodeKind::_INST_MOVSX_REG_IMM16
                                                                               : nodes::NodeKind::_INST_MOVSX_REG_IMM32;
            ptr = std::make_unique<nodes::NodeInstMovRegImm>();
            auto temp = (nodes::NodeInstMovRegImm *)ptr.get();
            temp->is_iden = true;
            temp->dest_regr = regr->second;
            temp->value = curr_tok.value;
        }
        else
        {
            kind = len == 1 ? nodes::NodeKind::_INST_MOVSX_REG_REG8 : len == 2 ? nodes::NodeKind::_INST_MOVSX_REG_REG16
                                                                               : nodes::NodeKind::_INST_MOVSX_REG_REG32;
            ptr = std::make_unique<nodes::NodeInstMovRegReg>();
            auto temp = (nodes::NodeInstMovRegReg *)ptr.get();
            temp->src_reg = iden2->second;
            temp->dest_regr = regr->second;
        }
    }
    else if (curr_tok.type == lexer::_TT_INT)
    {
        // then we have an immediate here
        // in future based on the size of the number, we could encode mov64 instruction
        kind = len == 1 ? nodes::NodeKind::_INST_MOVSX_REG_IMM8 : len == 2 ? nodes::NodeKind::_INST_MOVSX_REG_IMM16
                                                                           : nodes::NodeKind::_INST_MOVSX_REG_IMM32;
        ptr = std::make_unique<nodes::NodeInstMovRegImm>();
        auto temp = (nodes::NodeInstMovRegImm *)ptr.get();
        temp->dest_regr = regr->second;
        temp->value = curr_tok.value;
    }
    else
    {
        lexer.parse_error("Expected an immediate value, register or a variable name.");
    }
    nodes.push_back(std::make_unique<masm::nodes::Node>(nodes::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_inst_movX()
{
    size_t len = curr_tok.type == lexer::_TT_INST_MOVB ? 1 : curr_tok.type == lexer::_TT_INST_MOVW ? 2
                                                                                                   : 4;
    next_token();
    nodes::NodeKind kind;
    auto regr = nodes::_regr_iden_map.find(curr_tok.value);
    if (regr == nodes::_regr_iden_map.end())
    {
        lexer.parse_error("Expected register name after 'movX' instruction.");
    }
    next_token();
    std::unique_ptr<nodes::Base> ptr;
    if (curr_tok.type == lexer::_TT_IDENTIFIER)
    {
        auto iden2 = nodes::_regr_iden_map.find(curr_tok.value);
        if (iden2 == nodes::_regr_iden_map.end())
        {
            // it is a variable name
            kind = len == 1 ? nodes::NodeKind::_INST_MOV_REG_IMM8 : len == 2 ? nodes::NodeKind::_INST_MOV_REG_IMM16
                                                                             : nodes::NodeKind::_INST_MOV_REG_IMM32;
            ptr = std::make_unique<nodes::NodeInstMovRegImm>();
            auto temp = (nodes::NodeInstMovRegImm *)ptr.get();
            temp->is_iden = true;
            temp->dest_regr = regr->second;
            temp->value = curr_tok.value;
        }
        else
        {
            kind = len == 1 ? nodes::NodeKind::_INST_MOV_REG_REG8 : len == 2 ? nodes::NodeKind::_INST_MOV_REG_REG16
                                                                             : nodes::NodeKind::_INST_MOV_REG_REG32;
            ptr = std::make_unique<nodes::NodeInstMovRegReg>();
            auto temp = (nodes::NodeInstMovRegReg *)ptr.get();
            temp->src_reg = iden2->second;
            temp->dest_regr = regr->second;
        }
    }
    else if (curr_tok.type == lexer::_TT_INT)
    {
        // then we have an immediate here
        // in future based on the size of the number, we could encode mov64 instruction
        kind = len == 1 ? nodes::NodeKind::_INST_MOV_REG_IMM8 : len == 2 ? nodes::NodeKind::_INST_MOV_REG_IMM16
                                                                         : nodes::NodeKind::_INST_MOV_REG_IMM32;
        ptr = std::make_unique<nodes::NodeInstMovRegImm>();
        auto temp = (nodes::NodeInstMovRegImm *)ptr.get();
        temp->dest_regr = regr->second;
        temp->value = curr_tok.value;
    }
    else
    {
        lexer.parse_error("Expected an immediate value, register or a variable name.");
    }
    nodes.push_back(std::make_unique<masm::nodes::Node>(nodes::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_inst_moveX()
{
    size_t len = curr_tok.type == lexer::_TT_INST_MOVEB ? 1 : curr_tok.type == lexer::_TT_INST_MOVEW ? 2
                                                                                                     : 4;
    next_token();
    nodes::NodeKind kind;
    auto regr = nodes::_regr_iden_map.find(curr_tok.value);
    if (regr == nodes::_regr_iden_map.end())
    {
        lexer.parse_error("Expected register name after 'movX' instruction.");
    }
    next_token();
    std::unique_ptr<nodes::Base> ptr;
    if (curr_tok.type == lexer::_TT_IDENTIFIER)
    {
        auto iden2 = nodes::_regr_iden_map.find(curr_tok.value);
        if (iden2 == nodes::_regr_iden_map.end())
        {
            lexer.parse_error("The moveX instruction doesn't take a variable as it's operand.");
        }
        else
        {
            kind = len == 1 ? nodes::NodeKind::_INST_MOV_REG_MOVEB : len == 2 ? nodes::NodeKind::_INST_MOV_REG_MOVEW
                                                                              : nodes::NodeKind::_INST_MOV_REG_MOVED;
            ptr = std::make_unique<nodes::NodeInstMovRegReg>();
            auto temp = (nodes::NodeInstMovRegReg *)ptr.get();
            temp->src_reg = iden2->second;
            temp->dest_regr = regr->second;
        }
    }
    else if (curr_tok.type == lexer::_TT_INT)
    {
        lexer.parse_error("The moveX instruction expects registers as it's only operands not immediates.");
    }
    else
    {
        lexer.parse_error("Expected an immediate value, register or a variable name.");
    }
    nodes.push_back(std::make_unique<masm::nodes::Node>(nodes::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_inst_mov()
{
    // We have to identify what the variant is
    bool is_q = (curr_tok.type == lexer::_TT_INST_MOVQ);
    next_token();
    // This must be a register no matter what
    auto regr = nodes::_regr_iden_map.find(curr_tok.value);
    if (regr == nodes::_regr_iden_map.end())
    {
        lexer.parse_error("Expected register name after 'mov' instruction.");
    }
    next_token();
    nodes::NodeKind kind;
    std::unique_ptr<nodes::Base> ptr;
    if (curr_tok.type == lexer::_TT_IDENTIFIER)
    {
        // then it could be a mov reg, reg
        // or it could be a variable as well
        auto iden2 = nodes::_regr_iden_map.find(curr_tok.value);
        if (iden2 == nodes::_regr_iden_map.end())
        {
            // it is a variable name
            kind = nodes::NodeKind::_INST_MOV_REG_IMM;
            ptr = std::make_unique<nodes::NodeInstMovRegImm>();
            auto temp = (nodes::NodeInstMovRegImm *)ptr.get();
            temp->is_iden = true;
            temp->dest_regr = regr->second;
            temp->value = curr_tok.value;
        }
        else
        {
            kind = nodes::NodeKind::_INST_MOV_REG_REG;
            ptr = std::make_unique<nodes::NodeInstMovRegReg>();
            auto temp = (nodes::NodeInstMovRegReg *)ptr.get();
            temp->src_reg = iden2->second;
            temp->dest_regr = regr->second;
            is_q = false;
        }
    }
    else if (curr_tok.type == lexer::_TT_INT)
    {
        // then we have an immediate here
        // in future based on the size of the number, we could encode mov64 instruction
        kind = nodes::NodeKind::_INST_MOV_REG_IMM;
        ptr = std::make_unique<nodes::NodeInstMovRegImm>();
        auto temp = (nodes::NodeInstMovRegImm *)ptr.get();
        temp->dest_regr = regr->second;
        temp->value = curr_tok.value;
    }
    else
    {
        lexer.parse_error("Expected an immediate value, register or a variable name.");
    }
    kind = is_q ? nodes::NodeKind::_INST_MOV_REG_IMMQ : kind;
    nodes.push_back(std::make_unique<masm::nodes::Node>(nodes::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
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
            lexer.parse_error("Defining variables in the text section is not allowed");
        handle_definebyte(name);
        break;
    }
    case lexer::_TT_KEY_DW:
    {
        if (section != _SECTION_DATA)
            lexer.parse_error("Defining variables in the text section is not allowed");
        handle_defineword(name);
        break;
    }
    case lexer::_TT_KEY_DD:
    {
        if (section != _SECTION_DATA)
            lexer.parse_error("Defining variables in the text section is not allowed");
        handle_definedword(name);
        break;
    }
    case lexer::_TT_KEY_DQ:
    {
        if (section != _SECTION_DATA)
            lexer.parse_error("Defining variables in the text section is not allowed");
        handle_defineqword(name);
        break;
    }
    case lexer::_TT_KEY_STRING:
    {
        if (section != _SECTION_DATA)
            lexer.parse_error("Defining variables in the text section is not allowed");
        handle_string(name);
        break;
    }
    case lexer::_TT_KEY_RESB:
    case lexer::_TT_KEY_RESW:
    case lexer::_TT_KEY_RESD:
    case lexer::_TT_KEY_RESQ:
    {
        if (section != _SECTION_DATA)
            lexer.parse_error("Defining variables in the text section is not allowed");
        handle_resX(name);
        break;
    }
    default:
    {
        if (section == _SECTION_DATA)
        {
            lexer.parse_error("Defining labels in the data section is not allowed");
        }
        // this is a label
        handle_label(name);
        break;
    }
    }
}

void masm::parser::Parser::handle_resX(std::string name)
{
    auto kind = curr_tok.type == lexer::_TT_KEY_RESB ? nodes::_DEF_RESB : curr_tok.type == lexer::_TT_KEY_RESW ? nodes::_DEF_RESW
                                                                      : curr_tok.type == lexer::_TT_KEY_RESD   ? nodes::_DEF_RESD
                                                                                                               : nodes::_DEF_RESQ;
    next_token();
    if (curr_tok.type != lexer::_TT_INT)
        lexer.parse_error("Expected a number after 'resX'.");
    if (curr_tok.value == "0")
        lexer.parse_error("Cannot reserve 0 bytes.");
    std::unique_ptr<nodes::Base> ptr;
    ptr = std::make_unique<nodes::NodeRes>();
    auto temp = dynamic_cast<nodes::NodeRes *>(ptr.get());
    temp->number = std::stoull(curr_tok.value);
    temp->name = name;
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_DATA, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_label(std::string label_name)
{
    nodes::NodeKind kind;
    std::unique_ptr<nodes::Base> ptr;
    kind = nodes::_LABEL;
    ptr = std::make_unique<nodes::NodeLabel>();
    auto temp = (nodes::NodeLabel *)ptr.get();
    temp->label_name = label_name;
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_definebyte(std::string name)
{
    next_token();

    if (curr_tok.type != lexer::_TT_INT)
    {
        lexer.parse_err_previous_token(curr_tok.value, std::string("Expected a number, got ") + curr_tok.value + " instead.");
    }
    // nodes::Node& node = nodes.back();
    nodes::NodeKind kind;
    std::unique_ptr<nodes::Base> ptr;
    kind = nodes::_DEF_BYTE;
    ptr = std::make_unique<nodes::NodeDefByte>();
    auto temp = dynamic_cast<nodes::NodeDefByte *>(ptr.get());
    temp->byte_name = name;
    temp->byte_val = curr_tok.value;
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_DATA, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_defineword(std::string name)
{
    next_token();
    if (curr_tok.type != lexer::_TT_INT)
    {
        lexer.parse_err_previous_token(curr_tok.value, std::string("Expected a number, got ") + curr_tok.value + " instead.");
    }
    nodes::NodeKind kind;
    std::unique_ptr<nodes::Base> ptr;
    kind = nodes::_DEF_WORD;
    ptr = std::make_unique<nodes::NodeDefWord>();
    auto temp = dynamic_cast<nodes::NodeDefWord *>(ptr.get());
    temp->byte_name = name;
    temp->byte_val = curr_tok.value;
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_DATA, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_definedword(std::string name)
{
    next_token();
    if (curr_tok.type != lexer::_TT_INT)
    {
        lexer.parse_err_previous_token(curr_tok.value, std::string("Expected a number, got ") + curr_tok.value + " instead.");
    }
    nodes::NodeKind kind;
    std::unique_ptr<nodes::Base> ptr;
    kind = nodes::_DEF_DWORD;
    ptr = std::make_unique<nodes::NodeDefDword>();
    auto temp = dynamic_cast<nodes::NodeDefDword *>(ptr.get());
    temp->byte_name = name;
    temp->byte_val = curr_tok.value;
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_DATA, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_defineqword(std::string name)
{
    next_token();
    if (curr_tok.type != lexer::_TT_INT)
    {
        lexer.parse_err_previous_token(curr_tok.value, std::string("Expected a number, got ") + curr_tok.value + " instead.");
    }
    nodes::NodeKind kind;
    std::unique_ptr<nodes::Base> ptr;
    kind = nodes::_DEF_QWORD;
    ptr = std::make_unique<nodes::NodeDefQword>();
    auto temp = dynamic_cast<nodes::NodeDefQword *>(ptr.get());
    temp->byte_name = name;
    temp->byte_val = curr_tok.value;
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_DATA, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_string(std::string name)
{
    next_token();
    if (curr_tok.type != lexer::_TT_STRING)
    {
        lexer.parse_err_previous_token(curr_tok.value, std::string("Expected a string, got ") + curr_tok.value + " instead.");
    }
    nodes::NodeKind kind;
    std::unique_ptr<nodes::Base> ptr;
    kind = nodes::_DEF_STRING;
    ptr = std::make_unique<nodes::NodeDefByte>();
    auto temp = dynamic_cast<nodes::NodeDefByte *>(ptr.get());
    temp->byte_name = name;
    temp->byte_val = curr_tok.value;
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_DATA, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_proc_declaration()
{
    next_token();
    if (curr_tok.type != lexer::_TT_IDENTIFIER)
    {
        lexer.parse_error("Expected a procedure name after the 'proc' keyword");
    }
    nodes::NodeKind kind;
    std::unique_ptr<nodes::Base> ptr;
    kind = nodes::_PROC_DECLR;
    ptr = std::make_unique<nodes::NodeProcDeclr>();
    auto temp = (nodes::NodeProcDeclr *)ptr.get();
    temp->proc_name = curr_tok.value;
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_inst_cin()
{
    auto kind = nodes::NodeKind::_INST_CIN;
    next_token();
    if (curr_tok.type != lexer::_TT_IDENTIFIER)
        lexer.parse_error("Expected a register after 'cin'.");
    auto regr = nodes::_regr_iden_map.find(curr_tok.value);
    if (regr == nodes::_regr_iden_map.end())
        lexer.parse_error("Expected a register after 'cin'.");
    std::unique_ptr<nodes::Base> ptr;
    ptr = std::make_unique<nodes::NodeOneRegrOperands>();
    ((nodes::NodeOneRegrOperands *)(ptr.get()))->oper_rger = regr->second;
    nodes.push_back(std::make_unique<nodes::Node>(nodes::NodeType::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_inst_cout()
{
    auto kind = nodes::NodeKind::_INST_COUT;
    next_token();
    if (curr_tok.type != lexer::_TT_IDENTIFIER)
        lexer.parse_error("Expected a register after 'cout'.");
    auto regr = nodes::_regr_iden_map.find(curr_tok.value);
    if (regr == nodes::_regr_iden_map.end())
        lexer.parse_error("Expected a register after 'cout'.");
    std::unique_ptr<nodes::Base> ptr;
    ptr = std::make_unique<nodes::NodeOneRegrOperands>();
    ((nodes::NodeOneRegrOperands *)(ptr.get()))->oper_rger = regr->second;
    nodes.push_back(std::make_unique<nodes::Node>(nodes::NodeType::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_inst_sin()
{
    next_token();
    if (curr_tok.type != lexer::_TT_IDENTIFIER)
        lexer.parse_error("Expected an identifier after 'sin'.");
    if (nodes::_regr_iden_map.find(curr_tok.value) != nodes::_regr_iden_map.end())
        lexer.parse_error("The 'sin' instruction doesn't take registers as operands.");
    nodes::NodeKind kind = nodes::NodeKind::_INST_SIN;
    std::unique_ptr<nodes::Base> ptr = std::make_unique<nodes::NodeOneImmOperand>();
    auto temp = (nodes::NodeOneImmOperand *)ptr.get();
    temp->imm = curr_tok.value;
    nodes.push_back(std::make_unique<nodes::Node>(nodes::NodeType::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_inst_sout()
{
    next_token();
    if (curr_tok.type != lexer::_TT_IDENTIFIER)
        lexer.parse_error("Expected an identifier after 'sout'.");
    if (nodes::_regr_iden_map.find(curr_tok.value) != nodes::_regr_iden_map.end())
        lexer.parse_error("The 'sout' instruction doesn't take registers as operands.");
    nodes::NodeKind kind = nodes::NodeKind::_INST_SOUT;
    std::unique_ptr<nodes::Base> ptr = std::make_unique<nodes::NodeOneImmOperand>();
    auto temp = (nodes::NodeOneImmOperand *)ptr.get();
    temp->imm = curr_tok.value;
    nodes.push_back(std::make_unique<nodes::Node>(nodes::NodeType::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}
