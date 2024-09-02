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
    memcpy(_cmd_opts, argv, total_len);
#elif defined(_USE_LINUX_)
    for (msize_t i = 1; i < argc; i++)
        total_len += strlen(argv[i]) + 1;
    _cmd_opts = (mstr_t *)malloc(sizeof(char) * total_len + 8);
    if (_cmd_opts == NULL)
        return;
    _cmd_opt_len = argc - 1;
    memcpy(_cmd_opts, argv[1], total_len);
    *(_cmd_opts + total_len) = NULL;
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