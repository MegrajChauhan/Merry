#include "../includes/parser.hh"

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