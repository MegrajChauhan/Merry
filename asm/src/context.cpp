#include "context.hpp"

void masm::Context::init_context(std::string path)
{
    if (!path.ends_with(".mb"))
    {
        note("The given input file is not a valid input file.");
        die(1);
    }
    inp_file = std::filesystem::current_path() / path;
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
            Data d;
            std::string _file = inp_file_lexer.get_a_group();
            if (_file.empty() && inp_file_lexer.eof())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a data file after 'data' that needs to be read.", inp_file_lexer.extract_line());
                die(1);
            }
            setup_for_new_file(_file);
            d.setup_for_read(&table, std::make_shared<std::string>((flist[flist.size() - 1])), std::make_shared<std::string>(curr_file_conts));
            if (!d.read_data())
                die(1);
            break;
        }
        case MB_CODE:
        {
            Code _c;
            std::string _file = inp_file_lexer.get_a_group();
            if (_file.empty() && inp_file_lexer.eof())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a code file after 'code' that needs to be read.", inp_file_lexer.extract_line());
                die(1);
            }
            setup_for_new_file(_file);
            _c.setup_code_read(&nodes, &proc_list, std::make_shared<std::string>((flist[flist.size() - 1])), std::make_shared<std::string>(curr_file_conts), &table, &labels);
            if (!_c.read_code())
                die(1);
            break;
        }
        case KEY_DEPENDS:
        {
            std::string _file = inp_file_lexer.get_a_group();
            if (_file.empty() && inp_file_lexer.eof())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a BUILD file after 'depends' that needs to be read.", inp_file_lexer.extract_line());
                die(1);
            }
            ChildContext cont(&eepe);
            cont.init_context(std::filesystem::current_path() / _file);
            cont.setup_structure(&teepe, &table, &filelist, &flist, &nodes, &proc_list, &labels, &entries);
            cont.start();
            break;
        }
        case KEY_DEFINED:
            handle_defined();
            break;
        }
    }
    analyse_proc();
    gen.setup_codegen(&table, &nodes);
    gen.generate();
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
            Data d;
            std::string _file = inp_file_lexer.get_a_group();
            if (_file.empty() && inp_file_lexer.eof())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a data file after 'data' that needs to be read.", inp_file_lexer.extract_line());
                die(1);
            }
            setup_for_new_file(_file);
            d.setup_for_read(table, std::make_shared<std::string>((flist[flist->size() - 1])), std::make_shared<std::string>(curr_file_conts));
            if (!d.read_data())
                die(1);
            break;
        }
        case MB_CODE:
        {
            Code _c;
            std::string _file = inp_file_lexer.get_a_group();
            if (_file.empty() && inp_file_lexer.eof())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a code file after 'code' that needs to be read.", inp_file_lexer.extract_line());
                die(1);
            }
            setup_for_new_file(_file);
            _c.setup_code_read(nodes, proc_list, std::make_shared<std::string>((flist[flist->size() - 1])), std::make_shared<std::string>(curr_file_conts), table, labels);
            if (!_c.read_code())
                die(1);
            break;
        }
        case KEY_DEPENDS:
        {
            std::string _file = inp_file_lexer.get_a_group();
            if (_file.empty() && inp_file_lexer.eof())
            {
                err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, ERR_STR, "Expected a BUILD file after 'depends' that needs to be read.", inp_file_lexer.extract_line());
                die(1);
            }
            ChildContext cont(eepe);
            cont.init_context(std::filesystem::current_path() / _file);
            cont.setup_structure(teepe, table, filelist, flist, nodes, proc_list, labels, entries);
            cont.start();
            break;
        }
        case KEY_DEFINED:
            handle_defined();
            break;
        }
    }
}

void masm::Context::setup_for_new_file(std::string npath)
{
    npath = (std::filesystem::current_path() / npath);
    auto res = filelist.find(npath);
    if (res != filelist.end())
        return; // file already included
    read_file(npath);
    filelist[npath] = true;
    flist.push_back(npath);
}

void masm::ChildContext::setup_for_new_file(std::string npath)
{
    npath = (std::filesystem::current_path() / npath);
    auto res = filelist->find(npath);
    if (res != filelist->end())
        return; // file already included
    read_file(npath);
    (*filelist)[npath] = true;
    flist->push_back(npath);
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

void masm::ChildContext::setup_structure(std::unordered_map<std::string, size_t> *tp, SymbolTable *t, std::unordered_map<std::string, bool> *fl, std::vector<std::string> *_fl, std::vector<Node> *n, std::unordered_map<std::string, Procedure> *pl, std::unordered_map<std::string, size_t> *ll, std::vector<std::string> *e)
{
    table = t;
    filelist = fl;
    flist = _fl;
    nodes = n;
    proc_list = pl;
    labels = ll;
    entries = e;
    teepe = tp;
}

void masm::Context::handle_defined()
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
    if (table._var_list.find(t.val) == table._var_list.end())
    {
        if (table._const_list.find(t.val) == table._const_list.end())
        {
            note("Expected a constant or a variable after 'defined' that exists.");
            die(1);
        }
        // a constant hence it is defined
        return;
    }
    // doesn't exist so skip until newline
    inp_file_lexer.rid_until('\n');
}

void masm::Context::handle_ndefined()
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
    if (!(table._var_list.find(t.val) == table._var_list.end()))
    {
        if (!(table._const_list.find(t.val) == table._const_list.end()))
        {
            // not defined so go on
            return;
        }
    }
    // exists so skip until newline
    inp_file_lexer.rid_until('\n');
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
    if (!(table->_var_list.find(t.val) == table->_var_list.end()))
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
    if (table->_var_list.find(t.val) == table->_var_list.end())
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
