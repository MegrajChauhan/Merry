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
        err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col() - file.length(), inp_file_lexer.get_col(), building, invalext, error, "Unknown file type. Expected \".mdat\" or \".masm\"", inp_file_lexer.extract_line());
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
        std::string _file = inp_file_lexer.get_a_group();
        if (_file.empty() && inp_file_lexer.eof())
        {
            err(inp_file, inp_file_lexer.get_line(), inp_file_lexer.get_col(), inp_file_lexer.get_col() + 1, building, syntaxerr, error, "Expected a data file after 'data' that needs to be read.", inp_file_lexer.extract_line());
            die(1);
        }
        setup_for_new_file(_file);
        switch (t.type)
        {
        case MB_DATA:
        {
            Data d;
            d.setup_for_read(&table, std::make_shared<std::string>((flist[flist.size() - 1])), std::make_shared<std::string>(curr_file_conts));
            if (!d.read_data())
                die(1);
            break;
        }
        case MB_CODE:
        {
            Code _c;
            _c.setup_code_read(&nodes, &proc_list, std::make_shared<std::string>((flist[flist.size() - 1])), std::make_shared<std::string>(curr_file_conts), &table, &labels);
            if (!_c.read_code())
                die(1);
            break;
        }
        }
    }
    analyse_proc();
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

void masm::Context::analyse_proc()
{
    for (auto p : proc_list)
    {
        if (!p.second.defined)
        {
            fu_err(*nodes[p.second.ind].file.get(), nodes[p.second.ind].line_st, "Procedure \"" + p.first + "\" was not defined.");
            die(1);
        }
    }
}