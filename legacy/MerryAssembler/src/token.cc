#include "../includes/token.hh"

void merry::front_end::Token::print(){
    std::cout << static_cast<int>(_type) << ":" << _data << std::endl;
}