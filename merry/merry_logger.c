#include "../utils/merry_logger.h"

#if defined(_MERRY_LOGGER_ENABLED_)
void merry_logger_init()
{
    logger.f = fopen("merryLog.txt", "w");
    logger.lock = merry_mutex_init();
}

void merry_log(mstr_t _device_, mstr_t _info_, mstr_t _details_)
{
    merry_mutex_lock(logger.lock);
    fprintf(logger.f, "[%s][%s]: %s.\n", _device_, _info_, _details_);
    merry_mutex_unlock(logger.lock);
}

void merry_llog(mstr_t _device_, mstr_t _info_, mstr_t _msg_, ...)
{
    merry_mutex_lock(logger.lock);
    va_list list;
    va_start(list, _msg_);
    fprintf(logger.f, "[%s][%s]:", _device_, _info_);
    vfprintf(logger.f, _msg_,  list);
    putc((int)'\n', logger.f);
    va_end(list);
    merry_mutex_unlock(logger.lock);
}

void merry_logger_close()
{
    fclose(logger.f);
    merry_mutex_destroy(logger.lock);
}

#endif
