#include "../includes/parser.hh"
#include <algorithm>

merry::front_end::Ast merry::front_end::Parser::get_ast(){
    Ast return_ast;
    while(1){
        std::variant<AstNodeInst, AstNodeLabel> next_node = get_next_node();
        return_ast.add_inst_or_label(next_node);
        if(next_node.index() == 1){
            AstNodeLabel label = std::get<AstNodeLabel>(next_node);
            if(label.get_name() == "__last_node__"){
                break;
            }
        }
    }
    return return_ast;
}

std::vector<std::pair<merry::front_end::AstNodeInst, std::string>> instructions = {
    {merry::front_end::AstNodeInst(merry::front_end::AstInstType::NOP), "nop"},
    {merry::front_end::AstNodeInst(merry::front_end::AstInstType::HLT), "hlt"},
    {merry::front_end::AstNodeInst(merry::front_end::AstInstType::ADD), "add"},
    {merry::front_end::AstNodeInst(merry::front_end::AstInstType::SUB), "sub"},
    {merry::front_end::AstNodeInst(merry::front_end::AstInstType::MUL), "mul"},
    {merry::front_end::AstNodeInst(merry::front_end::AstInstType::DIV), "div"},
    {merry::front_end::AstNodeInst(merry::front_end::AstInstType::MOD), "mod"},
    {merry::front_end::AstNodeInst(merry::front_end::AstInstType::JMP), "jmp"},
    {merry::front_end::AstNodeInst(merry::front_end::AstInstType::RET), "ret"},
    {merry::front_end::AstNodeInst(merry::front_end::AstInstType::CALL), "call"},
};

std::vector<std::string> insts_with_ops = {
    "add",
    "sub",
    "mul",
    "div",
    "mod",
    "jmp",
    "call",
};

std::variant<merry::front_end::AstNodeInst, merry::front_end::AstNodeLabel> merry::front_end::Parser::get_next_node(){
    Token current = _lexer.next_token();
    // Parse label decl
    if(_lexer.peek().get_type() == TokenType::COLON){
        std::string name = current.get_data();
        current = _lexer.next_token();
        return AstNodeLabel(name);
    }
    // TODO: Operands
    for(std::pair<merry::front_end::AstNodeInst, std::string> inst : instructions){
        if(inst.second == current.get_data()){
            if(std::find(insts_with_ops.begin(), insts_with_ops.end(), inst.second) == insts_with_ops.end()){
                return inst.first;
            }
            current = _lexer.next_token();
            inst.first.add_operand(current.get_data());
            if(_lexer.peek().get_type() == TokenType::COMMA){
                _lexer.next_token(); // Skip comma
                current = _lexer.next_token();
                std::string data = current.get_data();
                if(_lexer.peek().get_type() == TokenType::TT_EOF){
                    data.pop_back();
                }
                inst.first.add_operand(data);
            }
            return inst.first;
        }
    }
    if(current.get_type() == TokenType::TT_EOF){
        std::string name = "__last_node__";
        return AstNodeLabel(name);
    }
    current.print();
    std::cout << "Invalid character!\n";
    exit(1);
}