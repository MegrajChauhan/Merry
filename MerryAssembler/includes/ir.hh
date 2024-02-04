#if !defined(_MERRY_ASM_IR_)
#define _MERRY_ASM_IR_
#include "core.hh"
#include "ast.hh"

namespace merry{
namespace back_end{
    enum struct IrInstType{
        NOP=0x00,
        HLT=0x01,
        ADD_IMM=0x02,
        ADD_REG=0x03,
        SUB_IMM=0x04,
        SUB_REG=0x05,
        MUL_IMM=0x06,
        MUL_REG=0x07,
        DIV_IMM=0x08,
        DIV_REG=0x09,
        MOD_IMM=0x0a,
        MOD_REG=0x0b,
        JMP_ADDR=0x27,
        CALL=0x28,
        RET=0x29,
    };
    class IrInst{
        private:
            IrInstType _type;
            std::vector<std::uint8_t> operands;
        public:
            IrInst(IrInstType type = IrInstType::NOP) :_type(type) {}
            void add_operand(std::uint8_t op) { operands.push_back(op); }
            IrInstType get_type() { return _type; }
            std::vector<std::uint8_t> get_operands() { return operands; } 
    };
    class IrGen{
        private:
            front_end::Ast _ast;
            size_t idx = 0;
            front_end::AstNodeInst current_node;
            std::vector<IrInst> insts;
        public:
            IrGen(front_end::Ast ast) :_ast(ast){
                idx = 0;
            }
            void add_inst(IrInst inst) { insts.push_back(inst); }
            void convert_astnode_to_inst();
            std::vector<IrInst> get_insts() {
                for(std::variant<front_end::AstNodeInst, front_end::AstNodeLabel> inst : _ast.get_insts()){
                    current_node = std::get<front_end::AstNodeInst>(_ast.get_insts().at(idx));
                    convert_astnode_to_inst();
                    ++idx;
                }
                return insts;
            }
    };
};
};


#endif // _MERRY_ASM_IR_
