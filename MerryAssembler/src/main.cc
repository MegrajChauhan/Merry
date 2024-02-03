#include <iostream>
#include <vector>
#include "../includes/token.hh"
#include "../includes/lexer.hh"
#include "../includes/core.hh"
#include "../includes/parser.hh"
#include "../includes/sema.hh"

int main(){
    merry::front_end::Lexer lexer(merry::core::readFile("./tests/main.merry"), "./tests/main.merry");
    merry::front_end::Parser parser(lexer);
    merry::front_end::Ast ast = parser.get_ast();
    merry::front_end::Sema sema(ast);
    ast = sema.resolve_ast();
    ast.print();
}