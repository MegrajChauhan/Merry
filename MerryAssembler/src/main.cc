#include <iostream>
#include <vector>
#include "../includes/token.hh"
#include "../includes/lexer.hh"
#include "../includes/core.hh"
#include "../includes/parser.hh"

int main(){
    merry::front_end::Lexer lexer(merry::core::readFile("./tests/main.merry"), "./tests/main.merry");
    merry::front_end::Parser parser(lexer);
    merry::front_end::Ast ast = parser.get_ast();
    ast.print();
}