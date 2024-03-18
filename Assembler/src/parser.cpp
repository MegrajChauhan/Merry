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
    while (curr_tok.type != lexer::_TT_EOF)
    {
        switch (curr_tok.type)
        {
        case lexer::_TT_SECTION_DATA:
            parseDataSection();
            break;
        case lexer::_TT_SECTION_TEXT:
            parseTextSection();
            break;
        case lexer::_TT_IDENTIFIER:
            handle_identifier();
            if (nodes.back()->kind != nodes::NodeKind::_LABEL)
                next_token();
            break;
        case lexer::_TT_KEY_PROC:
            if (section != _SECTION_TEXT)
            {
                lexer.parse_error("Declaring a procedure in the data section is not allowed");
                break;
            }
            handle_proc_declaration();
            next_token();
            break;
        case lexer::_TT_INST_NOP:
        case lexer::_TT_INST_HLT:
        case lexer::_TT_INST_MOV:
        case lexer::_TT_INST_MOVQ:
        case lexer::_TT_INST_MOVB:
        case lexer::_TT_INST_MOVW:
        case lexer::_TT_INST_MOVD:
        case lexer::_TT_INST_MOVEB:
        case lexer::_TT_INST_MOVEW:
        case lexer::_TT_INST_MOVED:
        case lexer::_TT_INST_MOVF:
        case lexer::_TT_INST_MOVLF:
        case lexer::_TT_INST_MOVSXB:
        case lexer::_TT_INST_MOVSXW:
        case lexer::_TT_INST_MOVSXD:
        case lexer::_TT_INST_OUTR:
        case lexer::_TT_INST_UOUTR:
        case lexer::_TT_INST_CIN:
        case lexer::_TT_INST_COUT:
        case lexer::_TT_INST_SIN:
        case lexer::_TT_INST_SOUT:
        case lexer::_TT_INST_IN:
        case lexer::_TT_INST_OUT:
        case lexer::_TT_INST_INW:
        case lexer::_TT_INST_OUTW:
        case lexer::_TT_INST_IND:
        case lexer::_TT_INST_OUTD:
        case lexer::_TT_INST_INQ:
        case lexer::_TT_INST_OUTQ:
        case lexer::_TT_INST_UIN:
        case lexer::_TT_INST_UOUT:
        case lexer::_TT_INST_UINW:
        case lexer::_TT_INST_UOUTW:
        case lexer::_TT_INST_UIND:
        case lexer::_TT_INST_UOUTD:
        case lexer::_TT_INST_UINQ:
        case lexer::_TT_INST_UOUTQ:
        case lexer::_TT_INST_OUTF:
        case lexer::_TT_INST_OUTLF:
        case lexer::_TT_INST_INLF:
        case lexer::_TT_INST_INF:
        case lexer::_TT_INST_ADD:
        case lexer::_TT_INST_SUB:
        case lexer::_TT_INST_MUL:
        case lexer::_TT_INST_DIV:
        case lexer::_TT_INST_MOD:
        case lexer::_TT_INST_IADD:
        case lexer::_TT_INST_ISUB:
        case lexer::_TT_INST_IMUL:
        case lexer::_TT_INST_IDIV:
        case lexer::_TT_INST_IMOD:
        case lexer::_TT_INST_FADD:
        case lexer::_TT_INST_LFADD:
        case lexer::_TT_INST_FSUB:
        case lexer::_TT_INST_LFSUB:
        case lexer::_TT_INST_FMUL:
        case lexer::_TT_INST_LFMUL:
        case lexer::_TT_INST_FDIV:
        case lexer::_TT_INST_LFDIV:
            handleInstruction();
            break;
        default:
            lexer.parse_error("Expected an identifier name or a keyword");
        }
    }
    if (!encountered_text)
    {
        std::cerr << "Parse Error: The input contains no text section which is not allowed." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void masm::parser::Parser::handleInstruction()
{
    if (section != _SECTION_TEXT)
    {
        lexer.parse_error("Using instructions in the data section is not allowed");
    }
    switch (curr_tok.type)
    {
    case lexer::_TT_INST_NOP:
        nodes.push_back(std::make_unique<nodes::Node>(nodes::Node(nodes::_TYPE_INST, nodes::_INST_NOP, std::make_unique<nodes::Base>())));
        break;
    case lexer::_TT_INST_HLT:
        nodes.push_back(std::make_unique<nodes::Node>(nodes::Node(nodes::_TYPE_INST, nodes::_INST_HLT, std::make_unique<nodes::Base>())));
        break;
    case lexer::_TT_INST_MOVQ:
    case lexer::_TT_INST_MOV:
        handle_inst_mov();
        break;
    case lexer::_TT_INST_MOVB:
    case lexer::_TT_INST_MOVW:
    case lexer::_TT_INST_MOVD:
        handle_inst_movX();
        break;
    case lexer::_TT_INST_MOVEB:
    case lexer::_TT_INST_MOVEW:
    case lexer::_TT_INST_MOVED:
        handle_inst_moveX();
        break;
    case lexer::_TT_INST_MOVSXB:
    case lexer::_TT_INST_MOVSXW:
    case lexer::_TT_INST_MOVSXD:
        handle_inst_movsx();
        break;
    case lexer::_TT_INST_OUTR:
        nodes.push_back(std::make_unique<nodes::Node>(nodes::NodeType::_TYPE_INST, nodes::NodeKind::_INST_OUTR, std::move(std::make_unique<nodes::Base>()), lexer.get_curr_line()));
        break;
    case lexer::_TT_INST_UOUTR:
        nodes.push_back(std::make_unique<nodes::Node>(nodes::NodeType::_TYPE_INST, nodes::NodeKind::_INST_UOUTR, std::move(std::make_unique<nodes::Base>()), lexer.get_curr_line()));
        break;
    case lexer::_TT_INST_CIN:
        handle_inst_Xin(nodes::NodeKind::_INST_CIN);
        break;
    case lexer::_TT_INST_SIN:
        handle_inst_sin();
        break;
    case lexer::_TT_INST_IN:
        handle_inst_Xin(nodes::NodeKind::_INST_IN);
        break;
    case lexer::_TT_INST_COUT:
        handle_inst_Xout(nodes::NodeKind::_INST_COUT);
        break;
    case lexer::_TT_INST_OUT:
        handle_inst_Xout(nodes::NodeKind::_INST_OUT);
        break;
    case lexer::_TT_INST_SOUT:
        handle_inst_sout();
        break;
    case lexer::_TT_INST_INW:
        handle_inst_Xin(nodes::NodeKind::_INST_INW);
        break;
    case lexer::_TT_INST_OUTW:
        handle_inst_Xout(nodes::NodeKind::_INST_OUTW);
        break;
    case lexer::_TT_INST_IND:
        handle_inst_Xin(nodes::NodeKind::_INST_IND);
        break;
    case lexer::_TT_INST_OUTD:
        handle_inst_Xout(nodes::NodeKind::_INST_OUTD);
        break;
    case lexer::_TT_INST_INQ:
        handle_inst_Xin(nodes::NodeKind::_INST_INQ);
        break;
    case lexer::_TT_INST_OUTQ:
        handle_inst_Xout(nodes::NodeKind::_INST_OUTQ);
        break;
    case lexer::_TT_INST_UIN:
        handle_inst_Xin(nodes::NodeKind::_INST_UIN);
        break;
    case lexer::_TT_INST_UOUT:
        handle_inst_Xout(nodes::NodeKind::_INST_UOUT);
        break;
    case lexer::_TT_INST_UINW:
        handle_inst_Xin(nodes::NodeKind::_INST_UINW);
        break;
    case lexer::_TT_INST_UOUTW:
        handle_inst_Xout(nodes::NodeKind::_INST_UOUTW);
        break;
    case lexer::_TT_INST_UIND:
        handle_inst_Xin(nodes::NodeKind::_INST_UIND);
        break;
    case lexer::_TT_INST_UOUTD:
        handle_inst_Xout(nodes::NodeKind::_INST_UOUTD);
        break;
    case lexer::_TT_INST_UINQ:
        handle_inst_Xin(nodes::NodeKind::_INST_UINQ);
        break;
    case lexer::_TT_INST_UOUTQ:
        handle_inst_Xout(nodes::NodeKind::_INST_UOUTQ);
        break;
    case lexer::_TT_INST_INF:
        handle_inst_Xout(nodes::NodeKind::_INST_INF);
        break;
    case lexer::_TT_INST_INLF:
        handle_inst_Xout(nodes::NodeKind::_INST_INLF);
        break;
    case lexer::_TT_INST_OUTF:
        handle_inst_Xout(nodes::NodeKind::_INST_OUTF);
        break;
    case lexer::_TT_INST_OUTLF:
        handle_inst_Xout(nodes::NodeKind::_INST_OUTLF);
        break;
    case lexer::_TT_INST_MOVF:
    case lexer::_TT_INST_MOVLF:
        handle_inst_movf();
        break;
    case lexer::_TT_INST_ADD:
    case lexer::_TT_INST_IADD:
        handle_inst_add();
        break;
    case lexer::_TT_INST_SUB:
    case lexer::_TT_INST_ISUB:
        handle_inst_sub();
        break;
    case lexer::_TT_INST_DIV:
    case lexer::_TT_INST_IDIV:
        handle_inst_div();
        break;
    case lexer::_TT_INST_MUL:
    case lexer::_TT_INST_IMUL:
        handle_inst_mul();
        break;
    case lexer::_TT_INST_MOD:
    case lexer::_TT_INST_IMOD:
        handle_inst_mod();
        break;
    case lexer::_TT_INST_FADD:
    case lexer::_TT_INST_LFADD:
        handle_inst_fadd();
        break;
    case lexer::_TT_INST_FSUB:
    case lexer::_TT_INST_LFSUB:
        handle_inst_fsub();
        break;
    case lexer::_TT_INST_FMUL:
    case lexer::_TT_INST_LFMUL:
        handle_inst_fmul();
        break;
    case lexer::_TT_INST_FDIV:
    case lexer::_TT_INST_LFDIV:
        handle_inst_fdiv();
        break;
        break;
    }
    next_token();
}

void masm::parser::Parser::parseDataSection()
{
    next_token();
    if (curr_tok.type != lexer::_TT_OPER_COLON)
    {
        lexer.parse_err_expected_colon("after \'.data\'");
    }
    if (section == _SECTION_DATA)
    {
        lexer.parse_err_whole_line("Data section redefintiion when already in the data section.");
    }
    section = _SECTION_DATA;
    next_token(); // i can't remember the reason why I moved this line back into each case statements instead of having just one at the end
}

void masm::parser::Parser::parseTextSection()
{
    next_token();
    if (curr_tok.type != lexer::_TT_OPER_COLON)
    {
        lexer.parse_err_expected_colon("after \'.text\'");
    }
    if (section == _SECTION_TEXT)
    {
        lexer.parse_err_whole_line("Text section redefintiion when already in the text section.");
    }
    section = _SECTION_TEXT;
    encountered_text = true;
    next_token();
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
    else if (curr_tok.type == lexer::_TT_INT || curr_tok.type == lexer::_TT_NINT)
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
    else if (curr_tok.type == lexer::_TT_INT || curr_tok.type == lexer::_TT_NINT)
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
    else if (curr_tok.type == lexer::_TT_INT || curr_tok.type == lexer::_TT_NINT)
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
    else if (curr_tok.type == lexer::_TT_INT || curr_tok.type == lexer::_TT_NINT)
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
    case lexer::_TT_KEY_DF:
    case lexer::_TT_KEY_DLF:
    {
        if (section != _SECTION_DATA)
            lexer.parse_error("Defining variables in the text section is not allowed");
        handle_definefloats(name);
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
        lexer.parse_error("Expected a positive number after 'resX'.");
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

    if (curr_tok.type != lexer::_TT_INT && curr_tok.type != lexer::_TT_NINT)
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
    if (curr_tok.type != lexer::_TT_INT && curr_tok.type != lexer::_TT_NINT)
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
    if (curr_tok.type != lexer::_TT_INT && curr_tok.type != lexer::_TT_NINT)
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
    if (curr_tok.type != lexer::_TT_INT && curr_tok.type != lexer::_TT_NINT)
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

void masm::parser::Parser::handle_inst_Xin(nodes::NodeKind k)
{
    auto kind = k;
    next_token();
    if (curr_tok.type != lexer::_TT_IDENTIFIER)
        lexer.parse_error("Expected a register after " + (k == nodes::_INST_INF || k == nodes::_INST_INLF) ? "inF." : "Xin.");
    auto regr = nodes::_regr_iden_map.find(curr_tok.value);
    if (regr == nodes::_regr_iden_map.end())
        lexer.parse_error("Expected a register after " + (k == nodes::_INST_INF || k == nodes::_INST_INLF) ? "inF." : "Xin.");
    std::unique_ptr<nodes::Base> ptr;
    ptr = std::make_unique<nodes::NodeOneRegrOperands>();
    ((nodes::NodeOneRegrOperands *)(ptr.get()))->oper_rger = regr->second;
    nodes.push_back(std::make_unique<nodes::Node>(nodes::NodeType::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_inst_Xout(nodes::NodeKind k)
{
    auto kind = k;
    next_token();
    if (curr_tok.type != lexer::_TT_IDENTIFIER)
        lexer.parse_error("Expected a register after " + (k == nodes::_INST_OUTF || k == nodes::_INST_OUTLF) ? "outF." : "Xin.");
    auto regr = nodes::_regr_iden_map.find(curr_tok.value);
    if (regr == nodes::_regr_iden_map.end())
        lexer.parse_error("Expected a register after " + (k == nodes::_INST_OUTF || k == nodes::_INST_OUTLF) ? "outF." : "Xin.");
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

void masm::parser::Parser::handle_inst_movf()
{
    nodes::NodeKind kind = curr_tok.type == lexer::_TT_INST_MOVF ? nodes::_INST_MOVF : nodes::_INST_MOVLF;
    next_token();
    auto dest_regr = nodes::_regr_iden_map.find(curr_tok.value);
    if (dest_regr == nodes::_regr_iden_map.end())
        lexer.parse_error("Expected a destination register is the movF instruction.");
    next_token();
    auto src_oper = nodes::_regr_iden_map.find(curr_tok.value);
    std::unique_ptr<nodes::Base> node = std::make_unique<nodes::NodeInstMovRegImm>();
    auto temp = (nodes::NodeInstMovRegImm *)node.get();
    temp->dest_regr = dest_regr->second;
    if (src_oper == nodes::_regr_iden_map.end())
    {
        // this is not a register
        // check if it is a number or a variable
        if (curr_tok.type == lexer::_TT_FLOAT || curr_tok.type == lexer::_TT_NFLOAT)
            temp->value = curr_tok.value;
        else if (curr_tok.type == lexer::_TT_IDENTIFIER)
        {
            temp->is_iden = true;
            temp->value = curr_tok.value;
        }
        else
            lexer.parse_error("Expected a variable name or a floating point number.");
    }
    else
    {
        // this instruction doesn't expect a register
        // there is no particular reason for this but still I DO IT BECAUSE I CAN
        lexer.parse_error("The floating point move instructions do not accept registers as the source.");
    }
    nodes.push_back(std::make_unique<masm::nodes::Node>(nodes::_TYPE_INST, kind, std::move(node), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_definefloats(std::string name)
{
    nodes::NodeKind kind = curr_tok.type == lexer::_TT_KEY_DF ? nodes::_DEF_FLOAT : nodes::_DEF_LFLOAT;
    next_token();
    if (curr_tok.type != lexer::_TT_FLOAT && curr_tok.type != lexer::_TT_NFLOAT)
    {
        lexer.parse_err_previous_token(curr_tok.value, std::string("Expected a float, got ") + curr_tok.value + " instead.");
    }
    std::unique_ptr<nodes::Base> ptr;
    ptr = std::make_unique<nodes::NodeDefFloat>();
    auto temp = dynamic_cast<nodes::NodeDefFloat *>(ptr.get());
    temp->byte_name = name;
    temp->byte_val = curr_tok.value;
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_DATA, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_inst_add()
{
    nodes::NodeKind kind;
    auto temp_curr = curr_tok;
    std::unique_ptr<nodes::Base> ptr;
    next_token(); // goto the next token
    if (curr_tok.type == lexer::_TT_IDENTIFIER)
    {
        // must be a register
        auto dest_regr = nodes::_regr_iden_map.find(curr_tok.value);
        if (dest_regr == nodes::_regr_iden_map.end())
            lexer.parse_err_previous_token(curr_tok.value, "Expected a destination register.");
        next_token();
        if (curr_tok.type != lexer::_TT_IDENTIFIER && curr_tok.type != lexer::_TT_INT && curr_tok.type != lexer::_TT_NINT)
            lexer.parse_err_previous_token(curr_tok.value, "Expected a source register, immediate or a source register.");
        auto src = nodes::_regr_iden_map.find(curr_tok.value);
        if (src == nodes::_regr_iden_map.end())
        {
            // this is an add_imm instruction
            ptr = std::make_unique<nodes::NodeAddRegImm>();
            auto temp = (nodes::NodeAddRegImm *)ptr.get();
            temp->dest_regr = dest_regr->second;
            if (curr_tok.type == lexer::_TT_INT || curr_tok.type == lexer::_TT_NINT)
                temp->is_iden = false;
            else
                temp->is_iden = true;
            temp->value = curr_tok.value;
            kind = temp_curr.type == lexer::_TT_INST_ADD ? nodes::_INST_ADD_IMM : nodes::_INST_IADD_IMM;
        }
        else
        {
            // this is an add_reg instruction
            ptr = std::make_unique<nodes::NodeAddRegReg>();
            auto temp = (nodes::NodeAddRegReg *)ptr.get();
            temp->dest_regr = dest_regr->second;
            temp->src_reg = src->second;
            kind = temp_curr.type == lexer::_TT_INST_ADD ? nodes::_INST_ADD_REG : nodes::_INST_IADD_REG;
        }
    }
    else
        lexer.parse_error("Expected a destination register.");
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_inst_sub()
{
    auto temp_curr = curr_tok;
    nodes::NodeKind kind;
    std::unique_ptr<nodes::Base> ptr;
    next_token(); // goto the next token
    if (curr_tok.type == lexer::_TT_IDENTIFIER)
    {
        auto dest_regr = nodes::_regr_iden_map.find(curr_tok.value);
        if (dest_regr == nodes::_regr_iden_map.end())
            lexer.parse_err_previous_token(curr_tok.value, "Expected a destination register.");
        next_token();
        if (curr_tok.type != lexer::_TT_IDENTIFIER && curr_tok.type != lexer::_TT_INT && curr_tok.type != lexer::_TT_NINT)
            lexer.parse_err_previous_token(curr_tok.value, "Expected a source register, immediate or a source register.");
        auto src = nodes::_regr_iden_map.find(curr_tok.value);
        if (src == nodes::_regr_iden_map.end())
        {
            ptr = std::make_unique<nodes::NodeSubRegImm>();
            auto temp = (nodes::NodeSubRegImm *)ptr.get();
            temp->dest_regr = dest_regr->second;
            if (curr_tok.type == lexer::_TT_INT || curr_tok.type == lexer::_TT_NINT)
                temp->is_iden = false;
            else
                temp->is_iden = true;
            temp->value = curr_tok.value;
            kind = temp_curr.type == lexer::_TT_INST_SUB ? nodes::_INST_SUB_IMM : nodes::_INST_ISUB_IMM;
        }
        else
        {
            ptr = std::make_unique<nodes::NodeSubRegReg>();
            auto temp = (nodes::NodeSubRegReg *)ptr.get();
            temp->dest_regr = dest_regr->second;
            temp->src_reg = src->second;
            kind = temp_curr.type == lexer::_TT_INST_SUB ? nodes::_INST_SUB_REG : nodes::_INST_ISUB_REG;
        }
    }
    else
        lexer.parse_error("Expected a destination register.");
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_inst_mul()
{
    auto temp_curr = curr_tok;
    nodes::NodeKind kind;
    std::unique_ptr<nodes::Base> ptr;
    next_token(); // goto the next token
    if (curr_tok.type == lexer::_TT_IDENTIFIER)
    {
        auto dest_regr = nodes::_regr_iden_map.find(curr_tok.value);
        if (dest_regr == nodes::_regr_iden_map.end())
            lexer.parse_err_previous_token(curr_tok.value, "Expected a destination register.");
        next_token();
        if (curr_tok.type != lexer::_TT_IDENTIFIER && curr_tok.type != lexer::_TT_INT && curr_tok.type != lexer::_TT_NINT)
            lexer.parse_err_previous_token(curr_tok.value, "Expected a source register, immediate or a source register.");
        auto src = nodes::_regr_iden_map.find(curr_tok.value);
        if (src == nodes::_regr_iden_map.end())
        {
            ptr = std::make_unique<nodes::NodeMulRegImm>();
            auto temp = (nodes::NodeMulRegImm *)ptr.get();
            temp->dest_regr = dest_regr->second;
            if (curr_tok.type == lexer::_TT_INT || curr_tok.type == lexer::_TT_NINT)
                temp->is_iden = false;
            else
                temp->is_iden = true;
            temp->value = curr_tok.value;
            kind = temp_curr.type == lexer::_TT_INST_MUL ? nodes::_INST_MUL_IMM : nodes::_INST_IMUL_IMM;
        }
        else
        {
            ptr = std::make_unique<nodes::NodeMulRegReg>();
            auto temp = (nodes::NodeMulRegReg *)ptr.get();
            temp->dest_regr = dest_regr->second;
            temp->src_reg = src->second;
            kind = temp_curr.type == lexer::_TT_INST_MUL ? nodes::_INST_MUL_REG : nodes::_INST_IMUL_REG;
        }
    }
    else
        lexer.parse_error("Expected a destination register.");
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_inst_div()
{
    nodes::NodeKind kind;
    auto temp_curr = curr_tok;
    std::unique_ptr<nodes::Base> ptr;
    next_token(); // goto the next token
    if (curr_tok.type == lexer::_TT_IDENTIFIER)
    {
        auto dest_regr = nodes::_regr_iden_map.find(curr_tok.value);
        if (dest_regr == nodes::_regr_iden_map.end())
            lexer.parse_err_previous_token(curr_tok.value, "Expected a destination register.");
        next_token();
        if (curr_tok.type != lexer::_TT_IDENTIFIER && curr_tok.type != lexer::_TT_INT && curr_tok.type != lexer::_TT_NINT)
            lexer.parse_err_previous_token(curr_tok.value, "Expected a source register, immediate or a source register.");
        auto src = nodes::_regr_iden_map.find(curr_tok.value);
        if (src == nodes::_regr_iden_map.end())
        {
            ptr = std::make_unique<nodes::NodeDivRegImm>();
            auto temp = (nodes::NodeDivRegImm *)ptr.get();
            temp->dest_regr = dest_regr->second;
            if (curr_tok.type == lexer::_TT_INT || curr_tok.type == lexer::_TT_NINT)
                temp->is_iden = false;
            else
                temp->is_iden = true;
            temp->value = curr_tok.value;
            kind = temp_curr.type == lexer::_TT_INST_DIV ? nodes::_INST_DIV_IMM : nodes::_INST_IDIV_IMM;
        }
        else
        {
            ptr = std::make_unique<nodes::NodeDivRegReg>();
            auto temp = (nodes::NodeDivRegReg *)ptr.get();
            temp->dest_regr = dest_regr->second;
            temp->src_reg = src->second;
            kind = temp_curr.type == lexer::_TT_INST_DIV ? nodes::_INST_DIV_REG : nodes::_INST_IDIV_REG;
        }
    }
    else
        lexer.parse_error("Expected a destination register.");
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_inst_mod()
{
    nodes::NodeKind kind;
    auto temp_curr = curr_tok;
    std::unique_ptr<nodes::Base> ptr;
    next_token(); // goto the next token
    if (curr_tok.type == lexer::_TT_IDENTIFIER)
    {
        auto dest_regr = nodes::_regr_iden_map.find(curr_tok.value);
        if (dest_regr == nodes::_regr_iden_map.end())
            lexer.parse_err_previous_token(curr_tok.value, "Expected a destination register.");
        next_token();
        if (curr_tok.type != lexer::_TT_IDENTIFIER && curr_tok.type != lexer::_TT_INT && curr_tok.type != lexer::_TT_NINT)
            lexer.parse_err_previous_token(curr_tok.value, "Expected a source register, immediate or a source register.");
        auto src = nodes::_regr_iden_map.find(curr_tok.value);
        if (src == nodes::_regr_iden_map.end())
        {
            ptr = std::make_unique<nodes::NodeModRegImm>();
            auto temp = (nodes::NodeModRegImm *)ptr.get();
            temp->dest_regr = dest_regr->second;
            if (curr_tok.type == lexer::_TT_INT || curr_tok.type == lexer::_TT_NINT)
                temp->is_iden = false;
            else
                temp->is_iden = true;
            temp->value = curr_tok.value;
            kind = temp_curr.type == lexer::_TT_INST_DIV ? nodes::_INST_MOD_IMM : nodes::_INST_IMOD_IMM;
        }
        else
        {
            ptr = std::make_unique<nodes::NodeModRegReg>();
            auto temp = (nodes::NodeModRegReg *)ptr.get();
            temp->dest_regr = dest_regr->second;
            temp->src_reg = src->second;
            kind = temp_curr.type == lexer::_TT_INST_DIV ? nodes::_INST_MOD_REG : nodes::_INST_IMOD_REG;
        }
    }
    else
        lexer.parse_error("Expected a destination register.");
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_inst_fadd()
{
    auto temp_curr_tok = curr_tok;
    nodes::NodeKind kind;
    std::unique_ptr<nodes::Base> ptr;
    // either it must be fadd or lfadd
    next_token();
    // must be a register
    if (curr_tok.type == lexer::_TT_IDENTIFIER)
    {
        auto dest_regr = nodes::_regr_iden_map.find(curr_tok.value);
        if (dest_regr == nodes::_regr_iden_map.end())
            lexer.parse_err_previous_token(curr_tok.value, "Expected a destination register.");
        next_token();
        // now it must either be an identifier or a floating point number
        if (curr_tok.type == lexer::_TT_IDENTIFIER)
        {
            // it could either be a register or a variable
            auto temp = nodes::_regr_iden_map.find(curr_tok.value);
            if (temp == nodes::_regr_iden_map.end())
            {
                // this is a variable
                kind = temp_curr_tok.type == lexer::_TT_INST_FADD ? nodes::_INST_FADD_IMM : nodes::_INST_LFADD_IMM;
                ptr = std::make_unique<nodes::NodeAddRegImm>();
                auto x = (nodes::NodeAddRegImm *)ptr.get();
                x->dest_regr = dest_regr->second;
                x->is_iden = true;
                x->value = curr_tok.value;
            }
            else
            {
                // then it is a register
                // this is the one we expect
                kind = temp_curr_tok.type == lexer::_TT_INST_FADD ? nodes::_INST_FADD_REG : nodes::_INST_LFADD_REG;
                ptr = std::make_unique<nodes::NodeAddRegReg>();
                auto x = (nodes::NodeAddRegReg *)ptr.get();
                x->dest_regr = dest_regr->second;
                x->src_reg = temp->second;
            }
        }
        else
            lexer.parse_error("Expected a register or a variable in floating point instruction.");
    }
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_inst_fsub()
{
    auto temp_curr_tok = curr_tok;
    nodes::NodeKind kind;
    std::unique_ptr<nodes::Base> ptr;
    next_token();
    if (curr_tok.type == lexer::_TT_IDENTIFIER)
    {
        auto dest_regr = nodes::_regr_iden_map.find(curr_tok.value);
        if (dest_regr == nodes::_regr_iden_map.end())
            lexer.parse_err_previous_token(curr_tok.value, "Expected a destination register.");
        next_token();
        if (curr_tok.type == lexer::_TT_IDENTIFIER)
        {
            auto temp = nodes::_regr_iden_map.find(curr_tok.value);
            if (temp == nodes::_regr_iden_map.end())
            {
                kind = temp_curr_tok.type == lexer::_TT_INST_FSUB ? nodes::_INST_FSUB_IMM : nodes::_INST_LFSUB_IMM;
                ptr = std::make_unique<nodes::NodeSubRegImm>();
                auto x = (nodes::NodeSubRegImm *)ptr.get();
                x->dest_regr = dest_regr->second;
                x->is_iden = true;
                x->value = curr_tok.value;
            }
            else
            {
                kind = temp_curr_tok.type == lexer::_TT_INST_FSUB ? nodes::_INST_FSUB_REG : nodes::_INST_LFSUB_REG;
                ptr = std::make_unique<nodes::NodeSubRegReg>();
                auto x = (nodes::NodeSubRegReg *)ptr.get();
                x->dest_regr = dest_regr->second;
                x->src_reg = temp->second;
            }
        }
        else
            lexer.parse_error("Expected a register or a variable in floating point instruction.");
    }
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_inst_fmul()
{
    auto temp_curr_tok = curr_tok;
    nodes::NodeKind kind;
    std::unique_ptr<nodes::Base> ptr;
    next_token();
    if (curr_tok.type == lexer::_TT_IDENTIFIER)
    {
        auto dest_regr = nodes::_regr_iden_map.find(curr_tok.value);
        if (dest_regr == nodes::_regr_iden_map.end())
            lexer.parse_err_previous_token(curr_tok.value, "Expected a destination register.");
        next_token();
        if (curr_tok.type == lexer::_TT_IDENTIFIER)
        {
            auto temp = nodes::_regr_iden_map.find(curr_tok.value);
            if (temp == nodes::_regr_iden_map.end())
            {
                kind = temp_curr_tok.type == lexer::_TT_INST_FMUL ? nodes::_INST_FMUL_IMM : nodes::_INST_LFMUL_IMM;
                ptr = std::make_unique<nodes::NodeMulRegImm>();
                auto x = (nodes::NodeMulRegImm *)ptr.get();
                x->dest_regr = dest_regr->second;
                x->is_iden = true;
                x->value = curr_tok.value;
            }
            else
            {
                kind = temp_curr_tok.type == lexer::_TT_INST_FMUL ? nodes::_INST_FMUL_REG : nodes::_INST_LFMUL_REG;
                ptr = std::make_unique<nodes::NodeMulRegReg>();
                auto x = (nodes::NodeMulRegReg *)ptr.get();
                x->dest_regr = dest_regr->second;
                x->src_reg = temp->second;
            }
        }
        else
            lexer.parse_error("Expected a register or a variable in floating point instruction.");
    }
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}

void masm::parser::Parser::handle_inst_fdiv()
{
    auto temp_curr_tok = curr_tok;
    nodes::NodeKind kind;
    std::unique_ptr<nodes::Base> ptr;
    next_token();
    if (curr_tok.type == lexer::_TT_IDENTIFIER)
    {
        auto dest_regr = nodes::_regr_iden_map.find(curr_tok.value);
        if (dest_regr == nodes::_regr_iden_map.end())
            lexer.parse_err_previous_token(curr_tok.value, "Expected a destination register.");
        next_token();
        if (curr_tok.type == lexer::_TT_IDENTIFIER)
        {
            auto temp = nodes::_regr_iden_map.find(curr_tok.value);
            if (temp == nodes::_regr_iden_map.end())
            {
                kind = temp_curr_tok.type == lexer::_TT_INST_FDIV ? nodes::_INST_FDIV_IMM : nodes::_INST_LFDIV_IMM;
                ptr = std::make_unique<nodes::NodeDivRegImm>();
                auto x = (nodes::NodeDivRegImm *)ptr.get();
                x->dest_regr = dest_regr->second;
                x->is_iden = true;
                x->value = curr_tok.value;
            }
            else
            {
                kind = temp_curr_tok.type == lexer::_TT_INST_FDIV ? nodes::_INST_FDIV_REG : nodes::_INST_LFDIV_REG;
                ptr = std::make_unique<nodes::NodeMulRegReg>();
                auto x = (nodes::NodeMulRegReg *)ptr.get();
                x->dest_regr = dest_regr->second;
                x->src_reg = temp->second;
            }
        }
        else
            lexer.parse_error("Expected a register or a variable in floating point instruction.");
    }
    nodes.push_back(std::make_unique<nodes::Node>(nodes::_TYPE_INST, kind, std::move(ptr), lexer.get_curr_line()));
}
