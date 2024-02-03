#if !defined(_MERRY_ASM_SEMA_)
#define _MERRY_ASM_SEMA_
#include "ast.hh"

namespace merry{
namespace front_end{
    class Sema{
        private:
            Ast in_ast;
            Ast copy_ast;
        public:
            Sema(Ast ast) :in_ast(ast) {}
            AstNodeInst resolve_inst_imm_or_reg(AstNodeInst inst);
            AstNodeLabel resolve_label_addr(AstNodeLabel label);
            Ast resolve_ast(){
                std::vector<std::variant<AstNodeInst, AstNodeLabel>> insts = in_ast.get_insts();
                for(size_t i = 0; i < insts.size(); ++i){
                    std::variant<AstNodeInst, AstNodeLabel> inst = insts[i];
                    if(inst.index() == 0){
                        copy_ast.add_inst_or_label(resolve_inst_imm_or_reg(std::get<AstNodeInst>(inst)));
                    } else if(inst.index() == 1){
                        copy_ast.add_inst_or_label(resolve_label_addr(std::get<AstNodeLabel>(inst)));
                    }
                }
                return copy_ast;
            }
    };
};
};

#endif // _MERRY_ASM_SEMA_
