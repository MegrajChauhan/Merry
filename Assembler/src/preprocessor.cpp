#include "../includes/preprocessor.hpp"

masm::prep::Prep::Prep(std::string file_name)
{
    _reader_.set_filename(file_name);
    _reader_.setup();
    _file_contents_ = _reader_.read();
    curr_char = _file_contents_.begin();
    line_no = 1;
    col_no = 0;
}

void masm::prep::Prep::setup(std::string file_name)
{
    _reader_.set_filename(file_name);
    _reader_.setup();
    _file_contents_ = _reader_.read();
    curr_char = _file_contents_.begin();
    line_no = 1;
    col_no = 0;
}

std::string masm::prep::Prep::get_curr_line()
{
    std::string _line_;
    for (; _file_contents_[_start_] != '\n' && _start_ < _file_contents_.length(); _start_++)
    {
        _line_ += _file_contents_[_start_];
    }
    return _line_;
}

void masm::prep::Prep::prep()
{
    _included_files_.push_back(_reader_.get_file_name());
    get_all_includes(_included_files_);
    _second_pass_();
    _third_pass_();
}

void masm::prep::Prep::handle_defines()
{
    // we need a character here or a '_'
    // no numbers
    if (*curr_char == '\n' || curr_char == _file_contents_.end())
    {
        error::_print_err_(_reader_.get_file_name(), line_no, col_no, _CONTEXT_PREP_, "Expected a preprocessing symbol after 'define'.", get_curr_line());
    }
    std::string _sym_name_;
    if (*curr_char == ' ' || *curr_char == '\t')
    {
        while (*curr_char == ' ' || *curr_char == '\t')
        {
            col_no++;
            curr_char++;
        }
    }
    if (*curr_char == '\n' || curr_char == _file_contents_.end())
    {
        error::_print_err_(_reader_.get_file_name(), line_no, col_no, _CONTEXT_PREP_, "Expected a preprocessing symbol after 'define'.", get_curr_line());
    }

    while ((std::isalpha(*curr_char) || *curr_char == '_') && curr_char != _file_contents_.end())
    {
        _sym_name_ += *curr_char;
        curr_char++;
        col_no++;
    }

    if (*curr_char == ' ' || *curr_char == '\t')
    {
        while (*curr_char == ' ' || *curr_char == '\t')
        {
            col_no++;
            curr_char++;
        }
    }

    std::string _val_;
    while (*curr_char != '\n' && curr_char != _file_contents_.end())
    {
        _val_ += *curr_char;
        curr_char++;
        col_no++;
    }
    // the symbol can have no value at all as well

    syms._syms_[_sym_name_] = _val_;
    if (curr_char != _file_contents_.end())
    {
        _start_ += col_no + 1;
        line_no++;
        col_no = 0;
        curr_char++;
    }
    // if _sym_name_ was already defined then the value will change
}

void masm::prep::Prep::get_all_includes(std::vector<std::string> list = {})
{
    /*
       Go through each line to specifically encounter includes
       Make a list of all files included.
       Go through all the included files and make the same list.
       Make a final file that makes all includes
       Finally preprocess it
       This makes for a Two-Pass Preprocessor
    */
    _included_files_ = list;
    std::string modified_file;
    size_t open_count = 0;
    size_t close_count = 0;
    while (curr_char != _file_contents_.end())
    {
        if (*curr_char == ';')
        {
            if (curr_char != _file_contents_.end() && *(curr_char + 1) == ';')
            {
                modified_file += '\n';
                while (*curr_char != '\n' && curr_char != _file_contents_.end())
                {
                    col_no++;
                    curr_char++;
                }
            }
        }
        if (*curr_char == '\n')
        {
            _start_ += col_no + 1;
            col_no = 0;
            line_no++;
            modified_file += *curr_char;
            curr_char++;
        }
        else if (*curr_char == '$')
        {
            curr_char++;
            if (*curr_char == ' ' || *curr_char == '\t')
            {
                while (*curr_char == ' ' || *curr_char == '\t')
                {
                    col_no++;
                    curr_char++;
                }
            }
            if (*curr_char == '\n')
            {
                error::_print_err_(_reader_.get_file_name(), line_no, col_no, _CONTEXT_PREP_, "Expected include, define directives after '$'.", get_curr_line());
            }
            std::string temp;
            while (std::isalpha(*curr_char) && curr_char != _file_contents_.end())
            {
                temp += *curr_char;
                curr_char++;
                col_no++;
            }
            if (temp == "define")
            {
                if (open_count == close_count)
                {
                    modified_file += "\n";
                    handle_defines();
                }
                else
                {
                    modified_file += "$";
                    modified_file += temp;
                }
            }
            else if (temp == "ifdef" || temp == "ifndef")
            {
                modified_file += "$";
                modified_file += temp;
                open_count++;
            }
            else if (temp == "undef")
            {
                modified_file += "$";
                modified_file += temp;
            }
            else if (temp == "endif")
            {
                modified_file += "$";
                modified_file += temp;
                close_count++;
            }
            else if (temp == "include")
            {
                std::string path;
                if (*curr_char == '\n' || curr_char == _file_contents_.end())
                {
                    error::_print_err_(_reader_.get_file_name(), line_no, col_no, _CONTEXT_PREP_, "Expected a path after 'include'.", get_curr_line());
                }
                curr_char++;
                if (*curr_char == ' ' || *curr_char == '\t')
                {
                    while (*curr_char == ' ' || *curr_char == '\t')
                    {
                        col_no++;
                        curr_char++;
                    }
                }
                while (*curr_char != '\n' && curr_char != _file_contents_.end())
                {
                    path += *curr_char;
                    curr_char++;
                    col_no++;
                }
                path = std::filesystem::path(std::filesystem::current_path() / path).generic_string();
                modified_file += "#" + path;
                if (std::filesystem::exists(path))
                {
                    auto ret = std::find(_included_files_.begin(), _included_files_.end(), path);
                    if (ret == _included_files_.end())
                    {
                        // This is a new path that wasn't in the list
                        _included_files_.push_back(path);
                        _self_files_.push_back(path);
                    }
                }
            }
            else
            {
                error::_print_err_(_reader_.get_file_name(), line_no, col_no, _CONTEXT_PREP_, "Expected include, define directives after '$' got something else.", get_curr_line());
            }
        }
        else
        {
            if (curr_char != _file_contents_.end())
            {
                modified_file += *curr_char;
                curr_char++;
                col_no++;
            }
        }
    }
    _file_contents_ = modified_file;
    // If we have reached this place that means we now have a list of all included files in the main file
    // We now need to go through each of the included files and do the same to create a tree like structure
    for (auto path : _self_files_)
    {
        // We need to create a lot of Preprocessor instances for each included file
        prep::Prep prep(path);
        prep.get_all_includes(_included_files_);
        // For each included file create a new FileDetails
        FileDetails details;
        details._file_contents_ = prep._file_contents_;
        details.lines = prep.line_no + 1;
        _details_[path] = details;
        _details_.merge(prep._details_);
        _included_files_ = prep._included_files_;
        syms._syms_.merge(prep.syms._syms_);
    }
}

std::string masm::prep::Prep::get_the_sym(std::string filename)
{
    if (*curr_char == ' ' || *curr_char == '\t')
    {
        while (*curr_char == ' ' || *curr_char == '\t')
        {
            col_no++;
            curr_char++;
        }
    }
    std::string _sym_name_;
    while (std::isalpha(*curr_char) || *curr_char == '_')
    {
        _sym_name_ += *curr_char;
        curr_char++;
        col_no++;
    }
    return _sym_name_;
}

void masm::prep::Prep::remove_until_endif(std::string filename, std::string *contents)
{
    // remove everything until a corresponding "endif" is encountered
    size_t start = line_no;
    size_t opening_count = 1;
    size_t closing_count = 0;
    while (true)
    {
        while (*curr_char != '$' && curr_char != _file_contents_.end())
        {
            if (*curr_char == '\n')
            {
                *contents += *curr_char;
                _start_ += col_no + 1;
                col_no = 0;
                line_no++;
                curr_char++;
            }
            else if (*curr_char == '#')
            {
                *contents += '\n';
                // special case
                // since we are removing this section
                // we don't need to include this too
                curr_char++;
                std::string path;
                while (*curr_char != '\n')
                {
                    path += *curr_char;
                    curr_char++;
                }
                // we will make use of _self_files_ to store those that needs excluding
                _self_files_.push_back(path);
            }
            else
            {
                col_no++;
                curr_char++;
            }
        }
        // we have ourselves a '$
        *contents += '\n';
        curr_char++;
        if (*curr_char == ' ' || *curr_char == '\t')
        {
            while (*curr_char == ' ' || *curr_char == '\t')
            {
                col_no++;
                curr_char++;
            }
        }
        std::string temp;
        while (std::isalpha(*curr_char) && curr_char != _file_contents_.end())
        {
            temp += *curr_char;
            curr_char++;
            col_no++;
        }
        if (temp == "endif")
            closing_count++;
        else if (temp == "ifndef" || temp == "ifdef")
            opening_count++;
        if (opening_count == closing_count)
            break;
        if (curr_char == _file_contents_.end())
            break;
    }
    if (opening_count != closing_count)
    {
        line_no = start;
        error::_print_err_(filename, start, 0, _CONTEXT_PREP_, "No terminating \"endif\" found.", get_curr_line());
    }
}

void masm::prep::Prep::add_until_endif(std::string filename, std::string *contents)
{
    size_t start = line_no;
    size_t opening_count = 1;
    size_t closing_count = 0;
    while (true)
    {
        while (*curr_char != '$' && curr_char != _file_contents_.end())
        {
            if (*curr_char == '\n')
            {
                *contents += *curr_char;
                _start_ += col_no + 1;
                col_no = 0;
                line_no++;
                curr_char++;
            }
            else if (*curr_char == '#')
            {
                *contents += '\n';
                // special case
                // since we are removing this section
                // we don't need to include this too
                curr_char++;
                std::string path;
                while (*curr_char != '\n' && curr_char != _file_contents_.end())
                {
                    path += *curr_char;
                    curr_char++;
                }
                if (!std::filesystem::exists(path))
                    error::_print_err_(filename, line_no, 0, _CONTEXT_PREP_, "This included file doesn't exist. Maybe check the path?", "$include " + path);
            }
            else
            {
                *contents += *curr_char;
                col_no++;
                curr_char++;
            }
        }
        // we have ourselves a '$
        *contents += '\n';
        if (*curr_char == ' ' || *curr_char == '\t')
        {
            while (*curr_char == ' ' || *curr_char == '\t')
            {
                col_no++;
                curr_char++;
            }
        }
        curr_char++;
        std::string temp;
        while (std::isalpha(*curr_char) && curr_char != _file_contents_.end())
        {
            temp += *curr_char;
            curr_char++;
            col_no++;
        }
        if (temp == "endif")
            closing_count++;
        else if (temp == "ifndef")
        {
            auto _sym_to_check_ = get_the_sym(filename);
            auto _res = syms._syms_.find(_sym_to_check_);
            opening_count++;
            if (_res != syms._syms_.end())
            {
                // it is defined and so we have to skip everything until we encounter an "endif"
                remove_until_endif(filename, contents);
            }
            else
            {
                add_until_endif(filename, contents);
            }
        }
        else if (temp == "ifdef")
        {
            auto _sym_to_check_ = get_the_sym(filename);
            auto _res = syms._syms_.find(_sym_to_check_);
            opening_count++;
            if (_res == syms._syms_.end())
            {
                // it is defined and so we have to skip everything until we encounter an "endif"
                remove_until_endif(filename, contents);
            }
            else
            {
                add_until_endif(filename, contents);
            }
        }
        else if (temp == "undef")
        {
            auto _sym_to_check_ = get_the_sym(filename);
            syms._syms_.erase(_sym_to_check_);
            *contents += '\n';
        }
        else if (temp == "define")
        {
            handle_defines();
        }
        if (opening_count == closing_count)
            break;
        if (curr_char == _file_contents_.end())
            break;
    }
    if (opening_count != closing_count)
    {
        line_no = start;
        error::_print_err_(filename, start, 0, _CONTEXT_PREP_, "No terminating \"endif\" found.", get_curr_line());
    }
}

void masm::prep::Prep::_clean_directives_(std::string filename)
{
    std::string modified_file;
    while (curr_char != _file_contents_.end())
    {
        if (*curr_char == ' ' || *curr_char == '\t')
        {
            while (*curr_char == ' ' || *curr_char == '\t')
            {
                modified_file += *curr_char;
                col_no++;
                curr_char++;
            }
        }
        else if (*curr_char == '\n')
        {
            _start_ += col_no + 1;
            col_no = 0;
            line_no++;
            modified_file += *curr_char;
            curr_char++;
        }
        else if (*curr_char == '$')
        {
            modified_file += '\n';
            curr_char++;
            if (*curr_char == ' ' || *curr_char == '\t')
            {
                while (*curr_char == ' ' || *curr_char == '\t')
                {
                    col_no++;
                    curr_char++;
                }
            }
            std::string temp;
            while (std::isalpha(*curr_char) && curr_char != _file_contents_.end())
            {
                temp += *curr_char;
                curr_char++;
                col_no++;
            }
            if (temp == "ifdef")
            {
                auto _sym_to_check_ = get_the_sym(filename);
                auto _res = syms._syms_.find(_sym_to_check_);
                if ((_res == syms._syms_.end()))
                {
                    // it is not defined and so we have to skip everything until we encounter an "endif"
                    remove_until_endif(filename, &modified_file);
                }
                else
                    add_until_endif(filename, &modified_file);
            }
            else if (temp == "ifndef")
            {
                auto _sym_to_check_ = get_the_sym(filename);
                auto _res = syms._syms_.find(_sym_to_check_);
                if (_res != syms._syms_.end())
                {
                    // it is defined and so we have to skip everything until we encounter an "endif"
                    remove_until_endif(filename, &modified_file);
                }
                else
                    add_until_endif(filename, &modified_file);
            }
            else if (temp == "undef")
            {
                auto _sym_to_check_ = get_the_sym(filename);
                syms._syms_.erase(_sym_to_check_);
                modified_file += '\n';
            }
            else if (temp == "endif")
            {
                error::_print_err_(filename, line_no, 0, _CONTEXT_PREP_, "Stray endif directive found.", get_curr_line());
            }
        }
        else if (*curr_char == '#')
        {
            // This is an include
            // since this is outside any block this must be included
            // Now check if this exists or not
            // We couldn't throw the error while resolving the includes
            // because we weren't sure if those includes would be used or not
            modified_file += '\n';
            curr_char++;
            std::string path;
            while (*curr_char != '\n' && curr_char != _file_contents_.end())
            {
                path += *curr_char;
                curr_char++;
            }
            if (!std::filesystem::exists(path))
                error::_print_err_(filename, line_no, 0, _CONTEXT_PREP_, "This included file doesn't exist. Maybe check the path?", "$include " + path);
        }
        else
        {

            modified_file += *curr_char;
            curr_char++;
            col_no++;
        }
    }
    _file_contents_ = modified_file;
}

void masm::prep::Prep::_second_pass_()
{
    // first analyze the main file itself
    // Our job in the second pass is to remove unnecessary code and make sure that unneeded files are removed based on the defines
    _self_files_.erase(_self_files_.begin(), _self_files_.end());
    _included_files_.erase(_included_files_.begin());
    std::string _curr_file_ = _file_contents_; // store temporarily
    long i = _included_files_.size() - 1;
    while (i >= 0)
    {
        if (std::find(_self_files_.begin(), _self_files_.end(), _included_files_[i]) == _self_files_.end())
        {
            // this hasn't been excluded yet and so we have to perform second pass on it
            _file_contents_ = _details_[_included_files_[i]]._file_contents_;
            curr_char = _file_contents_.begin();
            line_no = 1;
            col_no = 0;
            _clean_directives_(_included_files_[i]);
            _details_[_included_files_[i]]._file_contents_ = _file_contents_;
        }
        i--;
    }

    _file_contents_ = _curr_file_;
    curr_char = _file_contents_.begin();
    line_no = 1;
    col_no = 0;
    _clean_directives_(_reader_.get_file_name());
    _curr_file_ = _file_contents_ + "\n";
    // we will combine the entire file to get the final file here
    _FileSyms_ sym;
    sym.start = 1;
    sym.end = line_no;
    sym._file_name_ = _reader_.get_file_name();
    _syms_.push_back(sym);
    for (auto _p : _included_files_)
    {
        if (std::find(_self_files_.begin(), _self_files_.end(), _p) == _self_files_.end())
        {
            _curr_file_.append(_details_[_p]._file_contents_ + "\n");
            sym.start = line_no;
            sym.end = line_no + _details_[_p].lines;
            line_no = sym.end;
            sym._file_name_ = _p;
            _syms_.push_back(sym);
        }
    }
    _file_contents_ = _curr_file_;
    // We now have one big file and now we need to do one more pass to replace the values of the defines
}

masm::prep::_FileSyms_ masm::prep::Prep::get_file()
{
    for (auto _f : _syms_)
    {
        if (_f.start <= line_no && _f.end >= line_no)
            return _f;
    }
    __builtin_unreachable();
}

masm::prep::_FileSyms_ masm::prep::Prep::get_file(size_t line)
{
    for (auto _f : _syms_)
    {
        if (_f.start <= line && _f.end >= line)
            return _f;
    }
    __builtin_unreachable();
}

void masm::prep::Prep::_third_pass_()
{
    // go through the _file_contents_ one last time
    // replace all and every values that were defined
    /// NOTE: After this step, the lexer will not know that the value it is lexing is an expanded macro which is also the case for the preprocessor
    // The lexer will pass it's errors through the preprocessor
    // If the error was in an expanded macro
    // the preprocessor will not provide any info that the error happened at that line
    // The preprocessor will print the expanded macro and highlight that error
    curr_char = _file_contents_.begin();
    line_no = 1;
    _start_ = 0;
    col_no = 0;
    std::string _final_modified_file_;
    while (curr_char != _file_contents_.end())
    {
        // start at one space until the next and see if it is a defined symbol
        std::string curr_sym;
        if (*curr_char == ' ' || *curr_char == '\t')
        {
            while (*curr_char == ' ' || *curr_char == '\t')
            {
                _final_modified_file_ += *curr_char;
                col_no++;
                curr_char++;
            }
        }
        while (*curr_char != ' ' && *curr_char != '\n' && curr_char != _file_contents_.end())
        {
            curr_sym += *curr_char;
            curr_char++;
        }
        auto _det = syms._syms_.find(curr_sym);
        if (_det != syms._syms_.end())
        {
            // a valid macro
            // also check if the value of this macro is another macro
            auto _det2 = syms._syms_.find(_det->second);
            if (_det2 != syms._syms_.end())
            {
                // yes it is
                // keep expanding until we get the final value
                size_t count = 0;
                std::string val = _det2->second;
                while ((_det2 = syms._syms_.find(_det2->second)) != syms._syms_.end())
                {
                    if (count > syms._syms_.size())
                    {
                        // this checks if we are continuously expanding
                        // if this line is executed then it means we have been recursively expanding
                        auto res = get_file();
                        error::_print_err_(res._file_name_, line_no - res.start, col_no, _CONTEXT_PREP_, "Recursive macro expansion.", get_curr_line());
                    }
                    count++;
                    val = _det2->second;
                }
                _final_modified_file_ += val;
            }
            else
            {
                _final_modified_file_ += _det->second;
            }
            col_no += curr_sym.length();
            // we have the final value
        }
        else
        {
            _final_modified_file_ += curr_sym;
            col_no += curr_sym.length();
        }
        if (*curr_char == '\n')
        {
            _final_modified_file_ += '\n';
            _start_ = col_no + 1;
            col_no = 0;
            line_no++;
            curr_char++;
        }
    }
    _file_contents_ = _final_modified_file_;
}