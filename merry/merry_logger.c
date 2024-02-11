#include "../utils/merry_logger.h"

mret_t merry_logger_init(mbool_t enable_flag)
{
    if (enable_flag == mtrue)
    {
        logger.f = fopen("merry.log", "w");
        if (logger.f == NULL)
        {
            logger.enabled = mfalse;
            return RET_FAILURE;
        }
        logger.lock = merry_mutex_init();
        if (logger.lock == RET_NULL)
        {
            logger.enabled = mfalse;
            fclose(logger.f);
            return RET_FAILURE;
        }
        logger.enabled = mtrue;
    }
    else
    {
        logger.enabled = mfalse;
    }
    return RET_SUCCESS; // well this return doesn't matter really
}

void merry_log(mstr_t _device_, mstr_t _info_, mstr_t _details_)
{
    if (logger.enabled == mfalse)
        return;
    merry_mutex_lock(logger.lock);
    fprintf(logger.f, "[%s][%s]: %s.\n", _device_, _info_, _details_);
    merry_mutex_unlock(logger.lock);
}

void merry_llog(mstr_t _device_, mstr_t _info_, mstr_t _msg_, ...)
{
    if (logger.enabled == mfalse)
        return;
    merry_mutex_lock(logger.lock);
    va_list list;
    va_start(list, _msg_);
    fprintf(logger.f, "[%s][%s]:", _device_, _info_);
    vfprintf(logger.f, _msg_, list);
    putc((int)'\n', logger.f);
    va_end(list);
    merry_mutex_unlock(logger.lock);
}

void merry_logger_close()
{
    if (logger.enabled == mfalse)
        return;
    fclose(logger.f);
    merry_mutex_destroy(logger.lock);
}
