#if !defined(_MERRY_ASM_CODEGEN_MERRY_)
#define _MERRY_ASM_CODEGEN_MERRY_
#include <fstream>
#include "../core.hh"
#include "../ir.hh"

namespace merry{
namespace code_gen{
    class MerryGen{
        public:
            MerryGen(std::vector<back_end::IrInst> insts) :_insts(insts){
                
                myfile.open("./inpFile.mbin");
            }
            void write_header(){
                std::vector<char> header = {
                    0x4d, 0x49, 0x4e, 0x00, 0x00, 0x00, 0x00, 0x00
                };
                myfile.write(header.data(), 8);
                header.clear();
                // Insts size
                uint64_t insts_size = _insts.size();
                uint8_t upper_a = uint8_t((insts_size >> 56) & 0xff);
                uint8_t lower_a = uint8_t((insts_size >> 48) & 0xff);
                uint8_t upper_b = uint8_t((insts_size >> 40) & 0xff);
                uint8_t lower_b = uint8_t((insts_size >> 32) & 0xff);
                uint8_t upper_c = uint8_t((insts_size >> 24) & 0xff);
                uint8_t lower_c = uint8_t((insts_size >> 16) & 0xff);
                uint8_t upper_d = uint8_t((insts_size >> 8) & 0xff);
                uint8_t lower_d = uint8_t((insts_size >> 0) & 0xff);
                header.push_back(lower_d);
                header.push_back(upper_d);
                header.push_back(lower_c);
                header.push_back(upper_c);
                header.push_back(lower_b);
                header.push_back(upper_b);
                header.push_back(lower_a);
                header.push_back(upper_a);
                myfile.write(header.data(), 8);
                // Data
                header.clear();
                for(int i = 0; i < 8; ++i){
                    header.push_back(0);
                }
                myfile.write(header.data(), 8);
            }
            void emit_inst(){
                // Do some stuff that makes it a decimal (no idea what it does)
                char type = (int)_inst.get_type();
                myfile.write(reinterpret_cast<char*>(&type), sizeof(type));
                std::vector<std::uint8_t> operands = _inst.get_operands();
                for(int i = 0; i < 7; ++i) {
                    if (i < (int)operands.size()) {
                        myfile.write(reinterpret_cast<char*>(&operands[i]), sizeof(uint8_t));
                    } else {
                        std::uint8_t dumb_cxx_shit_that_doesnt_work = 0;
                        myfile.write(reinterpret_cast<char*>(&dumb_cxx_shit_that_doesnt_work), sizeof(uint8_t));
                    }
                }
            }
            void emit_all_insts(){
                write_header();
                for(back_end::IrInst inst : _insts){
                    _inst = inst;
                    emit_inst();
                }
            }
        private:
            std::vector<back_end::IrInst> _insts;
            back_end::IrInst _inst;
            std::ofstream myfile;
    };
};
};

#endif // _MERRY_ASM_CODEGEN_MERRY_
