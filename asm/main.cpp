#include "context.hpp"

int main()
{
    masm::Context _c;
    _c.init_context("tests/masm/test.mb");
    _c.start();
    return 0;
}