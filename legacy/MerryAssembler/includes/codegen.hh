#if !defined(_MERRY_ASM_CODEGEN_)
#define _MERRY_ASM_CODEGEN_
#include "codegen/merry.hh"
#include "core.hh"

namespace merry{
namespace code_gen{
    class Gen{
        public:
            void generate(core::Format fmt, std::vector<back_end::IrInst> insts){
                switch(fmt){
                    default:
                    case core::Format::elf_object:
                    case core::Format::assembly: {
                        std::cout << "UNIMPLEMENTED!!!\n";
                        exit(1);
                    } break;
                    case core::Format::merry: {
                        merry::code_gen::MerryGen gen(insts);
                        gen.emit_all_insts();
                    } break;
                }
            }
    };
};
};

#endif // _MERRY_ASM_CODEGEN_
