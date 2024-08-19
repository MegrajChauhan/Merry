#include "context.hpp"

int main()
{
    masm::Context _c;
    _c.init_context("test.mb");
    _c.start();
    return 0;
}