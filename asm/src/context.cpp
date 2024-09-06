#include "context.hpp"

void masm::Context::init_context(std::string path)
{
    inp_file = path;
    if (!path.ends_with(".mb"))
    {
        note("The given input file is not a valid input file.");
        die(1);
    }
    if (!std::filesystem::exists(inp_file))
    {
        note("The given input file " + inp_file + " doesn't exist.");
        die(1);
    }
    if (std::filesystem::is_directory(inp_file))
    {
        note("The given input file is a directory and not a file.");
        die(1);
    }
    std::fstream ins(inp_file, std::ios::in);
    while (!ins.eof())
    {
        std::string l;
        std::getline(ins, l);
        inp_file_conts += l + '\n';
    }
    ins.close();
    evaluator.add_table(&table);
    evaluator.add_addr(&data_addr);
}

void masm::ChildContext::init_context(std::string path)
{
    inp_file = path;
    if (!path.ends_with(".mb"))
    {
        note("The given input file is not a valid input file.");
        die(1);
    }
    if (!std::filesystem::exists(inp_file))
    {
        note("The given input file " + inp_file + " doesn't exist.");
        die(1);
    }
    if (std::filesystem::is_directory(inp_file))
    {
        note("The given input file is a directory and not a file.");
        die(1);
    }
    std::fstream ins(inp_file, std::ios::in);
    while (!ins.eof())
    {
        std::string l;
        std::getline(ins, l);
        inp_file_conts += l + '\n';
    }
    ins.close();
}

void masm::Context::read_file(std::string file)
{
    std::fstream _s(file, std::ios::in);
    curr_file = file;
    if (file.ends_with(".mdat"))
        curr_file_type = DATA;
    else if (file.ends_with(".masm"))
        curr_file_type = CODE;
    else
    {
        err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col() - file.length(), inp_file_lexer.get_col(), building, invalext, ERR_STR, "Unknown file type. Expected \".mdat\" or \".masm\"", inp_file_lexer.extract_line());
        die(1);
    }
    curr_file_type = (file.ends_with(".mdat") ? DATA : CODE);
    curr_file_conts = "";
    while (!_s.eof())
    {
        std::string l;
        std::getline(_s, l);
        curr_file_conts += l + '\n';
    }
    _s.close();
}

void masm::ChildContext::read_file(std::string file)
{
    std::fstream _s(file, std::ios::in);
    curr_file = file;
    if (file.ends_with(".mdat"))
        curr_file_type = DATA;
    else if (file.ends_with(".masm"))
        curr_file_type = CODE;
    else
    {
        err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col() - file.length(), inp_file_lexer.get_col(), building, invalext, ERR_STR, "Unknown file type. Expected \".mdat\" or \".masm\"", inp_file_lexer.extract_line());
        die(1);
    }
    curr_file_type = (file.ends_with(".mdat") ? DATA : CODE);
    curr_file_conts = "";
    while (!_s.eof())
    {
        std::string l;
        std::getline(_s, l);
        curr_file_conts += l + '\n';
    }
    _s.close();
}

void masm::Context::start()
{
    // lex the input file and parse it by ourselves
    inp_file_lexer.setup_lexer(std::make_shared<std::string>(inp_file_conts), std::make_shared<std::string>(inp_file));
    while (!inp_file_lexer.eof())
    {
        auto tok = inp_file_lexer.next_token();
        if (!tok.has_value())
        {
            if (!inp_file_lexer.eof())
                die(1);
            break;
        }
        Token t = tok.value();
        switch (t.type)
        {
        case MB_DATA:
        {
            if (skip)
                break;
            Data d;
            std::string _file = inp_file_lexer.get_a_group();
            if (_file.empty() && inp_file_lexer.eof())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a data file after 'data' that needs to be read.", inp_file_lexer.extract_line());
                die(1);
            }
            if (setup_for_new_file(_file))
            {
                d.setup_for_read(&table, std::make_shared<std::string>((flist[flist.size() - 1])), std::make_shared<std::string>(curr_file_conts));
                if (!d.read_data())
                    die(1);
            }
            break;
        }
        case MB_CODE:
        {
            if (skip)
                break;
            Code _c;
            std::string _file = inp_file_lexer.get_a_group();
            if (_file.empty() && inp_file_lexer.eof())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a code file after 'code' that needs to be read.", inp_file_lexer.extract_line());
                die(1);
            }
            if (setup_for_new_file(_file))
            {
                _c.setup_code_read(&nodes, &proc_list, std::make_shared<std::string>((flist[flist.size() - 1])), std::make_shared<std::string>(curr_file_conts), &table, &labels);
                if (!_c.read_code())
                    die(1);
            }
            break;
        }
        case KEY_DEPENDS:
        {
            if (skip)
                break;
            std::string _file = inp_file_lexer.get_a_group();
            if (_file.empty() && inp_file_lexer.eof())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a BUILD file after 'depends' that needs to be read.", inp_file_lexer.extract_line());
                die(1);
            }
            ChildContext cont(&eepe);
            std::string _p = _file;
            if (_std_paths.find(_p) == _std_paths.end())
                _p = std::filesystem::current_path() / _p;
            else
                _p = _std_paths.find(_p)->second;
            cont.init_context(_p);
            cont.setup_structure(&data_addr, &label_addr, &teepe, &table, &filelist, &flist, &nodes, &proc_list, &labels, &entries);
            cont.evaluator.add_addr(&data_addr);
            cont.evaluator.add_table(&table);
            cont.start();
            break;
        }
        case KEY_DEFINED:
            handle_defined();
            break;
        case KEY_NDEFINED:
            handle_ndefined();
            break;
        case KEY_ENTRY:
        {
            if (skip)
                break;
            auto res = inp_file_lexer.next_token();
            if (!res.has_value())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a label or a procedure name after 'entry'.", inp_file_lexer.extract_line());
                die(1);
            }
            t = res.value();
            if (t.type != IDENTIFIER)
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a label or a procedure name after 'entry'.", inp_file_lexer.extract_line());
                die(1);
            }
            entries.push_back(t.val);
            break;
        }
        case KEY_EEPE:
        {
            if (skip)
                break;
            auto res = inp_file_lexer.next_token();
            if (!res.has_value())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected an integer value after 'eepe'.", inp_file_lexer.extract_line());
                die(1);
            }
            t = res.value();
            if (t.type != NUM_INT)
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected an integer value after 'eepe'.", inp_file_lexer.extract_line());
                die(1);
            }
            eepe = t.val;
            break;
        }
        case KEY_TEEPE:
        {
            if (skip)
                break;
            auto res = inp_file_lexer.next_token();
            if (!res.has_value())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected an integer value after 'eepe'.", inp_file_lexer.extract_line());
                die(1);
            }
            t = res.value();
            if (t.type != IDENTIFIER)
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a label value after 'teepe'.", inp_file_lexer.extract_line());
                die(1);
            }
            std::string id = t.val;
            res = inp_file_lexer.next_token();
            if (!res.has_value())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected an integer value after 'eepe'.", inp_file_lexer.extract_line());
                die(1);
            }
            t = res.value();
            if (t.type != NUM_INT)
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected an integer value after 'eepe'.", inp_file_lexer.extract_line());
                die(1);
            }
            teepe[id] = t.val;
            break;
        }
        case KEY_END:
        {
            if (_end_queue.empty())
            {
                note("It may seem that a stary END keyword was found.");
                die(1);
            }
            _end_queue.pop_back();
            if (_end_queue.empty())
                skip = false;
            break;
        }
        }
    }
    if (!_end_queue.empty())
    {
        note("It may seem that one END keyword is missing.");
        die(1);
    }
    analyse_proc();
    confirm_entries();
    gen.setup_codegen(&table, &nodes, &label_addr, &data_addr);
    gen.generate_data();
    make_node_analysis();
    gen.give_address_to_labels();
    /// NOTE: The following change that I made to "label_addr" is an idiotic move
    // It would have been better with just one function.
    // I am too lazy to change it back so a good lesson.
    gen.generate();
}

void masm::Context::confirm_entries()
{
    if (entries.empty())
    {
        note("No entries were provided. At least one entry must be provided.");
        die(1);
    }
    for (auto e : entries)
    {
        if (labels.find(e) == labels.end())
        {
            note("The entry '" + e + "' doesn't exist.");
            die(1);
        }
    }
    for (auto te : teepe)
    {
        if (labels.find(te.first) == labels.end())
        {
            note("The entry '" + te.first + "' doesn't exist.");
            die(1);
        }
    }
}

void masm::ChildContext::start()
{
    // lex the input file and parse it by ourselves
    inp_file_lexer.setup_lexer(std::make_shared<std::string>(inp_file_conts), std::make_shared<std::string>(inp_file));
    while (!inp_file_lexer.eof())
    {
        auto tok = inp_file_lexer.next_token();
        if (!tok.has_value())
        {
            if (!inp_file_lexer.eof())
                die(1);
            break;
        }
        Token t = tok.value();
        switch (t.type)
        {
        case MB_DATA:
        {
            if (skip)
                break;
            Data d;
            std::string _file = inp_file_lexer.get_a_group();
            if (_file.empty() && inp_file_lexer.eof())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a data file after 'data' that needs to be read.", inp_file_lexer.extract_line());
                die(1);
            }
            if (setup_for_new_file(_file))
            {
                d.setup_for_read(table, std::make_shared<std::string>(((*flist)[flist->size() - 1])), std::make_shared<std::string>(curr_file_conts));
                if (!d.read_data())
                    die(1);
            }
            break;
        }
        case MB_CODE:
        {
            if (skip)
                break;
            Code _c;
            std::string _file = inp_file_lexer.get_a_group();
            if (_file.empty() && inp_file_lexer.eof())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a code file after 'code' that needs to be read.", inp_file_lexer.extract_line());
                die(1);
            }
            if (setup_for_new_file(_file))
            {
                _c.setup_code_read(nodes, proc_list, std::make_shared<std::string>(((*flist)[flist->size() - 1])), std::make_shared<std::string>(curr_file_conts), table, labels);
                if (!_c.read_code())
                    die(1);
            }
            break;
        }
        case KEY_DEPENDS:
        {
            if (skip)
                break;
            std::string _file = inp_file_lexer.get_a_group();
            if (_file.empty() && inp_file_lexer.eof())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a BUILD file after 'depends' that needs to be read.", inp_file_lexer.extract_line());
                die(1);
            }
            ChildContext cont(eepe);
            std::string _p = _file;
            if (_std_paths.find(_p) == _std_paths.end())
                _p = std::filesystem::current_path() / _p;
            else
                _p = _std_paths.find(_p)->second;
            cont.init_context(_p);
            cont.setup_structure(data_addr, label_addr, teepe, table, filelist, flist, nodes, proc_list, labels, entries);
            cont.evaluator.add_addr(data_addr);
            cont.evaluator.add_table(table);
            cont.start();
            break;
        }
        case KEY_DEFINED:
            handle_defined();
            break;
        case KEY_NDEFINED:
            handle_ndefined();
            break;
        case KEY_ENTRY:
        {
            if (skip)
                break;
            auto res = inp_file_lexer.next_token();
            if (!res.has_value())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a label or a procedure name after 'entry'.", inp_file_lexer.extract_line());
                die(1);
            }
            t = res.value();
            if (t.type != IDENTIFIER)
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a label or a procedure name after 'entry'.", inp_file_lexer.extract_line());
                die(1);
            }
            entries->push_back(t.val);
            break;
        }
        case KEY_EEPE:
        {
            if (skip)
                break;
            auto res = inp_file_lexer.next_token();
            if (!res.has_value())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected an integer value after 'eepe'.", inp_file_lexer.extract_line());
                die(1);
            }
            t = res.value();
            if (t.type != NUM_INT)
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected an integer value after 'eepe'.", inp_file_lexer.extract_line());
                die(1);
            }
            *eepe = t.val;
            break;
        }
        case KEY_TEEPE:
        {
            if (skip)
                break;
            auto res = inp_file_lexer.next_token();
            if (!res.has_value())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected an integer value after 'eepe'.", inp_file_lexer.extract_line());
                die(1);
            }
            t = res.value();
            if (t.type != IDENTIFIER)
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a label value after 'teepe'.", inp_file_lexer.extract_line());
                die(1);
            }
            std::string id = t.val;
            res = inp_file_lexer.next_token();
            if (!res.has_value())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected an integer value after 'eepe'.", inp_file_lexer.extract_line());
                die(1);
            }
            t = res.value();
            if (t.type != NUM_INT)
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected an integer value after 'eepe'.", inp_file_lexer.extract_line());
                die(1);
            }
            (*teepe)[id] = t.val;
            break;
        }
        case KEY_END:
        {
            if (_end_queue.empty())
            {
                note("It may seem that a stary END keyword was found.");
                die(1);
            }
            _end_queue.pop_back();
            if (_end_queue.empty())
                skip = false;
            break;
        }
        }
    }
    if (!_end_queue.empty())
    {
        note("It may seem that one END keyword is missing. In file " + inp_file);
        die(1);
    }
}

bool masm::Context::setup_for_new_file(std::string npath)
{
    if (_std_paths.find(npath) == _std_paths.end())
        npath = std::filesystem::current_path() / npath;
    else
        npath = _std_paths.find(npath)->second;
    auto res = filelist.find(npath);
    if (res != filelist.end())
        return false; // file already included
    if (!std::filesystem::exists(npath))
    {
        note("The file " + npath + " doesn't exist.");
        return false;
    }
    read_file(npath);
    filelist[npath] = true;
    flist.push_back(npath);
    return true;
}

bool masm::ChildContext::setup_for_new_file(std::string npath)
{
    if (_std_paths.find(npath) == _std_paths.end())
        npath = std::filesystem::current_path() / npath;
    else
        npath = _std_paths.find(npath)->second;
    auto res = filelist->find(npath);
    if (res != filelist->end())
        return false; // file already included
    if (!std::filesystem::exists(npath))
    {
        note("The file " + npath + " doesn't exist.");
        return false;
    }
    read_file(npath);
    (*filelist)[npath] = true;
    flist->push_back(npath);
    return true;
}

void masm::Context::analyse_proc()
{
    for (auto p : proc_list)
    {
        if (!p.second.defined)
        {
            note("The procedure \"" + p.first + "\" was not defined.");
            die(1);
        }
    }
}

void masm::ChildContext::setup_structure(std::unordered_map<std::string, size_t> *da, std::unordered_map<std::string, size_t> *lb, std::unordered_map<std::string, std::string> *tp, SymbolTable *t, std::unordered_map<std::string, bool> *fl, std::vector<std::string> *_fl, std::vector<Node> *n, std::unordered_map<std::string, Procedure> *pl, std::unordered_map<std::string, size_t> *ll, std::vector<std::string> *e)
{
    table = t;
    filelist = fl;
    flist = _fl;
    nodes = n;
    proc_list = pl;
    labels = ll;
    entries = e;
    teepe = tp;
    label_addr = lb;
    data_addr = da;
}

void masm::Context::handle_defined()
{
    auto tok = inp_file_lexer.next_token();
    _end_queue.push_back(true);
    if (skip)
        return;
    if (!tok.has_value())
    {
        note("Expected a constant or a variable after 'defined'.");
        die(1);
    }
    Token t = tok.value();
    if (t.type != IDENTIFIER && t.type != EXPR)
    {
        note("Expected a constant, expression or a variable after 'defined'.");
        die(1);
    }
    if (t.type == EXPR)
    {
        evaluator.add_expr(t.expr);
        auto _r = evaluator.evaluate();
        // In the case of defined an ndefined, the expression must evaluate to either 1 or 0
        // In defined, 1 would mean that the expression is evaluated but otherwise in the case of ndefined
        if (!_r.has_value())
        {
            fu_err(curr_file, t.line, "While evaluating expression here.");
            die(1);
        }
        if (std::stoull(_r.value()) == 1)
            return;
    }
    else if (table._var_list.find(t.val) == table._var_list.end())
    {
        if (table._const_list.find(t.val) == table._const_list.end())
        {
            note("Expected a constant or a variable after 'defined' that exists.");
            die(1);
        }
        // a constant hence it is defined
        return;
    }
    skip = true;
}

void masm::Context::handle_ndefined()
{
    auto tok = inp_file_lexer.next_token();
    _end_queue.push_back(true);
    if (skip)
        return;
    if (!tok.has_value())
    {
        note("Expected a constant or a variable after 'defined'.");
        die(1);
    }
    Token t = tok.value();
    if (t.type != IDENTIFIER)
    {
        note("Expected a constant or a variable after 'defined'.");
        die(1);
    }
    if (t.type == EXPR)
    {
        evaluator.add_expr(t.expr);
        auto _r = evaluator.evaluate();
        if (!_r.has_value())
        {
            fu_err(curr_file, t.line, "While evaluating expression here.");
            die(1);
        }
        if (std::stoull(_r.value()) == 0)
            return;
    }
    else if (!(table._var_list.find(t.val) == table._var_list.end()))
    {
        if (!(table._const_list.find(t.val) == table._const_list.end()))
        {
            // not defined so go on
            return;
        }
    }
    skip = true;
}

void masm::ChildContext::handle_ndefined()
{
    auto tok = inp_file_lexer.next_token();
    if (!tok.has_value())
    {
        note("Expected a constant or a variable after 'defined'.");
        die(1);
    }
    Token t = tok.value();
    if (t.type != IDENTIFIER)
    {
        note("Expected a constant or a variable after 'defined'.");
        die(1);
    }
    if (t.type == EXPR)
    {
        evaluator.add_expr(t.expr);
        auto _r = evaluator.evaluate();
        // In the case of defined an ndefined, the expression must evaluate to either 1 or 0
        // In defined, 1 would mean that the expression is evaluated but otherwise in the case of ndefined
        if (!_r.has_value())
        {
            fu_err(curr_file, t.line, "While evaluating expression here.");
            die(1);
        }
        if (std::stoull(_r.value()) == 0)
            return;
    }
    else if (!(table->_var_list.find(t.val) == table->_var_list.end()))
    {
        if (!(table->_const_list.find(t.val) == table->_const_list.end()))
        {
            // not defined so go on
            return;
        }
    }
    // exists so skip until newline
    inp_file_lexer.rid_until('\n');
}

void masm::ChildContext::handle_defined()
{
    auto tok = inp_file_lexer.next_token();
    if (!tok.has_value())
    {
        note("Expected a constant or a variable after 'defined'.");
        die(1);
    }
    Token t = tok.value();
    if (t.type != IDENTIFIER)
    {
        note("Expected a constant or a variable after 'defined'.");
        die(1);
    }
    if (t.type == EXPR)
    {
        evaluator.add_expr(t.expr);
        auto _r = evaluator.evaluate();
        // In the case of defined an ndefined, the expression must evaluate to either 1 or 0
        // In defined, 1 would mean that the expression is evaluated but otherwise in the case of ndefined
        if (!_r.has_value())
        {
            fu_err(curr_file, t.line, "While evaluating expression here.");
            die(1);
        }
        if (std::stoull(_r.value()) == 1)
            return;
    }
    else if (table->_var_list.find(t.val) == table->_var_list.end())
    {
        if (table->_const_list.find(t.val) == table->_const_list.end())
        {
            note("Expected a constant or a variable after 'defined' that exists.");
            die(1);
        }
        // a constant hence it is defined
        return;
    }
    // doesn't exist so skip until newline or another defined or ndefined
    inp_file_lexer.rid_until('\n');
}

std::unordered_map<std::string, size_t> *masm::Context::get_lbl_addr()
{
    return &label_addr;
}

std::unordered_map<std::string, std::string> *masm::Context::get_teepe()
{
    return &teepe;
}

std::string *masm::Context::get_eepe()
{
    return &eepe;
}

std::vector<std::string> *masm::Context::get_entries()
{
    return &entries;
}

masm::CodeGen *masm::Context::get_codegen()
{
    return &gen;
}

void masm::Context::make_node_analysis()
{
    for (auto &n : nodes)
    {
        switch (n.kind)
        {
        case ADD_MEM:
        case SUB_MEM:
        case MUL_MEM:
        case DIV_MEM:
        case MOD_MEM:
        case FADD_MEM:
        case FSUB_MEM:
        case FMUL_MEM:
        case FDIV_MEM:
        case LFADD_MEM:
        case LFSUB_MEM:
        case LFMUL_MEM:
        case LFDIV_MEM:
        {
            auto _n = (NodeArithmetic *)n.node.get();
            std::string var_name = std::get<std::string>(_n->second_oper);
            if (table._var_list.find(var_name) == table._var_list.end())
            {
                // could be a constant
                if (table._const_list.find(var_name) == table._const_list.end())
                {
                    fu_err(*n._file.get(), n.line, "The variable '" + var_name + "' doesn't exist.");
                    die(1);
                }
            }
            break;
        }
        case LOADB_VAR:
        case LOADW_VAR:
        case LOADD_VAR:
        case LOADQ_VAR:
        case STOREB_VAR:
        case STOREW_VAR:
        case STORED_VAR:
        case STOREQ_VAR:
        case ALOADB_VAR:
        case ALOADW_VAR:
        case ALOADD_VAR:
        case ALOADQ_VAR:
        case ASTOREB_VAR:
        case ASTOREW_VAR:
        case ASTORED_VAR:
        case ASTOREQ_VAR:
        {
            auto _n = (NodeArithmetic *)n.node.get();
            std::string var_name = std::get<std::string>(_n->second_oper);
            if (table._var_list.find(var_name) == table._var_list.end())
            {
                fu_err(*n._file.get(), n.line, "The variable '" + var_name + "' doesn't exist.");
                die(1);
            }
            break;
        }
        case ADD_EXPR:
        case SUB_EXPR:
        case MUL_EXPR:
        case DIV_EXPR:
        case MOD_EXPR:
        case IADD_EXPR:
        case ISUB_EXPR:
        case IMUL_EXPR:
        case IDIV_EXPR:
        case IMOD_EXPR:
        case AND_EXPR:
        case OR_EXPR:
        case XOR_EXPR:
        case LSHIFT_EXPR:
        case RSHIFT_EXPR:
        case CMP_EXPR:
        {
            auto _n = (NodeArithmetic *)n.node.get();
            auto expr = std::get<std::vector<Token>>(_n->second_oper);
            evaluator.add_expr(expr);
            auto v = evaluator.evaluate();
            if (!v.has_value())
            {
                fu_err(*n._file.get(), expr[0].line, "While evaluating the expression here.");
                die(1);
            }
            n.kind = (NodeKind)(n.kind - 3);
            _n->second_oper = v.value();
            break;
        }
        case MOV_EXPR:
        case MOVL_EXPR:
        {
            auto _n = (NodeMov *)n.node.get();
            auto expr = std::get<std::vector<Token>>(_n->second_oper);
            evaluator.add_expr(expr);
            auto v = evaluator.evaluate();
            if (!v.has_value())
            {
                fu_err(*n._file.get(), expr[0].line, "While evaluating the expression here.");
                die(1);
            }
            n.kind = MOVL_IMM;
            _n->second_oper = std::make_pair(v.value(), BYTE);
            break;
        }
        case MOVSXB_EXPR:
        case MOVSXW_EXPR:
        case MOVSXD_EXPR:
        case SVA_EXPR:
        case SVC_EXPR:
        {
            auto _n = (NodeMov *)n.node.get();
            auto expr = std::get<std::vector<Token>>(_n->second_oper);
            evaluator.add_expr(expr);
            auto v = evaluator.evaluate();
            if (!v.has_value())
            {
                fu_err(*n._file.get(), expr[0].line, "While evaluating the expression here.");
                die(1);
            }
            n.kind = (NodeKind)(n.kind - 3);
            _n->second_oper = std::make_pair(v.value(), BYTE);
            break;
        }
        case PUSH_EXPR:
        {
            auto _n = (NodePushPop *)n.node.get();
            auto expr = std::get<std::vector<Token>>(_n->val);
            evaluator.add_expr(expr);
            auto v = evaluator.evaluate();
            if (!v.has_value())
            {
                fu_err(*n._file.get(), expr[0].line, "While evaluating the expression here.");
                die(1);
            }
            n.kind = (NodeKind)(n.kind - 3);
            _n->val = v.value();
            break;
        }
        case IADD_VAR:
        case ISUB_VAR:
        case IMUL_VAR:
        case IDIV_VAR:
        case IMOD_VAR:
        case AND_VAR:
        case OR_VAR:
        case XOR_VAR:
        case LSHIFT_VAR:
        case RSHIFT_VAR:
        {
            auto _n = (NodeArithmetic *)n.node.get();
            std::string var_name = std::get<std::string>(_n->second_oper);
            auto r = table._const_list.find(var_name);
            // must be a constant
            if (r == table._const_list.end())
            {
                fu_err(*n._file.get(), n.line, "The constant '" + var_name + "' doesn't exist.");
                die(1);
            }
            n.kind = (NodeKind)(n.kind - 2);
            _n->second_oper = r->second.value;
            break;
        }
        case CMP_VAR:
        {
            auto _n = (NodeArithmetic *)n.node.get();
            std::string var_name = std::get<std::string>(_n->second_oper);
            auto r = table._var_list.find(var_name);
            if (r == table._var_list.end())
            {
                if (table._const_list.find(var_name) == table._const_list.end())
                {
                    fu_err(*n._file.get(), n.line, "The variable '" + var_name + "' doesn't exist.");
                    die(1);
                }
                else
                {
                    n.kind = (NodeKind)(n.kind - 2);
                    _n->second_oper = table._const_list.find(var_name)->second.value;
                }
            }
            break;
        }
        case MOV_VAR:
        case SVA_VAR:
        case SVC_VAR:
        case MOVL_VAR:
        {
            auto _n = (NodeMov *)n.node.get();
            auto var_name = std::get<std::pair<std::string, DataType>>(_n->second_oper);
            auto r = table._const_list.find(var_name.first);
            if (r == table._const_list.end())
            {
                if (n.kind == MOV_VAR || n.kind == MOVL_VAR)
                {
                    if ((labels.find(var_name.first) != labels.end()))
                        break;
                }
                auto _r = table._var_list.find(var_name.first);
                if (_r == table._var_list.end())
                {
                    fu_err(*n._file.get(), n.line, "The variable '" + var_name.first + "' doesn't exist.");
                    die(1);
                }
                break;
            }
            if (n.kind == MOV_VAR || n.kind == MOVL_VAR)
            {
                n.kind = MOVL_IMM;
                _n->second_oper = std::make_pair(r->second.value, BYTE);
            }
            else
            {
                n.kind = (NodeKind)(n.kind - 2);
                _n->second_oper = std::make_pair(r->second.value, BYTE);
            }
            break;
        }
        case MOVSXB_VAR:
        case MOVSXW_VAR:
        case MOVSXD_VAR:
        {
            auto _n = (NodeMov *)n.node.get();
            auto var_name = std::get<std::pair<std::string, DataType>>(_n->second_oper);
            auto r = table._const_list.find(var_name.first);
            if (r == table._const_list.end())
            {
                if (n.kind == MOV_VAR || n.kind == MOVL_VAR)
                {
                    if ((labels.find(var_name.first) != labels.end()))
                        break;
                }
                fu_err(*n._file.get(), n.line, "The variable '" + var_name.first + "' doesn't exist.");
                die(1);
            }
            if (n.kind == MOV_VAR || n.kind == MOVL_VAR)
            {
                n.kind = MOVL_IMM;
                _n->second_oper = std::make_pair(r->second.value, BYTE);
            }
            break;
        }
        case JMP:
        case JNZ:
        case JZ:
        case JNE:
        case JE:
        case JNC:
        case JC:
        case JNO:
        case JO:
        case JNN:
        case JN:
        case JNG:
        case JG:
        case JNS:
        case JS:
        case JGE:
        case JSE:
        case SETE:
        case LOOP:
        {
            auto _n = (NodeName *)n.node.get();
            std::string name = _n->name;
            if (labels.find(name) == labels.end())
            {
                fu_err(*n._file.get(), n.line, "This label to branch into doesn't exist: " + name);
                die(1);
            }
            break;
        }
        case CALL:
        {
            auto _n = (NodeCall *)n.node.get();
            std::string name = std::get<std::string>(_n->_oper);
            if (labels.find(name) == labels.end())
            {
                fu_err(*n._file.get(), n.line, "This label to branch into doesn't exist: " + name);
                die(1);
            }
            break;
        }
        case CMPXCHG:
        {
            auto _n = (NodeCmpxchg *)n.node.get();
            if (table._var_list.find(_n->var) == table._var_list.end())
            {
                fu_err(*n._file.get(), n.line, "This variable doesn't exist: " + _n->var);
                die(1);
            }
            break;
        }
        case SIN:
        case SOUT:
        {
            auto _n = (NodeSIO *)n.node.get();
            if (table._var_list.find(_n->name) == table._var_list.end())
            {
                fu_err(*n._file.get(), n.line, "This variable doesn't exist: " + _n->name);
                die(1);
            }
            break;
        }
        case INTR_VAR:
        {
            auto _n = (NodeIntr *)n.node.get();
            auto name = std::get<std::string>(_n->val);
            if (table._const_list.find(name) == table._const_list.end())
            {
                fu_err(*n._file.get(), n.line, "This constant doesn't exist: " + name);
                die(1);
            }
            n.kind = INTR;
            _n->val = table._const_list.find(name)->second.value;
            break;
        }
        case INTR_EXPR:
        {
            auto _n = (NodeIntr *)n.node.get();
            auto expr = std::get<std::vector<Token>>(_n->val);
            evaluator.add_expr(expr);
            auto v = evaluator.evaluate();
            if (!v.has_value())
            {
                fu_err(*n._file.get(), expr[0].line, "While evaluating the expression here.");
                die(1);
            }
            n.kind = INTR;
            _n->val = v.value();
            break;
        }
        }
    }
}
