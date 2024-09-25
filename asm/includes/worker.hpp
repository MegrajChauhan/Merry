/*
 * MIT License
 *
 * Copyright (c) 2024 MegrajChauhan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef _WORKER_
#define _WORKER_

#include <vector>
#include <string>
#include <filesystem>
#include <unordered_set>
#include <fstream>
#include "symtable.hpp"
#include "nodes.hpp"
#include "expr.hpp"
#include "codegen.hpp"

namespace masm
{
    static std::unordered_set<std::string> used_files;
    static std::string eepe = "1";
    static std::unordered_map<std::string, std::string> teepe;
    static std::vector<std::string> entries;
    static std::unordered_map<std::string, bool> proc_list;
    static std::unordered_set<std::string> lbl_list;
    static std::unordered_map<std::string, size_t> data_addr;
    static std::unordered_map<std::string, size_t> label_addr;
    static SymbolTable symtable;
    static Expr evaluator;
    static std::vector<uint8_t> data, str;
    static std::vector<Node> nodes;

    class Parser
    {
        Lexer l;
        std::string fname, fconts;
        bool read_again = true;
        Token old_tok;
        std::shared_ptr<std::string> file;
        std::vector<bool> _end_queue;
        bool skip = false;

    public:
        Parser() = default;

        void setup_parser(std::string filename);

        void parse();

        void add_for_codegen(CodeGen *g);

        void add_for_emit(Emit *e);

        // called after parsing is completely done
        void parser_confirm_info();

        bool handle_defines(DataType t, bool _const = false);

        bool handle_strings();

        bool add_variable(Variable v);

        bool add_const(Variable v);

        bool handle_names(bool _proc);

        void handle_one(NodeKind k);

        bool handle_arithmetic(NodeKind k);

        bool handle_mov(NodeKind k);

        bool handle_arithmetic_float(NodeKind k);

        bool handle_movX(NodeKind k);

        // making sure that this can be re-used
        bool handle_jmp(NodeKind k);

        bool handle_call();

        bool handle_sva_svc(NodeKind k);

        bool handle_push_pop(NodeKind k);

        bool handle_single_regr(NodeKind k);

        bool handle_logical_inst(NodeKind k, bool limit);

        bool handle_lea();

        bool handle_load_store(NodeKind k, bool atm = false);

        bool handle_atm();

        bool handle_cmpxchg();

        bool handle_sio(NodeKind k);

        bool handle_excg(NodeKind k);

        bool handle_intr();

        void handle_defined();

        void handle_ndefined();

        void handle_depends();

        void handle_entry();

        void handle_eepe();

        void handle_teepe();

        void confirm_entries();

        void analyse_proc();

        void make_label_address();

        void analyse_nodes();
    };

    static std::unordered_map<std::string, std::string> _std_paths = {
#ifdef _USE_LINUX_
        {"stdinit.asm", "lib/stdinit.asm"},
        {"stdutils.asm", "lib/stdutils.asm"},
        {"stdmem.asm", "lib/stdmem.asm"},
        {"stddefs.asm", "lib/stddefs.asm"},
        {"stdsync.asm", "lib/stdsync.asm"},
        {"_builtinalloc_.asm", "lib/internal/mem/_builtinalloc_.asm"},
        {"_builtinmcvars_.asm", "lib/internal/mt/_builtinmcvars_.asm"},
        {"_builtinmlocks_.asm", "lib/internal/mt/_builtinmlocks_.asm"},
        {"_builtinmt_.asm", "lib/internal/mt/_builtinmt_.asm"},
        {"_builtinmp_.asm", "lib/internal/mp/_builtinmp_.asm"},
        {"_builtininit_.asm", "lib/internal/init/_builtininit_.asm"},
        {"_builtindefs_.asm", "lib/internal/utils/_builtindefs_.asm"},
        {"_builtinintr_.asm", "lib/internal/utils/_builtinintr_.asm"},
        {"_builtinutils_.asm", "lib/internal/utils/_builtinutils_.asm"},
        {"_builtinerrno_.asm", "lib/internal/error/_builtinerrno_.asm"},
        {"_builtinerrconsts_.asm", "lib/internal/error/_builtinerrconsts_.asm"},
        {"_builtindyn_.asm", "lib/internal/bif/_builtindyn_.asm"},
        {"_builtincont_.asm", "lib/internal/base/_builtincont_.asm"},
        {"_builtinplatformconstants_.asm", "lib/internal/arch/x86_64/linux/_builtinplatformconstants_.asm"},
        {"_builtinsyscall_.asm", "lib/internal/arch/x86_64/linux/_builtinsyscall_.asm"},
        {"_syscalls_.asm", "lib/internal/arch/_syscalls_.asm"},
        {"stdfio.asm", "lib/stdfio.asm"},
        {"stddyn.asm", "lib/stdmp.asm"},
        {"stdmp.asm", "lib/stdmp.asm"},
        {"stdmt.asm", "lib/stdmt.asm"},
        {"stdcont.asm", "lib/stdcont.asm"},
#endif
    };
};

#endif