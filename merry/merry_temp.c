#include "merry_temp.h"

void merry_save_cmd_options(msize_t argc, mstr_t *argv)
{
    msize_t total_len = 0;
#ifdef _USE_WIN_
    for (msize_t i = 0; i < argc; i++)
        total_len += strlen(argv[i]) + 1;
    _cmd_opts = (mstr_t *)malloc(sizeof(char) * total_len);
    if (_cmd_opts == NULL)
        return;
    // don't mind the weird type cast
    _cmd_opt_len = argc;
    memcpy(_cmd_opts, &argv[0][0], total_len);
#elif defined(_USE_LINUX_)
   // Count the total length of all the arguments including their null terminators
    for (msize_t i = 0; i < argc; i++)
        total_len += strlen(argv[i]) + 1; // +1 for null terminators
    _cmd_opts = (mstr_t *)malloc((argc + 1) * sizeof(mstr_t *));
    if (_cmd_opts == NULL)
        return;
    for (msize_t i = 0; i < argc; i++)
        _cmd_opts[i] = strdup(argv[i]);
    
    _cmd_opts[argc] = NULL; // Null-terminate the argument array
    _cmd_opt_len = argc;
#endif
}

void merry_get_cmd_options(msize_t *argc, mstr_t **argv)
{
    *argc = _cmd_opt_len;
    *argv = _cmd_opts;
}

void merry_clean_state()
{
    if (_cmd_opts != NULL)
        free(_cmd_opts);
}